#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include <string.h> 

typedef struct {
    int R, G, B;
    int C;
    int i, j;
} rgb;

rgb **mat;

void *func(void *cmd) {
    rgb command = *((rgb *) cmd);

    int C = (command.R * 0.30) + (command.G * 0.59) + (command.B * 0.11);
    mat[command.i][command.j].C = C;
}

void init(int l, int c) {
    mat = (rgb **) malloc(l*sizeof(rgb *));
    if(mat == NULL) {
        printf("erro na alocacao\n");
        exit(-1);
    }
    int i, j;
    for(i=0; i<l; i++) {
        mat[i] = (rgb *) malloc(c*sizeof(rgb));
        if(mat[i] == NULL) {
            printf("erro na alocacao %d\n", i);
            exit(-1);
        }
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

    if(!(strcmp(formato, "P3"))) {
        fscanf(arq, "%d %d", c, l);
        fscanf(arq, "%d", max);
        init(*l, *c);

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

    for(i=0; i<(*l); i++) {
        for(j=0; j<(*c); j++) {
            printf("%d %d %d %d //", mat[i][j].R, mat[i][j].G, mat[i][j].B, mat[i][j].C);
        }
        printf("\n");
    }
}

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

    fprintf(arq, "%s\n%d %d\n%d\n", "P3", c, l, max);

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
    leArquivo(&linha, &coluna, &max);

    int k=0, i, j;

    rgb *msg = (rgb *) malloc((linha*coluna)*sizeof(rgb));
    pthread_t *threads = (pthread_t *) malloc((linha*coluna)*sizeof(pthread_t));

    for(i=0; i<linha; i++) {
        for(j=0; j<coluna; j++) {
            msg[k].R = mat[i][j].R;
            msg[k].G = mat[i][j].G;
            msg[k].B = mat[i][j].B;
            msg[k].C = mat[i][j].C;
            msg[k].i = mat[i][j].i;
            msg[k].j = mat[i][j].j;

            int rc = pthread_create(&threads[k], NULL, func, (void *) &msg[k]);
            if(rc) printf("Erro na criacao da thread[%d], codigo de retorno: %d\n", i, rc);
            k++;

        }
    }

    for(i=0; i<(linha*coluna); i++) {
        pthread_join(threads[i], NULL);
        printf("Esperando thread[%d]\n", i);
    }

    escreveArquivo(linha, coluna, max);

    free(msg);
    free(threads);
    for(i=0; i<linha; i++) {
        free(mat[i]);
    }
    free(mat);

    return 0;
}