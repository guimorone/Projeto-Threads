#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h> 
#include <pthread.h>

int qtdThreads,qntRequisicoes,id=-1;
int* readyResults;

pthread_t despachante; 

pthread_mutex_t* resultMutexes;
pthread_mutex_t mutexDespachante = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t emptyQueue = PTHREAD_COND_INITIALIZER;
pthread_cond_t threadDisponivel = PTHREAD_COND_INITIALIZER;


typedef struct structAgendar{
    void* param;
    void* funexec;
} structAgendar;

typedef struct requisicao{
    void* param;
    void* funexec;
    int id;
} requisicao;

typedef struct threadWrapper{
    pthread_t thread;
    int threadID;
    int occupied;
} threadWrapper;

typedef struct threadFunctionStruct{
    int threadID;
    requisicao req;
} threadFunctionStruct;

typedef struct elem{
   requisicao value;
   struct elem *prox;
} Elem;

typedef struct queue{
   unsigned int statusBuffer;
   Elem *head,*last;
} Queue;

threadWrapper* threads; // threads workers

Queue *buffer;

void adicionarElem(requisicao req) {
    // adicionar elem ao final da fila

    Elem *aux = (Elem *) malloc(sizeof(Elem));
    aux->value = req;
    aux->prox = NULL;

    // primeiro elemento da fila
    if(buffer->statusBuffer == 0){
        buffer->head = aux;
        buffer->last = aux;
    } else {
        // O elemento que antes era o último agora vai apontar para o NOVO último (aux)
        buffer->last->prox = aux;
        // aux vira o ultimo elemento
        buffer->last = aux;
    }

    buffer->statusBuffer++;
}

requisicao retirarElem() {
    if(buffer->statusBuffer == 0){
        // buffer vazio
        // já será verificado antes, mas só para ter certeza
        printf("Disney");
        exit(-1);
    }

    requisicao result = buffer->head->value;
    Elem *aux = buffer->head;

    if(buffer->head->prox != NULL && buffer->statusBuffer == 2) {
        buffer->head = buffer->head->prox;
        buffer->last = buffer->head;
    } else if(buffer->head->prox != NULL){
        buffer->head = buffer->head->prox;
    } else {
        buffer->head = NULL;
        buffer->last = NULL;
    }

    buffer->statusBuffer--;

    free(aux);

    return result;
}

int findFreeThread(){
    int i;
    for(i=0;i<qtdThreads;i++){
        if(threads[i].occupied == false){
            return i;
        }
    }
    return -1;
}

int agendarExecucao(structAgendar req){
    id++;
    requisicao novaReq;
    novaReq.param = req.param;
    novaReq.funexec = req.funexec;
    novaReq.id = id;
    adicionarElem(novaReq);
    pthread_mutex_lock(&resultMutexes[id]);
    pthread_cond_broadcast(&emptyQueue);
    return id;
}

void threadFunctionWrapper(threadFunctionStruct params){
    //executa a funcao do usuario e guarda o retorno
    //int result = params.req.funexec(params.req.param);
    int (*fun)(int) = params.req.funexec;
    int result = fun((int)params.req.param);
    readyResults[params.req.id]=result;
    //desbloqueia o usuario que quiser pegar essa informaçao
    pthread_mutex_unlock(&resultMutexes[params.req.id]);
    //sinaliza que essa thread nao está mais ocupada
    threads[params.threadID].occupied = false;
    pthread_cond_broadcast(&threadDisponivel);
}

void despachanteFunc(){
    int i;
    for(i = 0; i < qntRequisicoes; i++){

        while(buffer->statusBuffer==0) pthread_cond_wait(&emptyQueue, &mutexDespachante);
        requisicao command = retirarElem();
        //espera ter uma thread livre
        int threadID = findFreeThread();
        while(threadID==-1){
            pthread_cond_wait(&threadDisponivel, &mutexDespachante);
            threadID = findFreeThread();
        }
        //sinaliza que a nova thread está ocupada
        threads[threadID].occupied = true;

        threadFunctionStruct threadParams;
        threadParams.threadID = threadID;
        threadParams.req = command;
        //chama o function wrapper usando a thread livre
        pthread_create(&threads[threadID].thread,NULL,(void*)threadFunctionWrapper, (void*) &threadParams);
    }
}

int pegarResultadoExecucao(int searchID){
    //fica bloqueado até o mutex daquela requisicao especifica for desbloqueado
    pthread_mutex_lock(&resultMutexes[searchID]);
    return readyResults[searchID];
}

void init(){

    //inicializa buffer
    buffer = (Queue *) malloc(sizeof(Queue));
    buffer->statusBuffer = 0;
    buffer->head = NULL;
    buffer->last = NULL;

    //criar thread despachante
    pthread_create(&despachante, NULL, (void *) despachanteFunc, NULL);
    //inicializar threads worker
    threads = (threadWrapper *) malloc(qtdThreads * sizeof(threadWrapper));
    //inicializar mutexes
    resultMutexes = (pthread_mutex_t *) malloc(qtdThreads * sizeof(pthread_mutex_t));
    int i;
    for(i = 0; i < qtdThreads; i++) pthread_mutex_init(&resultMutexes[i], NULL);
    //inicializar o vetor de resultados
    readyResults = (int *) malloc(qntRequisicoes * sizeof(int));
}


void clear(){
    //liberar buffer
    //liberar threads worker
    while(buffer->statusBuffer > 0){
        requisicao aux = retirarElem();
    }

    free(buffer);

    //liberar threads worker
}


int funexec(int a){
    return a++;
}

int main() {

    init();

    printf("Digite a quantidade de processadores ou núcleos do sistema computacional: ");
    scanf("%d", &qtdThreads);

    printf("Digite a quantidade de requisicões: ");
    scanf("%d", &qntRequisicoes);

    //inicializar o servidor
    init();

    int i;
    for(i = 0; i < qntRequisicoes; i++){
        structAgendar novaReq;
        novaReq.funexec = (void*) funexec;
        novaReq.param = (void *) i;
        agendarExecucao(novaReq);
    }

    for(i = 0; i < qntRequisicoes; i++){
        int resultado = pegarResultadoExecucao(i);
        printf("Resultado %d: %d", i, resultado);
    }

    clear();

    pthread_exit(NULL);
}