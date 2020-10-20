#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>

int qtdThreads;

pthread_t *threads, despachante; 

// empty -> buffer vazio
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

// O buffer será implementado com a estrutura de dados
// de uma lista encadeada

typedef struct elem{
   int value;
   struct elem *prox;
} Elem;

// status buffer == tamanho atual do buffer
// o buffer não tem um tamanho máximo
typedef struct queue{
   unsigned int statusBuffer;
   Elem *head,*last;
} Queue;

Queue *buffer;

void adicionarElem(int v) {
    // adicionar elem ao final da fila

    Elem *aux = (Elem *) malloc(sizeof(Elem));
    aux->value = v;
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

// retira o primeiro elemento do buffer
int retirarElem() {
    if(buffer->statusBuffer == 0){
        // buffer vazio
        // já será verificado antes, mas só para ter certeza
        return -1;
    }

    int result = buffer->head->value;
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

void criarBuffer(){
    buffer = (BlockingQueue *) malloc(sizeof(BlockingQueue));
    buffer->statusBuffer = 0;
    buffer->head = NULL;
    buffer->last = NULL;
}

void *funexec(int param){ // param == id
    pthread_mutex_lock(&mutex);



    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *agendarExecução(int id){
    adicionarElem(id);
    printf("Requisição de id %d colocada no buffer\n", id);

    pthread_create(&despachante, NULL, (void *) funexec, (void *) id);

    // espera a despachante terminar de executar
    pthread_join(despachante, NULL);    
    pthread_exit(NULL);
}

void pegarResultadoExecucao(int id){
    // Se não terminou:
    printf("Execução de %d ainda não terminou\n", id);
    // Se sim:
    printf("Execução de %d finalizada\n", id);
}

void clear(){
    while(buffer->statusBuffer > 0){
        int aux = retirarElem();
    }

    free(buffer);
}

int main() {

    printf("Digite a quantidade de processadores ou núcleos do sistema computacional: ");
    scanf("%d", &qtdThreads);

    threads = (pthread_t *) malloc(qtdThreads * sizeof(pthread_t));
    criarBuffer();

    int i;
    for(i = 0; i < qtdThreads; i++){
        pthread_create(&threads[i], NULL, (void *) agendarExecução, (void *) i);
    }

    // esperar o término de execução das threads
    for(i = 0; i < qtdThreads; i++){
        pthread_join(threads[i], NULL);
    }

    // libera memória
    free(threads);
    clear(); //limpa todo o buffer

    pthread_exit(NULL);
}