#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h> 
#include <pthread.h>

int qtdThreads;
int qntRequisicoes;
int id = -1;
//vetor para guardar resultados das operaçoes
int* readyResults;


pthread_t despachante; 

//mutexes para bloquear resultados que ainda nao estao prontos
pthread_mutex_t* resultMutexes;
//mutex e cond para despachante esperar por mais requisicoes ou aguardar se todas as threads estiverem ocupadas
pthread_mutex_t mutexDespachante = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t emptyQueue = PTHREAD_COND_INITIALIZER;
pthread_cond_t threadDisponivel = PTHREAD_COND_INITIALIZER;


typedef struct structAgendar{
  int param;
  void* funexec;
} structAgendar;

typedef struct requisicao{
  int param;
  void* funexec;
  int id;
} requisicao;

//struct para guardar informacoes adicionais com relacao as threads como estado de uso e id
typedef struct threadWrapper{
  pthread_t thread;
  int threadID;
  int occupied;
} threadWrapper;

//struct para a despachante se comunicar com a thread
typedef struct threadFunctionStruct{
  int threadID;
  requisicao req;
} threadFunctionStruct;

//structs para implementacao de lista encadeada
typedef struct elem{
  requisicao value;
  struct elem *prox;
} Elem;

typedef struct queue{
  unsigned int statusBuffer;
  Elem *head,*last;
} Queue;

threadWrapper* threads; // threads worker

//buffer onde ficarao as requisicoes até que sejam mandadas para execucao pela despachante
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


//buscar dentre as threads se alguma delas acabou sua execucao anterior e se encontra livre
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
  //bloqueia a retirada de um resultado que ainda nao está pronto
  pthread_mutex_lock(&resultMutexes[id]);
  //sinaliza para despachante que tem novas requisicoes na fila
  pthread_cond_broadcast(&emptyQueue);
  return id;
}

void threadFunctionWrapper(void* param){
  //executa a funcao do usuario e guarda o retorno
  threadFunctionStruct params = *((threadFunctionStruct*) param);
  int (*fun)(int) = params.req.funexec;
  int result = fun(params.req.param);
  readyResults[params.req.id]=result;
  //desbloqueia o usuario que quiser pegar essa informaçao
  pthread_mutex_unlock(&resultMutexes[params.req.id]);
  //sinaliza que essa thread nao está mais ocupada
  threads[params.threadID].occupied = false;
  free(param);
  pthread_cond_broadcast(&threadDisponivel);
}

void despachanteFunc(){
  int i;
  pthread_mutex_lock(&mutexDespachante);
  for(i = 0; i < qntRequisicoes; i++){
    //se nao tiverem requisicoes na fila dorme
    while(buffer->statusBuffer==0) pthread_cond_wait(&emptyQueue, &mutexDespachante);
    //retira uma nova requisicao da fila
    requisicao req = retirarElem();
    //espera ter uma thread livre
    int threadID = findFreeThread();
    while(threadID==-1){
        pthread_cond_wait(&threadDisponivel, &mutexDespachante);
        threadID = findFreeThread();
    }
    //sinaliza que a nova thread está ocupada
    threads[threadID].occupied = true;
    threadFunctionStruct* threadParams= (threadFunctionStruct *) malloc(sizeof(threadFunctionStruct));
    threadParams->threadID = threadID;
    threadParams->req = req;
    //chama o function wrapper usando a thread livre
    pthread_create(&threads[threadID].thread, NULL, (void*)threadFunctionWrapper, (void*) threadParams);
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

  //cria thread despachante
  pthread_create(&despachante, NULL, (void *) despachanteFunc, NULL);
  //inicializa threads worker
  threads = (threadWrapper *) malloc(qtdThreads * sizeof(threadWrapper));
  //inicializa mutexes
  resultMutexes = (pthread_mutex_t *) malloc(qntRequisicoes * sizeof(pthread_mutex_t));
  int i;
  for(i = 0; i < qtdThreads; i++) pthread_mutex_init(&resultMutexes[i], NULL);
  //inicializa o vetor de resultados
  readyResults = (int *) malloc(qntRequisicoes * sizeof(int));
}


void clear(){
  //libera buffer
  //libera threads worker
  //libera mutexes
  while(buffer->statusBuffer > 0){
      requisicao aux = retirarElem();
  }

  free(buffer);
  free(resultMutexes);
  free(readyResults);
  free(threads);
}

//funcoes usadas para execucao
int funexec1(int a){
  return ++a;
}

int funexec2(int a){
  return 2*a;
}

int main() {

  printf("Digite a quantidade de processadores ou núcleos do sistema computacional: ");
  scanf("%d", &qtdThreads);
  printf("Digite a quantidade de requisicões: ");
  scanf("%d", &qntRequisicoes);

  //inicializar o servidor
  init();
  int i;
  for(i = 0; i < qntRequisicoes; i++){
    structAgendar novaReq;
    if(i%2) novaReq.funexec = (void *) &funexec1;
    else novaReq.funexec = (void *) &funexec2;
    novaReq.param = i;
    agendarExecucao(novaReq);
  }

  for(i = 0; i < qntRequisicoes; i++){
    int resultado = pegarResultadoExecucao(i);
    printf("Resultado %d: %d\n", i, resultado);
  }
  pthread_join(despachante, NULL);

  clear();

  pthread_exit(NULL);
}