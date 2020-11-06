#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
  
// Contador 
int count = 0; 

int MAXNUM = 1000000;

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

  
//Função de incrementar o contador. 
void *inc(void *id) 
{ 

    int tid = *((int *)id);
    /* Incrementa o valor do contador até chegar em seu valor máximo (1.000.000).
    Utilização de mutex e funções lock/unlock para garantir a exclusão mútua relacionada ao contador.*/
    while(count < MAXNUM){
        pthread_mutex_lock(&mymutex);
        if(++count == MAXNUM) {
            printf("Thread %ld chegou ao numero.\n", (long) tid);
            pthread_mutex_unlock(&mymutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&mymutex);
    }
}
  
int main() 
{ 
    int i, n; 

    printf("Digite o valor de N para quantas threads incrementaram o contador: ");
    scanf("%d",&n);

    //Cria-se array de threads.
    pthread_t *threads = (pthread_t *) malloc(n*sizeof(pthread_t));

    //Cria-se array de inteiros que será utilizado para identificar cada thread criada.
    int *threadsId = (int *) malloc(n*sizeof(int));
  
    /* Criam-se n threads de acordo com a função inc, cujo parâmetro
    corresponde ao respectivo threadsId[i].*/
    for (i = 0; i < n; i++) {
        threadsId[i] = i;
        int rc = pthread_create(&threads[i], NULL, inc, (void *) &threadsId[i]);
        if(rc) {
            printf("ERRO na criação da thread[%d], código de retorno: %d\n", threadsId[i], rc);
            exit(-1);
        }
    }

    //Uso do join para esperar a finalização da execução de todas as threads criadas.
    for(i=0; i<n; i++) {
        pthread_join(threads[i], NULL);
    }

    //Liberação de memória alocada dinamicamente.
    free(threadsId);
    free(threads);

    pthread_exit(NULL);
    return 0; 
}
