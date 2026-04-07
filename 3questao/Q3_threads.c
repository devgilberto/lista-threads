#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define TAM 16
#define NTHREADS 4

// nó da lista
typedef struct no {
    int chave;
    int valor;
    struct no* prox;
} No;

// bucket com mutex próprio
typedef struct {
    No* inicio;
    pthread_mutex_t lock;
} Bucket;

// hashmap
typedef struct {
    Bucket* buckets;
    int tamanho;
} HashMap;

// função hash simples
int hash(HashMap* h, int chave) {
    return abs(chave) % h->tamanho;
}

// cria hashmap
HashMap* criar() {
    HashMap* h = malloc(sizeof(HashMap));
    h->tamanho = TAM;
    h->buckets = malloc(sizeof(Bucket) * TAM);

    for (int i = 0; i < TAM; i++) {
        h->buckets[i].inicio = NULL;
        pthread_mutex_init(&h->buckets[i].lock, NULL);
    }

    return h;
}

// inserir ou atualizar
void inserir(HashMap* h, int chave, int valor) {
    int i = hash(h, chave);
    Bucket* b = &h->buckets[i];

    pthread_mutex_lock(&b->lock);

    No* atual = b->inicio;

    // se já existir, atualiza
    while (atual != NULL) {
        if (atual->chave == chave) {
            atual->valor = valor;
            pthread_mutex_unlock(&b->lock);
            return;
        }
        atual = atual->prox;
    }

    // senão, insere no começo
    No* novo = malloc(sizeof(No));
    novo->chave = chave;
    novo->valor = valor;
    novo->prox = b->inicio;
    b->inicio = novo;

    pthread_mutex_unlock(&b->lock);
}

// busca
int buscar(HashMap* h, int chave) {
    int i = hash(h, chave);
    Bucket* b = &h->buckets[i];

    pthread_mutex_lock(&b->lock);

    No* atual = b->inicio;

    while (atual != NULL) {
        if (atual->chave == chave) {
            int v = atual->valor;
            pthread_mutex_unlock(&b->lock);
            return v;
        }
        atual = atual->prox;
    }

    pthread_mutex_unlock(&b->lock);
    return -1; // não achou
}

// remover
void remover(HashMap* h, int chave) {
    int i = hash(h, chave);
    Bucket* b = &h->buckets[i];

    pthread_mutex_lock(&b->lock);

    No* atual = b->inicio;
    No* ant = NULL;

    while (atual != NULL) {
        if (atual->chave == chave) {

            if (ant == NULL) {
                b->inicio = atual->prox;
            } else {
                ant->prox = atual->prox;
            }

            free(atual);
            pthread_mutex_unlock(&b->lock);
            return;
        }

        ant = atual;
        atual = atual->prox;
    }

    pthread_mutex_unlock(&b->lock);
}

// liberar memória
void destruir(HashMap* h) {
    for (int i = 0; i < TAM; i++) {
        No* atual = h->buckets[i].inicio;

        while (atual != NULL) {
            No* temp = atual;
            atual = atual->prox;
            free(temp);
        }

        pthread_mutex_destroy(&h->buckets[i].lock);
    }

    free(h->buckets);
    free(h);
}

// =====================
// THREADS
// =====================

typedef struct {
    HashMap* h;
    int id;
} Args;

void* tarefa(void* arg) {
    Args* a = (Args*)arg;

    // cada thread faz algumas operações
    for (int i = 0; i < 3; i++) {
        int chave = a->id * 10 + i;

        inserir(a->h, chave, chave * 10);
        printf("Thread %d inseriu %d\n", a->id, chave);

        int v = buscar(a->h, chave);
        if (v != -1) {
            printf("Thread %d buscou %d -> %d\n", a->id, chave, v);
        }

        if (i == 1) {
        remover(a->h, chave);
        printf("Thread %d removeu %d\n", a->id, chave);

        // verifica se realmente foi removido
        int v = buscar(a->h, chave);
        printf("Thread %d buscou %d -> %d\n", a->id, chave, v);
        }
    }

    return NULL;
}

// =====================
// MAIN
// =====================

int main() {
    printf("Inicio\n");

    HashMap* h = criar();

    printf("Teste simples:\n");

    inserir(h, 10, 100);
    printf("Inseriu 10\n");

    inserir(h, 20, 200);
    printf("Inseriu 20\n");

    remover(h, 20);
    printf("Removeu 20\n");

    int d = buscar(h, 10);
    printf("Busca 10 -> %d\n", d);

    // verificar se realmente saiu
    int v = buscar(h, 20);
    printf("Busca 20 -> %d\n", v);

    // teste com threads
    printf("\nTeste com threads\n");

    pthread_t t[NTHREADS];
    Args args[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        args[i].h = h;
        args[i].id = i + 1;
        pthread_create(&t[i], NULL, tarefa, &args[i]);
    }

    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(t[i], NULL);
    }

    destruir(h);

    printf("\nFim\n");
    return 0;
}