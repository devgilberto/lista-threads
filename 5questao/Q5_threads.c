#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 3              // tamanho da matriz quadrada (NxN)
#define NUM_THREADS 3    // número de threads para verificação

int matriz[N][N];
int soma_magica = 0;
int eh_magico = 1;       // 1 = verdadeiro, 0 = falso
pthread_mutex_t mutex;   // mutex para proteger acesso a eh_magico

// estrutura para passar dados para cada thread
typedef struct {
    int id;
    int inicio;
    int fim;
} ThreadData;

// função que verifica um intervalo de linhas
void* verifica_parte(void* arg) {
    ThreadData* dados = (ThreadData*)arg;
    
    // verifica as linhas atribuídas a esta thread
    for (int i = dados->inicio; i < dados->fim; i++) {
        int somaLinha = 0;
        for (int j = 0; j < N; j++) {
            somaLinha += matriz[i][j];
        }
        
        if (somaLinha != soma_magica) {
            pthread_mutex_lock(&mutex);   // entrada na região crítica
            eh_magico = 0;                // modifica variável compartilhada
            pthread_mutex_unlock(&mutex); // saída da região crítica
            pthread_exit(NULL);
        }
    }
    
    // apenas a thread 0 verifica colunas e diagonais (evita retrabalho)
    if (dados->id == 0) {
        // verifica todas as colunas
        for (int j = 0; j < N; j++) {
            int somaColuna = 0;
            for (int i = 0; i < N; i++) {
                somaColuna += matriz[i][j];
            }
            
            if (somaColuna != soma_magica) {
                pthread_mutex_lock(&mutex);
                eh_magico = 0;
                pthread_mutex_unlock(&mutex);
                pthread_exit(NULL);
            }
        }
        
        // verifica as duas diagonais
        int somaDiag1 = 0, somaDiag2 = 0;
        for (int i = 0; i < N; i++) {
            somaDiag1 += matriz[i][i];           // diagonal principal
            somaDiag2 += matriz[i][N - i - 1];   // diagonal secundária
        }
        
        if (somaDiag1 != soma_magica || somaDiag2 != soma_magica) {
            pthread_mutex_lock(&mutex);
            eh_magico = 0;
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }
    }
    
    pthread_exit(NULL);
}

int main() {
    // inicializa o mutex
    pthread_mutex_init(&mutex, NULL);
    
    // usuário digita os valores da matriz
    printf("Digite os valores da matriz %dx%d:\n", N, N);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            scanf("%d", &matriz[i][j]);
        }
    }
    
    // calcula a soma mágica esperada (usando a primeira linha)
    for (int j = 0; j < N; j++) {
        soma_magica += matriz[0][j];
    }
    
    // prepara os dados para cada thread
    pthread_t threads[NUM_THREADS];
    ThreadData dados[NUM_THREADS];
    
    // divide as linhas igualmente entre as threads
    int linhas_por_thread = N / NUM_THREADS;
    int resto = N % NUM_THREADS;
    int linha_atual = 0;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        dados[i].id = i;
        dados[i].inicio = linha_atual;
        
        int linhas_para_essa_thread = linhas_por_thread;
        if (resto > 0) {
            linhas_para_essa_thread++;
            resto--;
        }
        dados[i].fim = linha_atual + linhas_para_essa_thread;
        linha_atual = dados[i].fim;
        
        // cria a thread
        pthread_create(&threads[i], NULL, verifica_parte, &dados[i]);
    }
    
    // aguarda todas as threads terminarem
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // destrói o mutex
    pthread_mutex_destroy(&mutex);
    
    // resultado final
    if (eh_magico) {
        printf("A matriz eh um quadrado magico!\n");
    } else {
        printf("A matriz NAO eh um quadrado magico!\n");
    }
    
    return 0;
}