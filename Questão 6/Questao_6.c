#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>

// tamanho do buffer -> tamanho máximo da FILA
// numElements -> número de elementos no total
int threadsProdutoras, threadsConsumidoras, numElements;

// empty -> fila vazia, fill -> buffer cheio
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;


typedef struct elem{
   int value;
   struct elem *prox;
} Elem;
 
typedef struct blockingQueue{
   unsigned int sizeBuffer, statusBuffer;
   Elem *head,*last;
} BlockingQueue;

typedef struct threadStruct{
    BlockingQueue *buffer;
    int threadID;
} threadStruct;

void adicionarElem(BlockingQueue *Q, int v) {
    // adicionar elem ao final da fila

    Elem *aux = (Elem *) malloc(sizeof(Elem));
    aux->value = v;
    aux->prox = NULL;

    // primeiro elemento da fila
    if(Q->statusBuffer == 0){
        Q->head = aux;
        Q->last = aux;
    } else {
        // O elemento que antes era o último agora vai apontar para o NOVO último (aux)
        Q->last->prox = aux;
        // aux vira o ultimo elemento
        Q->last = aux;
    }

    Q->statusBuffer++;
}

// retira o primeiro elemento da fila
int retirarElem(BlockingQueue *Q) {
    if(Q->statusBuffer == 0){
        // buffer vazio
        // isso vai ser checado dentro do takeBlockingQueue já
        return -1;
    }

    int result = Q->head->value;
    Elem *aux = Q->head;

    if(Q->head->prox != NULL && Q->statusBuffer == 2) {
        // quando retirar o elemento, vai sobrar apenas um
        // aqui vamos fazer o elemento que sobrar ser o LAST e o HEAD simultaneamente
        Q->head = Q->head->prox;
        Q->last = Q->head;
    } else if(Q->head->prox != NULL){
        Q->head = Q->head->prox;
    } else {
        Q->head = NULL;
        Q->last = NULL;
    }

    Q->statusBuffer--;

    free(aux);

    return result;
}
 

BlockingQueue* newBlockingQueue(unsigned int SizeBuffer){
    BlockingQueue *q = (BlockingQueue *) malloc(sizeof(BlockingQueue));
    q->sizeBuffer = SizeBuffer;
    q->statusBuffer = 0;
    q->head = NULL;
    q->last = NULL;

    return q;
}

// produtor -> put
void putBlockingQueue(BlockingQueue* Q, int newValue){
    pthread_mutex_lock(&mutex);

    // verifica se o buffer está cheio
    while(Q->sizeBuffer <= Q->statusBuffer){
        printf("Fila cheia\n");
        // thread vai dormir
        pthread_cond_wait(&empty, &mutex);
    }

    adicionarElem(Q, newValue);
    
    if(Q->statusBuffer == 1) {
        pthread_cond_broadcast(&fill);
        // acorda as outras threads
    }

    pthread_mutex_unlock(&mutex); 
    
}

// consumidor -> get
int takeBlockingQueue(BlockingQueue* Q){
    pthread_mutex_lock(&mutex);

    // verifica se o buffer está vazio
    while(Q->statusBuffer == 0){
        printf("Fila vazia\n");
        // thread vai dormir
        pthread_cond_wait(&fill, &mutex);
    }

    int result = retirarElem(Q);

    if(Q->statusBuffer == Q->sizeBuffer - 1){
        // acordar as outras threads
        pthread_cond_broadcast(&empty);
    }

    pthread_mutex_unlock(&mutex); 

    return result;
}

void *produtor(threadStruct *param){

    int i;
    // como pode ter mais de uma thread consumidora,
    // se produz numElements * threadsConsumidoras itens
    // isso é feito para não faltar itens para as threads
    for(i = 0; i < numElements * threadsConsumidoras; i++){
        // +1 elemento no buffer
        putBlockingQueue(param->buffer, (param->threadID * numElements * threadsConsumidoras) + i + 1);
        printf("Thread %d produziu: %d \n", param->threadID, (param->threadID * numElements * threadsConsumidoras) + i + 1);
    }
    free(param);
    pthread_exit(NULL);
}

void *consumidor(threadStruct* param){
    
    int i;
    // como pode ter mais de uma thread produtora,
    // se consome numElements * threadsProdutoras itens
    for(i = 0; i < numElements * threadsProdutoras; i++){
        // -1 elemento no buffer
        int aux = takeBlockingQueue(param->buffer);
        printf("Thread %d consumiu: %d \n", param->threadID, aux);
    }  

    pthread_exit(NULL);
}

void clear(BlockingQueue *Q){
    while(Q->statusBuffer > 0){
        int aux = retirarElem(Q);
    }

    free(Q);
}

void printa(int val){
  printf("Produzi: %d", val);
  printf("Consumi: %d", val);
}


int main() {

    unsigned int tamBuffer;

    printf("Digite o número de threads produtoras, threads consumidoras e o tamanho do buffer e o número de elementos no total: ");
    scanf("%d %d %u %d", &threadsProdutoras, &threadsConsumidoras, &tamBuffer, &numElements);

    BlockingQueue *fila = newBlockingQueue(tamBuffer);

    // array de threads
    pthread_t *consumer = (pthread_t *) malloc(threadsConsumidoras * sizeof(pthread_t)); 
    pthread_t *producer = (pthread_t *) malloc(threadsProdutoras * sizeof(pthread_t));

    int j;
    // Cria as threads produtoras
    for(j = 0; j < threadsProdutoras; j++){
        threadStruct *aux = (threadStruct *) malloc(sizeof(threadStruct));
        aux->buffer = fila;
        aux->threadID = j;
        pthread_create(&producer[j], NULL, (void *) produtor, (void *) aux);
    } 

    // Cria as threads consumidoras
    for(j = 0; j < threadsConsumidoras; j++){
        threadStruct *aux = (threadStruct *) malloc(sizeof(threadStruct));
        aux->buffer = fila;
        aux->threadID = j;
        pthread_create(&consumer[j], NULL, (void *) consumidor, (void *) aux);
    }

    // Esperar o término de execução das threads

    for(j = 0; j < threadsProdutoras; j++){
        pthread_join(producer[j], NULL);
    } 

    for(j = 0; j < threadsConsumidoras; j++){
        pthread_join(consumer[j], NULL);
    } 

    // Libera memória

    free(consumer);
    free(producer);
    clear(fila);

    pthread_exit(NULL);
}