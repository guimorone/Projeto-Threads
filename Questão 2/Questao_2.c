#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <string.h> 

char colores[][7] = {"\e[40m", "\e[41m",
"\e[42m", "\e[43m", "\e[44m", "\e[45m", "\e[46m",
"\e[47m"};

pthread_mutex_t *mutexes;
pthread_t *threads;

typedef struct {
    int linha;
    char *text;
} msg;

typedef struct {
    int size;
    msg *vetMsg;
} theMsg;

int *tamLinhasArq;
int L;
msg** matrixMsg;
msg *tela;

void *func(void *cmd) {
    //printf("oi");
    theMsg command = *((theMsg *)cmd);
    //printf("command.size=%d", command.size);
    int i;
    for(i=0; i<command.size; i++) {
        pthread_mutex_lock(&mutexes[(command.vetMsg[i].linha)-1]);
        tela[(command.vetMsg[i].linha)-1].text = (char *) malloc(strlen(command.vetMsg[i].text));
        strcpy(tela[(command.vetMsg[i].linha)-1].text, command.vetMsg[i].text);
        //printf("tela.linha = %d e tela.text = %s\n", tela[(command.vetMsg[i].linha)-1].linha, tela[(command.vetMsg[i].linha)-1].text);
        printf("command.size=%d\n", command.size);
        pthread_mutex_unlock(&mutexes[(command.vetMsg[i].linha)-1]);

    }
    //pthread_exit(NULL);
}

void printLinhaTela(int index) {
    printf("%d", tela[index].linha);
    printf("%s", colores[index]);        
    printf("%s\n", tela[index].text);
    printf("\e[0m");
}

void init(int L, int qtdThreads) {
    threads = (pthread_t *) malloc(qtdThreads*sizeof(pthread_t));
    mutexes = (pthread_mutex_t *) malloc(L*sizeof(pthread_mutex_t));
    tela = (msg *) malloc(L*sizeof(msg));

    int i;
    for(i=0; i<L; i++) {
        tela[i].linha = i;
        tela[i].text = (char *) malloc(6);
        strcpy(tela[i].text, "undef");
        printLinhaTela(i);
        pthread_mutex_init(&mutexes[i], NULL);
    }    
}

void leArquivos(int *n) {
    int i;
    printf("Digite o numero de arquivos: ");
    scanf("%d", n);

    matrixMsg = (msg **) malloc((*n)*sizeof(msg *));
    tamLinhasArq = (int *) malloc((*n)*sizeof(int));

    for(i=0; i< (*n); i++) {
        char str[50];
        printf("Digite o nome do arquivo no formato *.txt: ");
        scanf(" %[^\n]", str);
        FILE *arq = fopen(str, "r");
        if(arq == NULL) {
            printf("ERRO ao criar arquivo %d", i);
            exit(-1);
        }

        int linha;
        char text[100];
        int j=0;
        matrixMsg[i] = NULL;
        tamLinhasArq[i] = 0;
        while(fscanf(arq, "%d %[^\n]", &linha, text) != EOF) {
            matrixMsg[i] = (msg *) realloc(matrixMsg[i], (1+j)*sizeof(msg));
            matrixMsg[i][j].linha = linha;
            matrixMsg[i][j].text = (char *) malloc(strlen(text));
            strcpy(matrixMsg[i][j].text, text);
            //printf("%d-%s ", matrixMsg[i][j].linha, matrixMsg[i][j].text);
            j++;
            tamLinhasArq[i]++;
        }
        printf("tam *msg = %d", tamLinhasArq[i]);
        printf("\n");
    }

}


int main() {
    int nArq;
    leArquivos(&nArq);

    int i, j; 
    for(i=0; i<nArq; i++) {
        for(j=0; j<tamLinhasArq[i]; j++) {
            printf("%d %s\n", matrixMsg[i][j].linha, matrixMsg[i][j].text);
        }
        printf("\n\n");
    }

    printf("Digite a quantidade de linhas da tela: ");
    scanf("%d", &L);     
    init(L, nArq);

    theMsg *trueMsg = (theMsg *) malloc(nArq*sizeof(theMsg));

    int tam;
    for(i=0; i<nArq; i++) {

        tam = tamLinhasArq[i];
        //printf("%d\n", tam);
        trueMsg[i].vetMsg = (msg *) malloc(tam*sizeof(msg));
        trueMsg[i].size = tam;
        //printf("trueMsg.size=%d\n", trueMsg[i].size);
        for(j=0; j<tam; j++) {
            trueMsg[i].vetMsg[j].linha = matrixMsg[i][j].linha;
            trueMsg[i].vetMsg[j].text = (char *) malloc(strlen(matrixMsg[i][j].text));   
            strcpy(trueMsg[i].vetMsg[j].text, matrixMsg[i][j].text);
            //printf("cmd.linha = %d e cmd.text = %s\n", trueMsg[i].vetMsg[j].linha, trueMsg[i].vetMsg[j].text);
        }

        int rc = pthread_create(&threads[i], NULL, func, (void *) &trueMsg[i]);
        if(rc) printf("ERRO na criação %d, código de retorno: %d\n", j, rc);
    }

    for(i=0; i<nArq; i++) {
        pthread_join(threads[i], NULL);
        printf("esperando a thread[%d]\n", i);
    }
    
    for(i=0; i < L; i++) {
        printf("%d", tela[i].linha);
        printf("%s", colores[i]);        
        printf("%s\n", tela[i].text);
        printf("\e[0m");
    }

    free(threads);
    free(mutexes);
    for(i=0; i<L; i++) {
        free(tela[i].text);
    }
    free(tela);

    pthread_exit(NULL);
    return 0;
}