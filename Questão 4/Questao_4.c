#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h> 
#include <pthread.h>

#define NUM_REQ 500 // maximo, mas não o que tem na fila
#define qtdThreads 15 // número de núcleos/processadores do SO

int statusQueue = 0, reqsRestantes = NUM_REQ, readyResults[NUM_REQ] = {0};

int funexec(int a){
  reqsRestantes--;
  return a++;
}

typedef struct requisicao{
  int param;
  void* funexec;
  int id;
} requisicao;

typedef struct threadWrapper{
  pthread_t thread;
  int threadID;
  bool occupied;
} threadWrapper;

typedef struct threadFunctionStruct{
  int threadID;
  requisicao req;
} threadFunctionStruct; // só para função de executar a thread

requisicao Queue[NUM_REQ];

pthread_t despachante;
threadWrapper threads[qtdThreads];

pthread_mutex_t mutexDespachante = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexThreads = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t emptyQueue = PTHREAD_COND_INITIALIZER;

void adicionarElem(requisicao q){
  if(statusQueue >= NUM_REQ){
    exit(-1);
  }

  Queue[statusQueue] = q;
  statusQueue++;
}

requisicao retirarElem(){
  if(!statusQueue){
    exit(-1);
  }  

  requisicao aux = Queue[statusQueue - 1];
  statusQueue--;

  return aux;
}

void agendarExecucao(int id){
  requisicao novaReq;
  novaReq.param = id;
  novaReq.funexec = (void *) funexec;
  novaReq.id = id;
  adicionarElem(novaReq);
  if(statusQueue == 1){
    pthread_cond_broadcast(&emptyQueue);
  }
}

void *threadFunctionWrapper(threadFunctionStruct params){
  pthread_mutex_lock(&mutexThreads);
  //executa a funcao do usuario e guarda o retorno
  int (*fun)(int) = params.req.funexec;
  int result = fun((int)params.req.param);
  readyResults[params.req.id] = result;
  //desbloqueia o usuario que quiser pegar essa informaçao
  //sinaliza que essa thread nao está mais ocupada
  threads[params.threadID].occupied = false;
  pthread_mutex_unlock(&mutexThreads);
  pthread_exit(NULL);
}

int findFreeThread(){
  int i;
  for(i=0; i < qtdThreads; i++){
    if(threads[i].occupied == false){
      return i;
    }
  }
  return -1;
}

void *despachanteFunc(){
  pthread_mutex_lock(&mutexDespachante);
  while(reqsRestantes){
    while(!statusQueue){
      pthread_cond_wait(&emptyQueue, &mutexDespachante);
    }

    requisicao command = retirarElem();
    int threadID;
    do {
      threadID = findFreeThread();
    } while(threadID == -1);

    threads[threadID].occupied = true;
    pthread_create(&threads[threadID].thread, NULL, (void *) threadFunctionWrapper, (void *) &command);
    pthread_join(threads[threadID].thread, NULL);
  }
  pthread_mutex_unlock(&mutexDespachante);
  pthread_exit(NULL);
}

int main(){

  int i;
  pthread_create(&despachante, NULL, (void *) despachanteFunc, NULL);
  for(i = 0; i < qtdThreads; i++){
    threads[i].occupied = false;
    threads[i].threadID = i;
  }

  for(i = 0; i < NUM_REQ; i++){
    agendarExecucao(i);
  }

  while(reqsRestantes){
    for(i = 0; i < NUM_REQ; i++){
      if(!readyResults[i]){
        printf("Requisição %d ainda não foi executada!\n", i);
      } else{
        printf("Resultado da requisição %d = %d\n", i, readyResults[i]);
      }
    }
  }

  pthread_join(despachante, NULL);

  pthread_exit(NULL);
}