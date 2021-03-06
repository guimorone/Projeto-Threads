#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>

//Tamanho do buffer -> tamanho máximo da fila.
//numElements -> número de elementos no total.
int threadsProdutoras, threadsConsumidoras, numElements;

//empty -> fila vazia, fill -> buffer cheio.
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

//Utilizada para empacotar os dados passados como argumento para
//as funções de criação das threads produtoras/consumidoras.
typedef struct threadStruct{
    BlockingQueue *buffer;
    int threadID;
} threadStruct;

//Adiciona elemento ao final da fila.
void adicionarElem(BlockingQueue *Q, int v) {

    Elem *aux = (Elem *) malloc(sizeof(Elem));
    aux->value = v;
    aux->prox = NULL;

    //Se a fila estiver vazia, o elemento é o primeiro da fila.
    if(Q->statusBuffer == 0){
        Q->head = aux;
        Q->last = aux;
    } 
    else {

        //O elemento que antes era o último agora vai apontar para o novo último aux.
        Q->last->prox = aux;

        //Aux vira o ultimo elemento.
        Q->last = aux;
    }

    Q->statusBuffer++;
}

//Retira o primeiro elemento da fila.
int retirarElem(BlockingQueue *Q) {
    if(Q->statusBuffer == 0){
        //Buffer vazio
        //Isso vai ser checado dentro do takeBlockingQueue já.
        return -1;
    }

    int result = Q->head->value;
    Elem *aux = Q->head;

    if(Q->head->prox != NULL && Q->statusBuffer == 2) {

        //Quando retirar o elemento, vai sobrar apenas um. Assim, fazemos
        //o elemento que sobrar ser o last e o head simultaneamente.
        Q->head = Q->head->prox;
        Q->last = Q->head;
    } else if(Q->head->prox != NULL){

        //Se o tamanho da fila for maior que 2, o primeiro é removido e head
        //aponta para o próximo.
        Q->head = Q->head->prox;
    } else {

        //Se a fila tiver 1 elemento, ela se torna vazia.
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

//Produtor -> put.
void putBlockingQueue(threadStruct* param, int newValue){
    pthread_mutex_lock(&mutex);

    //Verifica se o buffer está cheio.
    while(param->buffer->sizeBuffer <= param->buffer->statusBuffer){

        //Se estiver, thread vai dormir.
        printf("\nFila cheia!\nThread produtora %d foi dormir\n\n", param->threadID);
        pthread_cond_wait(&empty, &mutex);
    }

    //Se buffer não estiver cheio, coloca elemento no fim da fila.
    adicionarElem(param->buffer, newValue);
    printf("Thread produtora %d produziu: %d\n", param->threadID, newValue);
    
    if(param->buffer->statusBuffer == 1) {
        
        //Acorda as outras consumidoras.
        printf("Todas as threads consumidoras foram acordadas.\n\n");
        pthread_cond_broadcast(&fill);
    }

    pthread_mutex_unlock(&mutex); 
    
}

//Consumidor -> take
int takeBlockingQueue(threadStruct* param){
    pthread_mutex_lock(&mutex);

    //Verifica se o buffer está vazio.
    while(param->buffer->statusBuffer == 0){
        
        //Se estiver, thread vai dormir.
        printf("\nFila vazia!\nThread consumidora %d foi dormir\n\n", param->threadID);
        pthread_cond_wait(&fill, &mutex);
    }

    //Se buffer não estiver vazio, retira primeiro elemento da fila.
    int result = retirarElem(param->buffer);
    printf("Thread consumidora %d consumiu: %d\n", param->threadID, result);

    if(param->buffer->statusBuffer == param->buffer->sizeBuffer - 1){
        
        //Acorda as outras produtoras.
        printf("Todas as threads produtoras foram acordadas.\n\n");
        pthread_cond_broadcast(&empty);
    }

    pthread_mutex_unlock(&mutex); 

    return result;
}

void *produtor(threadStruct *param){

    int i;

    //Como pode ter mais de uma thread consumidora, se produz numElements * threadsConsumidoras itens.
    //Portanto, isso é feito para não faltar itens para as threads.
    for(i = 0; i < numElements * threadsConsumidoras; i++){
        
        //+1 elemento no buffer.
        putBlockingQueue(param, (param->threadID * numElements * threadsConsumidoras) + i + 1);
    }
    free(param);
    pthread_exit(NULL);
}

void *consumidor(threadStruct* param){
    
    int i;

    //Como pode ter mais de uma thread produtora, se consome numElements * threadsProdutoras itens.
    for(i = 0; i < numElements * threadsProdutoras; i++){
        
        //-1 elemento no buffer
        int aux = takeBlockingQueue(param);
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

    //Array de threads.
    pthread_t *consumer = (pthread_t *) malloc(threadsConsumidoras * sizeof(pthread_t)); 
    pthread_t *producer = (pthread_t *) malloc(threadsProdutoras * sizeof(pthread_t));

    int j;
    //Cria as threads produtoras.
    for(j = 0; j < threadsProdutoras; j++){
        threadStruct *aux = (threadStruct *) malloc(sizeof(threadStruct));
        aux->buffer = fila;
        aux->threadID = j;
        pthread_create(&producer[j], NULL, (void *) produtor, (void *) aux);
    } 

    // ria as threads consumidoras.
    for(j = 0; j < threadsConsumidoras; j++){
        threadStruct *aux = (threadStruct *) malloc(sizeof(threadStruct));
        aux->buffer = fila;
        aux->threadID = j;
        pthread_create(&consumer[j], NULL, (void *) consumidor, (void *) aux);
    }

    //Esperar o término de execução das threads
    for(j = 0; j < threadsProdutoras; j++){
        pthread_join(producer[j], NULL);
    } 

    for(j = 0; j < threadsConsumidoras; j++){
        pthread_join(consumer[j], NULL);
    } 

    //Libera memória alocada dinamicamente.
    free(consumer);
    free(producer);
    clear(fila);

    pthread_exit(NULL);
}