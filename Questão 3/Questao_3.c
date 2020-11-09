#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <string.h>

//Struct que será utilizada para passar dados como parâmetro para a função func
//A ideia é que o componente char *sub seja uma substring da string s1, ao passo
//que o componente char *s2 seja a string s2 dada pelo usuário.
typedef struct {
    char *sub;
    char *s2;
} msg;

//Variável responsável por guardar o resultado final, ou seja, a quantidade de
//ocorrências de s2 em s1.
int count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Função passada como parâmetro na criação das threads. Ela recebe uma estrutura
//do tipo "cmd". Assim, ela calculará quantas ocorrências da substring s2 existem
//na componente char *sub (componente da struct), que é uma substring da string s1.
//Essa quantidade será guardada na variável 'count', respeitando sua exclusão mútua.
void *func(void *cmd) {
    msg command = *((msg *) cmd);

    int tams2 = strlen(command.s2);
    int tamStr = strlen(command.sub);

    int i, j=0, k;

    //Cálculo das ocorrências.
    for(i=0; i<tamStr; i++) {
        if(command.sub[i] == command.s2[0]) {
            int ok = 0;
            for(k=0; k<tams2 && k+i<tamStr; k++) {
                ok = 1;
                if(command.sub[i+k] != command.s2[k]) {
                    ok = 0;
                    break;
                }
            }
            printf("%d\n", k);
            //Se ocorre, incrementa 'count' respeitando a exclusão mútua.
            if(ok && k == tams2) {
                pthread_mutex_lock(&mutex);
                count++;
                pthread_mutex_unlock(&mutex);
            }
        }
    }
}

//Função responsável por criar uma substring de uma string qualquer passada como argumento.
//É necessário passar mais 2 argumentos, a posição inicial e o tamanho dessa substring a ser criada.
char *substr(char str[], int pos, int length) {
    if(pos + length > strlen(str)) return NULL;

    char *sub = (char *) malloc(length);
    int i = 0;
    while (i < length) {
        sub[i] = str[pos+i];
        i++;
    }
    sub[i] = '\0';

    return sub;
}

int main() {

    char s1[100], s2[100];

    printf("Digite as string s1 e s2: ");
    scanf(" %s %s", s1, s2);
 
    //Guarda-se o tamanho de s1 e s2.
    int n1 = strlen(s1);
    int n2 = strlen(s2);

    int p, var=1;
    
    //Aqui é estabelecido o valor 'p' correspondente à quantidade de threads a serem criadas,
    //respeitando as condições dadas no enunciado da questão.
    while(var <= n1/2) {
        if(n1 % var == 0 && n1 > var * n2) p = var;
        var++;
    }

    //Array de 'p' threads é criado.
    pthread_t *threads = (pthread_t *) malloc(p*sizeof(pthread_t));
    
    //Array de 'p' structs 'msg' é criado. 
    msg *vetMsg = (msg *) malloc(p*sizeof(msg));

    int i, j=0;
    
    //Aqui é feita a separação da string s1 em suas substrings que serão passadas como
    //argumento juntamente com a string s2. De acordo com a política da questão, s1 é
    //dividida em 'p' pedaços de tamanho 'n1/p'.
    for(i=0; i<p; i++) {
        char *sub = substr(s1, j, n1/p);

        //Os dados são passados para a i-ésima posição do array de 'msg', que será
        //passado como argumento na criação da thread. 
        vetMsg[i].s2 = (char *) malloc(n2);
        vetMsg[i].sub = (char *) malloc(strlen(sub));
        strcpy(vetMsg[i].s2, s2);
        strcpy(vetMsg[i].sub, sub);
        printf("vetmsg[%d].sub = %s\n", i, vetMsg[i].sub);
        printf("vetmsg[%d].s2 = %s\n", i, vetMsg[i].s2);

        int rc = pthread_create(&threads[i], NULL, func, (void *) &vetMsg[i]);
        if(rc) printf("ERRO na criacao da thread[%d], codigo de retorno: %d\n", i, rc);

        j += n1/p;

        free(sub);
    }
    
    //O join é utilizado para que a main aguarde a finalização da execução das 'p' threads criadas.
    for(i=0; i<p; i++) {
        pthread_join(threads[i], NULL);
        printf("esperando a thread[%d]\n", i);
    }
    
    //Desse modo, é garantida a corretude do valor final de 'count'. 
    printf("count = %d\n", count);
    
    //Liberação da memória alocada dinamicamente.
    free(threads);
    free(vetMsg);
    
    pthread_exit(NULL);
    return 0;
}
