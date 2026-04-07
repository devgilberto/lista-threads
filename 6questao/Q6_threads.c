#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
//Struct onde são armazenados os argumentos da função mergesort
typedef struct{
    int* arr;
    int i;
    int f;
}sort_args;

void merge(int* arr, int l, int m, int r){
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
    
    int *esq = malloc(sizeof(int)*n1);
    int *dir = malloc(sizeof(int)*n2);

    for(i=0; i<n1; i++){
        esq[i] = arr[l+i];
    }
    for(j=0; j<n2; j++){
        dir[j] = arr[m+1+j];
    }

    i = 0;
    j = 0;
    k = l;

    while (i<n1 && j<n2){
        if(esq[i] <= dir[j]){
            arr[k] = esq[i];
            i++;
        }
        else{
            arr[k] = dir[j];
            j++;
        }
        k++;
    }

    while (i<n1){
        arr[k] = esq[i];
        i++;
        k++;
    }
    while (j<n2){
        arr[k] = dir[j];
        j++;
        k++;
    }
    free(esq);
    free(dir);

} 
//Função Mergesort em paralelo
void* parallel_mergesort(void* arg){
    sort_args* args = (sort_args*)arg;
    int* arr = args->arr;
    int i = args->i;
    int f = args->f;

    if(i<f){
        int m = i + (f-i) / 2;
        sort_args esq_args = {arr, i, m};
        sort_args dir_args = {arr, m+1, f};
        pthread_t tid;
        printf("Criando thread para o intervalo [%d:%d]\n", i, m);
        //É criada uma thread que fica responsável pela metade esquerda do array a ser ordenado
        pthread_create(&tid, NULL, parallel_mergesort, &esq_args);
        //A metade direita é ordenada pela main (caso seja a primeira execução da função recursiva) ou pela thread atual
        parallel_mergesort(&dir_args);
        printf("Aguardando conclusao da thread\n");
        pthread_join(tid, NULL);
        //A função merge funciona da mesma maneira tanto para o mergesort sequencial quanto para o paralelo!
        merge(arr, i, m, f);
    }
    return NULL;
}
void mostrarVetorOriginal(int* v, int tamanho){
    printf("Este eh o vetor original:\n");
    for(int i=0; i<tamanho; i++){
        printf("%d ", v[i]);
    }
    printf("\n");
}

void mostrarVetorOrdenado(int* v, int tamanho){
    printf("O vetor foi ordenado com sucesso!\n");
    for(int i=0; i<tamanho; i++){
        printf("%d ", v[i]);
    }
    printf("\n");
}



int main(){
    //Vetor a ser ordenado:
    int v[] = {7,-2,3,9,1,0,8};
    int tamanho = sizeof(v)/sizeof(v[0]);
    sort_args args = {v, 0, tamanho-1};
    mostrarVetorOriginal(v, tamanho);
    parallel_mergesort(&args);
    mostrarVetorOrdenado(v, tamanho);
    return 0;
}