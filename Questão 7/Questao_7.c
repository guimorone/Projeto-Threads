#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <string.h> 

//Struct responsável por guardar os valores RGB bem como seu correspondente C.
//Além disso, a estrutura guarda valores [i][j] relacionados à posição do pixel
//na matriz.
typedef struct {
    int R, G, B;
    int C;
    int i, j;
} rgb;

//Variável responsável por guardar as informações do arquivo *.ppm de entrada
//em estruturas do tipo 'rgb' para cada pixel da matriz representada por esse arquivo.
rgb **mat;

//Função passada como argumento na criação das threads. Ela recebe como argumento
//uma estrutura do tipo 'rgb'. Essa função apenas atualiza o valor de C corres-
//pondente à estrutura parâmetro e o salva.
void *func(void *cmd) {
    rgb command = *((rgb *) cmd);

    int C = (command.R * 0.30) + (command.G * 0.59) + (command.B * 0.11);
    mat[command.i][command.j].C = C;
}


//Função inicializadora. Alocação e inicializaçãoda variável 'mat' de acordo
//com as dimensões da matriz determinada em "void leArquivo(int *l, int *c, int *max)".
void init(int l, int c) {
    mat = (rgb **) malloc(l*sizeof(rgb *));

    int i, j;
    for(i=0; i<l; i++) {
        mat[i] = (rgb *) malloc(c*sizeof(rgb));

        for(j=0; j<c; j++) {
            mat[i][j].R = 0;
            mat[i][j].G = 0;
            mat[i][j].B = 0;
            mat[i][j].C = 0;
            mat[i][j].i = 0;
            mat[i][j].j = 0;
        }
    }
}

//Função responsável pela leitura de um arquivo entrada do tipo *.ppm. Os dados lidos
//são salvos em 'mat'.
void leArquivo(int *l, int *c, int *max) {
    int i, j;

    char str[50];
    printf("Digite o nome do arquivo no formato *.ppm: ");
    scanf(" %[^\n]", str);

    FILE *arq = fopen(str, "r");
    if(arq == NULL) {
        printf("ERRO ao criar arquivo");
            exit(-1);
    }

    char formato[5];
    fscanf(arq, " %[^\n]", formato);
    
    //Prossegue se o formato for o adequado.
    if(!(strcmp(formato, "P3"))) {
        fscanf(arq, "%d %d", c, l);
        fscanf(arq, "%d", max);
        
        //Inicializa 'mat'.
        init(*l, *c);
        
        //Faz a leitura dos valores RGB do arquivo ao passo que salva esses dados em 'mat'.
        //Os valores de [i][j] são também salvos uma vez que serão necessários para organizar
        //a escrita do arquivo *.ppm de sáida.
        for(i=0; i< (*l); i++) {
            for(j=0; j< (*c); j++) {
                int r, g, b;
                fscanf(arq, "%d %d %d", &r, &g, &b);
                if((r >= 0 && r <= (*max)) && (g >= 0 && g <= (*max)) && (b >= 0 && b <= (*max))) {
                    mat[i][j].R = r;
                    mat[i][j].G = g;
                    mat[i][j].B = b;
                }
                mat[i][j].i = i;
                mat[i][j].j = j;
            }
        }
    }
    
    else printf("Erro na formatacao do arquivo ppm.\n");
    fclose(arq);
}

//Função responsável por escrever os valores de C, correspondentes aos respectivos
//RGB de cada pixel da matriz de entrada, em um arquivo de saída do tipo *.ppm.
//É válido ressaltar que o arquivo só é escrito após todas as threads finalizarem
//sua execução, garantido que todos os pixels tiveram seus valores de C calculados.
void escreveArquivo(int l, int c, int max) {
    int i, j;
    
    char str[50];
    printf("Qual o nome do arquivo de saida desejado? ");
    scanf(" %[^\n]", str);

    FILE *arq = fopen(str, "w");
    if(arq == NULL) {
        printf("ERRO ao criar arquivo");
            exit(-1);
    }
    
    //Escrita do formato, quantidade de linhas e colunas e valor máximo.
    fprintf(arq, "%s\n%d %d\n%d\n", "P3", c, l, max);
    
    //Escrita dos valores de C.
    for(i=0; i<l; i++) {
        for(j=0; j<c; j++) {
            fprintf(arq, "%d %d %d\n", mat[i][j].C, mat[i][j].C, mat[i][j].C);
        }
    }

    fclose(arq);

}

int main() {
    int max;
    int linha, coluna;
    
    //Leitura do arquivo de entrada, salvando as dimensões da matriz bem como o valor
    //RGB máximo para cada pixel.
    leArquivo(&linha, &coluna, &max);

    int k=0, i, j;
    
    //Cria-se array de estruturas do tipo 'rgb'. Esse array será utilizado para passar
    //as estruturas como parâmetro de 'func' na criação de cada thread.
    rgb *msg = (rgb *) malloc((linha*coluna)*sizeof(rgb));
    
    //Cria-se array de (linha*coluna) threads, ou seja, uma thread para tratar cada pixel da matriz.
    pthread_t *threads = (pthread_t *) malloc((linha*coluna)*sizeof(pthread_t));
    
    //Atualização do array 'msg' de acordo com os valores lidos e salvos em 'mat'.
    //A ideia é que as threads apenas modificarão os valores C, correspondentes a cada pixel,
    //da matriz 'mat'.
    for(i=0; i<linha; i++) {
        for(j=0; j<coluna; j++) {
            msg[k].R = mat[i][j].R;
            msg[k].G = mat[i][j].G;
            msg[k].B = mat[i][j].B;
            msg[k].C = mat[i][j].C;
            msg[k].i = mat[i][j].i;
            msg[k].j = mat[i][j].j;
            
            //Criação das threads.
            int rc = pthread_create(&threads[k], NULL, func, (void *) &msg[k]);
            if(rc) printf("Erro na criacao da thread[%d], codigo de retorno: %d\n", i, rc);
            k++;

        }
    }
    
    //Esperam-se todas as threads finalizarem sua execução para que todos os pixels tenham
    //seus respectivos valores C atualizados.
    for(i=0; i<(linha*coluna); i++) {
        pthread_join(threads[i], NULL);
    }
    
    //O resultado salvo em 'mat' é escrito em um arquivo de saída do tipo *.ppm.
    escreveArquivo(linha, coluna, max);
    
    //Liberação da memória alocada dinamicamente ao longo do código.
    free(msg);
    free(threads);
    for(i=0; i<linha; i++) {
        free(mat[i]);
    }
    free(mat);
    
    pthread_exit(NULL);
    return 0;
}
