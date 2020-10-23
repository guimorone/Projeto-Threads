#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <string.h>
#include <time.h>  

char colores[][7] = {"\e[40m", "\e[41m",
"\e[42m", "\e[43m", "\e[44m", "\e[45m", "\e[46m",
"\e[47m"};

typedef struct {
    int linha;
    char *text;
} msg;

pthread_mutex_t printMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *mutexes;
pthread_t *threads;
int L;
msg *tela;
char** filenames;

void delay(int number_of_seconds) 
{ 
    int milli_seconds = 1000 * number_of_seconds; 
    clock_t start_time = clock(); 
  
    while (clock() < start_time + milli_seconds); 
} 

void printLinhaTela(int index) {
    printf("%d", tela[index].linha);
    printf("%s", colores[index]);        
    printf("%s\n", tela[index].text);
    printf("\e[0m");
}

void *func(void *cmd) {
    delay(2000);
    char *command = *((char **)cmd);

    FILE *arq = fopen(command, "r");
    if(arq == NULL) {
        printf("ERRO ao tentar abrir arquivo %s\n", command);
        exit(-1);
    }

    int size = 0;
    int linha;
    char text[100];
    msg *dados = NULL;

    while(fscanf(arq, "%d %[^\n]", &linha, text) != EOF) {
        dados = (msg *) realloc(dados, (1+size)*sizeof(msg));
        dados[size].linha = linha;
        dados[size].text = (char *) malloc(strlen(text));
        strcpy(dados[size].text, text);
        size++;
    }
    fclose(arq);

    int i;
    for(i=0; i<size; i++) {
        pthread_mutex_lock(&mutexes[(dados[i].linha)-1]);
        tela[(dados[i].linha)-1].text = (char *) malloc(strlen(dados[i].text));
        strcpy(tela[(dados[i].linha)-1].text, dados[i].text);

        //pthread_mutex_lock(&printMutex);
        int j;
        for(j=0; j<L; j++) {
          printLinhaTela(j);
        }
        printf("\n");
        //pthread_mutex_unlock(&printMutex);

        delay(4500);
        system("clear");
        pthread_mutex_unlock(&mutexes[(dados[i].linha)-1]);

    }

    free(dados);
}

void init(int L, int qtdThreads) {
    threads = (pthread_t *) malloc(qtdThreads*sizeof(pthread_t));
    mutexes = (pthread_mutex_t *) malloc(L*sizeof(pthread_mutex_t));
    tela = (msg *) malloc(L*sizeof(msg));
    filenames = (char **) malloc(qtdThreads*sizeof(char *));

    int i;

    for(i=0; i<qtdThreads; i++) {
        filenames[i] = (char *) malloc(50);
    }

    for(i=0; i<L; i++) {
        tela[i].linha = i;
        tela[i].text = (char *) malloc(6);
        strcpy(tela[i].text, "undef");
        printLinhaTela(i);
        pthread_mutex_init(&mutexes[i], NULL);
    }    
}


int main() {
    int nArq;
    printf("Digite o numero de arquivos: ");
    scanf("%d", &nArq);

    int i, j; 

    printf("Digite a quantidade de linhas da tela: ");
    scanf("%d", &L);     
    init(L, nArq);

    for(i=0; i<nArq; i++) {
        printf("Digite o nome do arquivo[%d] a ser lido: ", i);
        scanf(" %[^\n]", filenames[i]);
    }

    int tam;
    for(i=0; i<nArq; i++) {

        int rc = pthread_create(&threads[i], NULL, func, (void *) &filenames[i]);
        if(rc) printf("ERRO na criacao da thread[%d], codigo de retorno: %d\n", i, rc);
    }

    for(i=0; i<nArq; i++) {
        pthread_join(threads[i], NULL);
    }
    
    for(i=0; i < L; i++) {
        printf("%d", tela[i].linha);
        printf("%s", colores[i]);        
        printf("%s\n", tela[i].text);
        printf("\e[0m");
    }



    free(threads);
    free(mutexes);
    for(i=0; i<nArq; i++) {
        free(filenames[i]);
    }
    free(filenames);

    for(i=0; i<L; i++) {
        free(tela[i].text);
    }
    free(tela);

    pthread_exit(NULL);
    return 0;
}