#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>

// tamanho do buffer -> tamanho máximo da FILA
// numElements -> número de elementos no total
int threadsProdutoras, threadsConsumidoras, numElements;

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

    if(Q->head->prox != NULL && Q->statusBuffer == 2) {
        Q->head = Q->head->prox;
        Q->last = Q->head;
    } else if(Q->head->prox != NULL){
        Q->head = Q->head->prox;
    } else Q->head = NULL;

    Q->statusBuffer--;

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

    while(Q->sizeBuffer == Q->statusBuffer){
        printf("Fila cheia\n");
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

    while(Q->statusBuffer == 0){
        printf("Fila vazia\n");
        pthread_cond_wait(&fill, &mutex);
    }

    int result = retirarElem(Q);

    if(Q->statusBuffer == Q->sizeBuffer - 1){
        pthread_cond_broadcast(&empty);
    }

    pthread_mutex_unlock(&mutex); 

    return result;
}

void *produtor(BlockingQueue* Q){
    printf("Produtor \n");

    int i;
    for(i = 0; i < numElements * threadsConsumidoras; i++){
        putBlockingQueue(Q, i);
        printf("Produzi: %d \n", i);
    }

    pthread_exit(NULL);
}

void *consumidor(BlockingQueue* Q){
    printf("Consumidor \n");
    
    int i;
    for(i = 0; i < numElements * threadsProdutoras; i++){
        int aux = takeBlockingQueue(Q);
        printf("Peguei: %d \n", aux);
    }  

    pthread_exit(NULL);
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
    for(j = 0; j < threadsProdutoras; j++){
        pthread_create(&producer[j], NULL, produtor, (void *) fila);
    } 

    for(j = 0; j < threadsConsumidoras; j++){
        pthread_create(&consumer[j], NULL, consumidor, (void *) fila);
    }

    pthread_exit(NULL);


    int i;

    for(i = 0; i < threadsConsumidoras; i++){
        free(consumer[i]);
    }
    free(consumer);

    for(i = 0; i < threadsProdutoras; i++){
        free(producer[i]);
    }
    free(producer);

    free(fila);

    return 0;
}