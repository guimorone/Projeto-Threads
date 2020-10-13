#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <string.h>

char colores[][7] = {"\e[40m", "\e[41m",
"\e[42m", "\e[43m", "\e[44m", "\e[45m", "\e[46m",
"\e[47m"};

char **tela;

pthread_mutex_t *mymutex;

int L, N; //L = linhas totais, N = quantos arquivos

typedef struct{
  int linha;
  char msg[50];
} insertMsg;


// Função pra executar todas as threads 
void *myThreadFun(void *vargp) 
{ 
  insertMsg cmd= *((insertMsg*) vargp);

  pthread_mutex_lock((mymutex+cmd.linha-1));

  tela[(cmd.linha)-1] = (char*) malloc(strlen(cmd.msg));
  strcpy(tela[(cmd.linha)-1], cmd.msg);

  pthread_mutex_unlock((mymutex+cmd.linha-1));

  return NULL;
} 

void createThread(int linha, char* texto,pthread_t* thread){
  insertMsg* cmd = (insertMsg *) malloc(sizeof(insertMsg));
  cmd->linha=linha;
  strcpy((cmd->msg), texto);

  int resp = pthread_create(thread, NULL, myThreadFun,(void *) cmd);
  if(resp){
        printf("Erro na criação de thread!\n");
        return;
  }

}

int main() {

    printf("Digite o numero de arquivos e linhas: ");
    scanf("%d %d", &N, &L);

    pthread_t *thread = (pthread_t *) malloc(L*sizeof(pthread_t));
    tela = (char**) malloc(L*sizeof(char*));
    mymutex = (pthread_mutex_t *) malloc(L*sizeof(pthread_mutex_t));

    for (int i = 0; i < L; i++) {
        tela[i] = "undef"; // random
        pthread_mutex_init(&mymutex[i], NULL);
    };

    int count = 0; //contas threads

    for(int i = 0; i < N; i++){
        char str[50];
        printf("Digite o nome do arquivo com sua extensão (.txt): ");
        scanf(" %[^\n]", str);
        FILE *arq = fopen(str, "r");
        if(arq == NULL){
            printf("ERRO\n");
            exit(-1);
        }

        int linha;
        char mensagem[100];

        while(fscanf(arq, "%d\n %[^\n]", &linha, mensagem) != EOF){
            createThread(linha, mensagem, &(thread[count++]));
        }

        int i;
        for(i = 0; i < L; i++){
            printf("%s", colores[i%8]);
            printf("%s\n",tela[i]);
            printf("\e[0m");
        }

        printf("\n");
        fclose(arq); 
    }

    pthread_exit(NULL);

    return 0;
}