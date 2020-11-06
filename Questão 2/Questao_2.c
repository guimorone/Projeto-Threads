#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <string.h>
#include <time.h>  

char colores[][7] = {"\e[40m", "\e[41m",
"\e[42m", "\e[43m", "\e[44m", "\e[45m", "\e[46m",
"\e[47m"};

//Struct que representa a tela a ser atualizada pelas threads.
typedef struct {
    int linha;
    char *text;
} msg;

//Struct que guarda uma lista de nomes de arquivos e seu tamanho.
typedef struct {
    int size;
    char **files;
} cmd;

pthread_mutex_t printMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *mutexes;
pthread_t *threads;
int L;
msg *tela;
cmd *filenamesPerThread;

//Função que executa delay.
void delay(int number_of_seconds) { 
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

//Função passada na criação das threads.
void *func(void *command) {
    delay(2000);
    cmd filesArr = *((cmd *)command);
    int k;

    //Para cada string na lista de strings de filesArr, lê-se o conteúdo do arquivo, cujo nome corresponde à string corrente
    //e faz-se as devidas atualizações na tela respeitando a exclusão mútua por linha.
    for(k=0; k<filesArr.size; k++) {
        FILE *arq = fopen(filesArr.files[k], "r");
        if(arq == NULL) {
            printf("ERRO ao tentar abrir arquivo.\n");
            exit(-1);
        }

        int size = 0;
        int linha;
        char text[100];
        msg *dados = NULL;

        //Leitura do arquivo respeitando o intervalo [1, L], onde L é a quantidade de linhas na tela.
        //Caso o intervalo não seja respeitado, a linha correspondente é desconsiderada na leitura do
        //arquivo em questão.
        while(fscanf(arq, "%d %[^\n]", &linha, text) != EOF) {
            dados = (msg *) realloc(dados, (1+size)*sizeof(msg));
            if(linha < 1 || linha > L) {
                printf("Valor para linha invalido. Esta sera desconsiderada.\n");
            }
            else {
                dados[size].linha = linha;
                dados[size].text = (char *) malloc(strlen(text));
                strcpy(dados[size].text, text);
                size++;
            }
        }
        fclose(arq);

        //Atualização da tela de acordo com a quantidade de alterações (size) calculadas para cada arquivo.
        //Essa atualização é feita de acordo com os dados lidos de cada arquivo e respeita a exclusão mútua
        //por linha. A linha correspondente é printada após a alteração e é adicionado um delay de 3 segundos
        //até que a próxima alteração seja feita.
        int i;
        for(i=0; i<size; i++) {
            pthread_mutex_lock(&mutexes[(dados[i].linha)-1]);
            tela[(dados[i].linha)-1].text = (char *) malloc(strlen(dados[i].text));
            strcpy(tela[(dados[i].linha)-1].text, dados[i].text);

            pthread_mutex_lock(&printMutex);
            int j;
            for(j=0; j<L; j++) {
              printLinhaTela(j);
            }
            printf("\n");
            pthread_mutex_unlock(&printMutex);

            delay(3000);
            system("clear");
            pthread_mutex_unlock(&mutexes[(dados[i].linha)-1]);

        }

        free(dados);
    }
    
}

//Função responsável por inicializar as estruturas que serão utilizadas ao longo do código.
void init(int L, int qtdArq, int qtdThreads) {
    threads = (pthread_t *) malloc(qtdThreads*sizeof(pthread_t));
    mutexes = (pthread_mutex_t *) malloc(L*sizeof(pthread_mutex_t));
    tela = (msg *) malloc(L*sizeof(msg));
    filenamesPerThread = (cmd *) malloc(qtdThreads*sizeof(cmd));
    
    int sizeFilePerThread = qtdArq/qtdThreads;
    int i,j;
    int k=0;
    
    //Aqui é feito uma divisão, entre as threads, de todos os arquivos que serão analisados.
    //De acordo com essa política, a última thread sempre ficará com um maior número de arquivos
    //para analisar. Isso ocorre se qtdArq % qtdThreads != 0.
    for(i=0; i<qtdThreads; i++) {
        if(i+1 == qtdThreads) {
            int tam = sizeFilePerThread + (qtdArq%qtdThreads);
            filenamesPerThread[i].files = (char **) malloc(tam*(sizeof(char *)));
            for(j=0; j<tam; j++) {
                printf("Digite o nome do arquivo[%d] a ser lido: ", k++);
                filenamesPerThread[i].files[j] = (char *) malloc(50);
                scanf(" %[^\n]", filenamesPerThread[i].files[j]);
            }
            filenamesPerThread[i].size = tam;
        }
        else {
            filenamesPerThread[i].files = (char **) malloc(sizeFilePerThread*(sizeof(char *)));
            for(j=0; j<sizeFilePerThread; j++) {
                printf("Digite o nome do arquivo[%d] a ser lido: ", k++);
                filenamesPerThread[i].files[j] = (char *) malloc(50);
                scanf(" %[^\n]", filenamesPerThread[i].files[j]);
            }
            filenamesPerThread[i].size = sizeFilePerThread;
        }
    }

    //Inicialização da tela responsável por mostrar as alterações.
    //Aqui também é inicializado o array de mutexes.
    for(i=0; i<L; i++) {
        tela[i].linha = i;
        tela[i].text = (char *) malloc(6);
        strcpy(tela[i].text, "undef");
        printLinhaTela(i);
        pthread_mutex_init(&mutexes[i], NULL);
    }    
}


int main() {
    int i, j;

    //Lê-se o números de arquivos a serem tratados.
    int nArq;
    printf("Digite o numero de arquivos: ");
    scanf("%d", &nArq);

    //Lê-se a quantidade de threads, e, caso não respeite o intervalo,
    //o usuário é novamente direcionado a entrar um valor válido.
    int nThreads;
    printf("Digite a quantidade de threads (1 < T <= %d) a serem executadas: ", nArq);
    scanf("%d", &nThreads);
    while(nThreads > nArq || nThreads <= 1) {
        printf("Quantidade de threads invalida. Entre com o numero de threads (1 < T <= %d): ", nArq);
        scanf("%d", &nThreads);
    } 

    printf("Digite a quantidade de linhas da tela: ");
    scanf("%d", &L);     
    
    //init é chamado de acordo com as variáveis já lidas.
    init(L, nArq, nThreads);

    //nThreads são criadas de acordo com a função func, que recebe como
    //parâmetro uma struct do tipo "cmd". Dessa maneira, a thread criada
    //recebe uma lista de strings, relacionadas aos nomes dos arquivos
    //correspondentes, e seu respectivo tamanho, para que a iteração
    //seja possível.
    for(i=0; i<nThreads; i++) {
        int rc = pthread_create(&threads[i], NULL, func, (void *) &filenamesPerThread[i]);
        if(rc) printf("ERRO na criacao da thread[%d], codigo de retorno: %d\n", i, rc);
    }

    //main espera a finalização de cada thread criada, para que se tenha
    //certeza de que todas as alterações válidas foram feitas na tela.
    for(i=0; i<nThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    //Printa pela última vez a tela já finalizada com todas as alterações.
    for(i=0; i < L; i++) {
        printf("%d", tela[i].linha);
        printf("%s", colores[i]);        
        printf("%s\n", tela[i].text);
        printf("\e[0m");
    }

    //Liberação da memória alocada dinamicamente ao longo do código.
    free(threads);
    free(mutexes);
    free(filenamesPerThread);
    for(i=0; i<L; i++) {
        free(tela[i].text);
    }
    free(tela);

    pthread_exit(NULL);
    return 0;
}