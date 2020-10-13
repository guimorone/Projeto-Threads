#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
  
// Contador 
int count = 0; 

int MAXNUM = 1000000;

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

  
// função de incrementar o contador 
void *myThreadFun(void *vargp) 
{ 
    // Incrementa o valor do contador até chegar em seu valor máximo (1.000.000)

    // Utilização de mutex para alternar as threads que incrementam o contador
    // LOCK -> NÃO DEIXA OUTRAS THREADS ENTRAREM
    // UNLOCK -> DESBLOQUEIA O MUTEX
    
    while(count < MAXNUM){
      pthread_mutex_lock(&mymutex);
      if(++count == MAXNUM) {
        printf("Thread %ld chegou ao número.\n", (long) vargp);
        pthread_mutex_unlock(&mymutex);
        pthread_exit(NULL);
      }
      pthread_mutex_unlock(&mymutex);
    }

    return NULL;
} 
  
int main() 
{ 
    int i; 
    pthread_t tid; 
    int n;

    printf("Digite o número de threads: ");
    scanf("%d",&n);
  
    // Cria N threads 
    for (i = 0; i < n; i++) pthread_create(&tid, NULL, myThreadFun, (void *)(i+1)); 

    pthread_exit(NULL);

    return 0; 
}