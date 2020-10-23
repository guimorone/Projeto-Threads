#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <string.h>

typedef struct {
    int size;
    int *arr;
} msg;

pthread_barrier_t barrier;

int matA[5][5] = {17,  2,  1,  5,  6,
                   3,  7,  2,  0,  1,
                   8,  4, 19,  6,  0,
                   9, 10,  3, 25,  2,
                   1,  5,  6, 11, 39};

int vetB[5] = {2, 3, 5, 7, 11};

double **incognitas;

int P;
int N;

void *func(void *command) {
    msg cmd = *((msg *) command);
    
    int k, u, i;
    for(k=0; k<P; k++) {
        for(u=0; u<cmd.size; u++) {
            i = cmd.arr[u];
            double soma=0;
            int j;

            for(j=0; j<5; j++) {
                if(j != i) soma += ((double) matA[i][j]) * incognitas[j][k];
            }

            incognitas[i][k+1] = (1 / ((double) matA[i][i])) * (((double)vetB[i]) - soma);
        }

        pthread_barrier_wait(&barrier);
    }

}




int main() {

    int u, v, w=0;

    printf("Digite a quantidade N de threads (0 < N <= 4) a serem executadas: ");
    scanf("%d", &N);

    while(0 >= N || N > 4) {
        printf("Valor invalido para N. Digite novamente: ");
        scanf("%d", &N);
    }

    printf("Digite o valor P para a quantidade de iteracoes que refinarao o resultado: ");
    scanf("%d", &P);
    printf("\n");
    
    incognitas = (double **) malloc(5*sizeof(double *));
    for(u=0; u<5; u++) {
        incognitas[u] = (double *) malloc((P+1)*sizeof(double));
        incognitas[u][0] = 1;
    }

    pthread_t *threads = (pthread_t *) malloc(N*sizeof(pthread_t));
    msg *param = (msg *) malloc(N*sizeof(msg));


    int qtdIncog = 5/N;
    for(u=0; u<N; u++) {
        if(u+1 == N) {
            param[u].arr = (int *) malloc((qtdIncog + (5 % N))*sizeof(int));
            param[u].size = qtdIncog + (5% N);
            for(v=0; v<(qtdIncog + (5 % N)); v++) {
                param[u].arr[v] = w;
                w++;
            }
        }
        else {
            param[u].arr = (int *) malloc(qtdIncog*sizeof(int));
            param[u].size = qtdIncog;
            for(v=0; v<qtdIncog; v++) {
                param[u].arr[v] = w;
                w++;
            }
        }
    }
    
    pthread_barrier_init(&barrier, NULL, N);

    for(u=0; u<N; u++) {
        int rc = pthread_create(&threads[u], NULL, func, (void *) &param[u]);
        if(rc) printf("Erro ao criar thread[%d], codigo de retorno: %d\n", u, rc);
    }

    for(u=0; u<N; u++) {
        pthread_join(threads[u], NULL);
        //printf("Esperando thread[%d]\n", u);
    }

    printf("Para o sistema linear abaixo:\n");
    printf("(17)*x1 +  (2)*x2 +  (1)*x3 +  (5)*x4 +  (6)*x5 =  2\n");
    printf(" (3)*x1 +  (7)*x2 +  (2)*x3 +  (0)*x4 +  (1)*x5 =  3\n");
    printf(" (8)*x1 +  (4)*x2 + (19)*x3 +  (6)*x4 +  (0)*x5 =  5\n");
    printf(" (9)*x1 + (10)*x2 +  (3)*x3 + (25)*x4 +  (2)*x5 =  7\n");
    printf(" (1)*x1 +  (5)*x2 +  (6)*x3 + (11)*x4 + (39)*x5 = 11\n\n");
    printf("Os valores de Xi, na P-esima iteracao, que satisfazem tal sistema de equacoes sao:\n\n");
    for(u=0; u<5; u++) {
        printf("X%d = %.7lf\n", u+1, incognitas[u][P]);
    }
    printf("\n");

    return 0;
}