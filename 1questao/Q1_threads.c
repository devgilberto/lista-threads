#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 2

typedef struct {
    int r, g, b;
} Pixel;


Pixel *original;
Pixel *convertida;
int numeroTotalDePixels;

// função de converter da thead
void *converter(void *arg) {
    int id = *(int *)arg;

    int inicio = id * (numeroTotalDePixels / NUM_THREADS);
    int fim;

    if (id == NUM_THREADS-1)
        fim = numeroTotalDePixels;
    else
        fim = (id + 1) * (numeroTotalDePixels / NUM_THREADS);

    for (int i = inicio; i < fim; i++) {
        int r = original[i].r;
        int g = original[i].g;
        int b = original[i].b;
        int cinza = (int)(0.3 * r + 0.59 * g + 0.11 * b);
        convertida[i].r = cinza;
        convertida[i].g = cinza;
        convertida[i].b = cinza;
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        printf("Erro ao abrir arquivo \n");
        return 1;
    }

    char formato[3];
    int largura, altura, maxval;

    fscanf(f,"%s", formato);
    fscanf(f,"%d %d", &largura, &altura);
    fscanf(f,"%d", &maxval);

    numeroTotalDePixels = largura * altura;

    original = malloc(sizeof(Pixel) * numeroTotalDePixels);
    convertida = malloc(sizeof(Pixel) * numeroTotalDePixels);

    for (int i = 0; i < numeroTotalDePixels; i++) {
        fscanf(f, "%d %d %d",
               &original[i].r,
               &original[i].g,
               &original[i].b);
    }

    fclose(f);

    // da threads
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, converter, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    //salvar
    FILE *out = fopen(argv[2], "w");

    fprintf(out, "P3\n");
    fprintf(out, "%d %d\n", largura, altura);
    fprintf(out, "%d\n", maxval);

    for (int i = 0; i < numeroTotalDePixels; i++) {
        fprintf(out, "%d %d %d\n",
                convertida[i].r,
                convertida[i].g,
                convertida[i].b);
    }

    fclose(out);

    free(original);
    free(convertida);

    return 0;
}