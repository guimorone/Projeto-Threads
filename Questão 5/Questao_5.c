#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <string.h>

typedef struct {
    int size;
    int *arr;
} msg;

pthread_barrier_t barrier;

//Matriz A de ordem 5.
int matA[5][5] = {17,  2,  1,  5,  6,
                   3,  7,  2,  0,  1,
                   8,  4, 19,  6,  0,
                   9, 10,  3, 25,  2,
                   1,  5,  6, 11, 39};

//Matriz B (5 x 1).
int vetB[5] = {2, 3, 5, 7, 11};

//Variável responsável por guardar os valores das 5 incógnitas para cada iteração.
//Cada linha dessa matriz representa uma das 5 incógnitas e cada coluna, o valor
//dessa incógnita em cada uma das P iterações.
double **incognitas;

//Quantidade de iterações que refinarão o resultado.
int P;

//Quantidade de threads a serem executadas.
int N;

//Função passada como argumento na criação das threads. Esta recebe uma struct 
//do tipo 'msg' que tem como componentes um array de inteiros e seu respectivo
//tamanho.
void *func(void *command) {
    msg cmd = *((msg *) command);
    
    //O algoritmo descrito na questão é executado, calculando o [k+1]-ésimo
    //valor de cada incógnita representada em 'cmd.arr' para cada uma das
    //'P' iterações determinadas pelo usuário.
    int k, u, i;
    for(k=0; k<P; k++) {
        for(u=0; u<cmd.size; u++) {
            i = cmd.arr[u];
            double soma=0;
            int j;

            for(j=0; j<5; j++) {
                if(j != i) soma += ((double) matA[i][j]) * incognitas[j][k];
            }
            
            //O resultado refinado em cada iteração é guardado na variável.
            incognitas[i][k+1] = (1 / ((double) matA[i][i])) * (((double)vetB[i]) - soma);
        }
        
        //Aqui é utilizado o mecanismo principal que permite o desenrolar do Método de Jacobi.
        //Cada uma das threads criadas só consegue prosseguir para a próxima iteração quando
        //todas as outras tiverem finalizado a iteração em curso. Isso é necessário para o
        //cálculo do algoritmo, uma vez que não há como calcular , por exemplo, o valor de uma
        //incógnita na [k+2]-ésima iteração sem haver calculado todas as outras na [k+1]-ésima
        //iteração.
        pthread_barrier_wait(&barrier);
    }

}




int main() {

    int u, v, w=0;

    printf("Digite a quantidade N de threads (0 < N <= 4) a serem executadas: ");
    scanf("%d", &N);

    //Tratador para direcionar o usuário a escolher um valor válido para uma máquina
    //com processador de 4 núcleos.
    while(0 >= N || N > 4) {
        printf("Valor invalido para N. Digite novamente: ");
        scanf("%d", &N);
    }

    printf("Digite o valor P para a quantidade de iteracoes que refinarao o resultado: ");
    scanf("%d", &P);
    printf("\n");
    
    //Inicializa-se a variável 'incognitas', alocando os devidos espaços de memória
    //para que todas as incognitas possam ser calculadas.
    incognitas = (double **) malloc(5*sizeof(double *));
    for(u=0; u<5; u++) {
        incognitas[u] = (double *) malloc((P+1)*sizeof(double));
        incognitas[u][0] = 1;
    }
    
    //Criação do array de 'N' threads.
    pthread_t *threads = (pthread_t *) malloc(N*sizeof(pthread_t));
    
    //Criação do array de 'N' structs do tipo 'msg'.
    msg *param = (msg *) malloc(N*sizeof(msg));

    /* Esse bloco de código (linha 105 até 123) é responsável por dividir as 5
    incógnitas em 'N' pedaços. Cada um desses pedaços, junto com seu tamanho,
    será empacotado em uma struct do tipo 'msg' que será passada como argumento
    de 'func' na criação de cada thread. De acordo com a política adotada, a
    última thread poderá ficar com mais incógnitas, caso (5 % N != 0).*/
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
    
    //Barreira é inicializada indicando que, para cada thread que chamar
    //a função "pthread_barrier_wait(&barrier)", esta dorme até que o número de
    //threads que dormiram nessa barreira totalize 'N'.
    pthread_barrier_init(&barrier, NULL, N);

    //Criam-se 'N' threads com seus respectivos parâmetros para 'func'.
    for(u=0; u<N; u++) {
        int rc = pthread_create(&threads[u], NULL, func, (void *) &param[u]);
        if(rc) printf("Erro ao criar thread[%d], codigo de retorno: %d\n", u, rc);
    }

    //Espera-se a finalização da execução das 'N' threads, para que seja garantido
    //o cálculo de todas as incógnitas com o refinamento desejado pelo usuário.
    for(u=0; u<N; u++) {
        pthread_join(threads[u], NULL);
    }
    
    //Os valores finais das 5 incógnitas são printados na tela.
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
