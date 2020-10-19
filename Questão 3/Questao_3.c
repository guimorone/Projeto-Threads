#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <string.h>

typedef struct {
    char *sub;
    char *s2;
} msg;

int count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *func(void *cmd) {
    msg command = *((msg *) cmd);

    int tams2 = strlen(command.s2);
    int tamStr = strlen(command.sub);

    int i, j=0, k;
    for(i=0; i<tamStr; i++) {
        if(command.sub[i] == command.s2[0]) {
            int ok = 0;
            for(k=0; k<tams2 && k+i<tamStr; k++) {
                ok = 1;
                //printf("%s: command.sub[%d]=%c --- command.sub[%d]=%c\n", command.sub, k+i, command.sub[k+i], k, command.s2[k]);
                if(command.sub[i+k] != command.s2[k]) {
                    ok = 0;
                    break;
                }
            }
            if(ok) {
                pthread_mutex_lock(&mutex);
                count++;
                pthread_mutex_unlock(&mutex);
            }
        }
    }
}

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
 
    int n1 = strlen(s1);
    int n2 = strlen(s2);

    int p, var=1;
    
    while(var <= n1/2) {
        if(n1 % var == 0 && n1 > var * n2) p = var;
        var++;
    }

    pthread_t *threads = (pthread_t *) malloc(p*sizeof(pthread_t));
    msg *vetMsg = (msg *) malloc(p*sizeof(msg));

    int i, j=0;
    for(i=0; i<p; i++) {
        char *sub;
        if(i == p-1) {
            sub = substr(s1, j, strlen(s1)-j);
            //printf("%d %s\n", i, sub);
        }
        else {
            sub = substr(s1, j, n1/p);
            //printf("%d %s\n", i, sub);
        }

        vetMsg[i].s2 = (char *) malloc(strlen(s2));
        vetMsg[i].sub = (char *) malloc(strlen(sub));
        strcpy(vetMsg[i].s2, s2);
        strcpy(vetMsg[i].sub, sub);

        int rc = pthread_create(&threads[i], NULL, func, (void *) &vetMsg[i]);
        if(rc) printf("ERRO na criacao da thread[%d], codigo de retorno: %d\n", i, rc);

        j += n1/p;

        free(sub);
    }

    for(i=0; i<p; i++) {
        pthread_join(threads[i], NULL);
        printf("esperando a thread[%d]\n", i);
    }

    printf("count = %d\n", count);

    free(threads);
    free(vetMsg);
    return 0;
}