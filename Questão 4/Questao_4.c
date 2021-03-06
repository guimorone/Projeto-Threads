#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h> 
#include <pthread.h>

int qtdThreads;
int qntRequisicoes;
int id = -1;

//Vetor para guardar resultados das operaçoes.
int* readyResults;


pthread_t despachante; 

//Mutexes para bloquear resultados que ainda nao estão prontos.
pthread_mutex_t* resultMutexes;

//Mutex e cond para despachante esperar por mais requisições ou aguardar se todas as threads estiverem ocupadas.
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

//Struct para guardar informações adicionais com relação às threads como estado de uso e id.
typedef struct threadWrapper{
  pthread_t thread;
  int threadID;
  int occupied;
} threadWrapper;

//Struct para a despachante se comunicar com a thread.
typedef struct threadFunctionStruct{
  int threadID;
  requisicao req;
} threadFunctionStruct;

//Structs para implementação de lista encadeada.
typedef struct elem{
  requisicao value;
  struct elem *prox;
} Elem;

typedef struct queue{
  unsigned int statusBuffer;
  Elem *head,*last;
} Queue;

//Threads trabalhadoras.
threadWrapper* threads;

//Buffer onde ficarão as requisições até que sejam mandadas para execução pela despachante.
Queue *buffer;

//Adicionar elemento ao final da fila.
void adicionarElem(requisicao req) {

  Elem *aux = (Elem *) malloc(sizeof(Elem));
  aux->value = req;
  aux->prox = NULL;

  //Se o buffer for vazio, 'aux' é o primeiro elemento da fila.
  if(buffer->statusBuffer == 0) {
    buffer->head = aux;
    buffer->last = aux;
  } 
  else {
    //O elemento que antes era o último agora vai apontar para o novo último 'aux'.
    buffer->last->prox = aux;

    //'aux' vira o último elemento.
    buffer->last = aux;
  }

  buffer->statusBuffer++;
}

//Retira o primeiro elemento da fila.
requisicao retirarElem() {
  if(buffer->statusBuffer == 0){
    //Buffer vazio.
    //Já será verificado antes, mas só para ter certeza.
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


//Buscar dentre as threads se alguma delas acabou sua execução anterior e se encontra livre
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

  //Bloqueia a retirada de um resultado que ainda nao está pronto.
  pthread_mutex_lock(&resultMutexes[id]);

  //Sinaliza para despachante que têm novas requisições na fila.
  pthread_cond_broadcast(&emptyQueue);

  return id;
}

void threadFunctionWrapper(void* param){
  
  //Executa a função do usuário e guarda o retorno.
  threadFunctionStruct params = *((threadFunctionStruct*) param);

  int (*fun)(int) = params.req.funexec;
  int result = fun(params.req.param);

  readyResults[params.req.id] = result;
  
  //Desbloqueia o usuário que quiser pegar essa informação.
  pthread_mutex_unlock(&resultMutexes[params.req.id]);

  //Sinaliza que essa thread nao está mais ocupada
  threads[params.threadID].occupied = false;

  free(param);
  pthread_cond_broadcast(&threadDisponivel);
}

void despachanteFunc(){
  int i;
  pthread_mutex_lock(&mutexDespachante);
  for(i = 0; i < qntRequisicoes; i++){

    //Se não tiverem requisições na fila, dorme.
    while(buffer->statusBuffer==0) pthread_cond_wait(&emptyQueue, &mutexDespachante);

    //Retira uma nova requisição da fila.
    requisicao req = retirarElem();

    //Espera ter uma thread livre.
    int threadID = findFreeThread();
    while(threadID==-1){
        pthread_cond_wait(&threadDisponivel, &mutexDespachante);
        threadID = findFreeThread();
    }

    //Sinaliza que a thread que seŕa utilizada está agora ocupada.
    threads[threadID].occupied = true;
    threadFunctionStruct* threadParams= (threadFunctionStruct *) malloc(sizeof(threadFunctionStruct));
    threadParams->threadID = threadID;
    threadParams->req = req;

    //Chama 'threadFunctionWrapper' usando a thread preparada.
    pthread_create(&threads[threadID].thread, NULL, (void*)threadFunctionWrapper, (void*) threadParams);
  }
}

int pegarResultadoExecucao(int searchID){

    //Fica bloqueado até que o mutex daquela requisição específica seja desbloqueado.
    pthread_mutex_lock(&resultMutexes[searchID]);
    return readyResults[searchID];
}

void init(){

  //Inicializa buffer.
  buffer = (Queue *) malloc(sizeof(Queue));
  buffer->statusBuffer = 0;
  buffer->head = NULL;
  buffer->last = NULL;

  //Cria thread despachante.
  pthread_create(&despachante, NULL, (void *) despachanteFunc, NULL);

  //Inicializa threads trabalhadoras.
  threads = (threadWrapper *) malloc(qtdThreads * sizeof(threadWrapper));
  
  //Inicializa mutexes.
  resultMutexes = (pthread_mutex_t *) malloc(qntRequisicoes * sizeof(pthread_mutex_t));
  
  int i;
  for(i = 0; i < qtdThreads; i++) pthread_mutex_init(&resultMutexes[i], NULL);
  //Inicializa o vetor de resultados.
  readyResults = (int *) malloc(qntRequisicoes * sizeof(int));
}


void clear(){

  //Libera buffer, threads trabalhadoras e mutexes.
  while(buffer->statusBuffer > 0){
      requisicao aux = retirarElem();
  }

  free(buffer);
  free(resultMutexes);
  free(readyResults);
  free(threads);
}

//Funções usadas para execução.
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

  //Inicializar servidor.
  init();

  //Criação de requisições e agendamento.
  int i;
  for(i = 0; i < qntRequisicoes; i++){
    structAgendar novaReq;
    if(i%2) novaReq.funexec = (void *) &funexec1;
    else novaReq.funexec = (void *) &funexec2;
    novaReq.param = i;
    agendarExecucao(novaReq);
  }

  //Pegar os resultados das requisições e printar na tela.
  for(i = 0; i < qntRequisicoes; i++){
    int resultado = pegarResultadoExecucao(i);
    printf("Resultado %d: %d\n", i, resultado);
  }
  
  pthread_join(despachante, NULL);

  clear();

  pthread_exit(NULL);
}