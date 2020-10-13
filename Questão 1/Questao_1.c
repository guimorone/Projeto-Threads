#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
  
// Contador 
int count = 0; 

int MAXNUM = 1000000;

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

  
// função de incrementar o contador 
void *inc(void *id) 
{ 
  int tid = *((int *)id);
  // Incrementa o valor do contador até chegar em seu valor máximo (1.000.000)
  // Utilização de mutex para alternar as threads que incrementam o contador
  // LOCK e UNLOCK-> NÃO DEIXA QUE OUTRAS THREADS ALTEREM A VARIÁVEL count SIMULTANEAMENTE
  
  while(count < MAXNUM){
    pthread_mutex_lock(&mymutex);
    if(++count == MAXNUM) {
      printf("Thread %ld chegou ao número.\n", (long) tid);
      pthread_mutex_unlock(&mymutex);
      pthread_exit(NULL);
    }
    pthread_mutex_unlock(&mymutex);
  }
  return NULL;
} 
  
int main() 
{ 
    int i, n; 

    printf("Digite o valor de N para quantas threads incrementaram o contador: ");
    scanf("%d",&n);

    pthread_t *threads = (pthread_t *) malloc(n*sizeof(pthread_t)); //cria array de threads
    int **threadsId = (int **) malloc(n*sizeof(int *));//cria array de ponteiros para id das threads
  
    // Cria N threads 
    for (i = 0; i < n; i++) {
      threadsId[i] = (int *) malloc(sizeof(int));
      *(threadsId[i]) = i;
      int rc = pthread_create(&threads[i], NULL, inc, (void *)threadsId[i]);

      if(rc) {
        printf("ERRO na criação da thread[%d], código de retorno: %d\n", *(threadsId[i]), rc);
        exit(-1);
      }
    }

    pthread_exit(NULL);
    for(i=0; i<n; i++) {
      free(threadsId[i]);
    }
    free(threadsId);
    free(threads);

    return 0; 
}