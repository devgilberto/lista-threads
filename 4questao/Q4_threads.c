#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// ==========================================
// CONSTANTES QUE EU POSSO MUDAR
// ==========================================
#define C 3     // numero de cabines de pedagio
#define T 10    // numero de carros (threads)

// ============================================
// ESTRUTURAS GLOBAIS
// ============================================
int cabines[C];              // 0 = livre, 1 = ocupada
pthread_mutex_t mutex;       // mutex pra proteger as cabines
pthread_cond_t cond;         // variavel de condicao pra esperar cabine livre
int contador_carros = 0;     // so pra numerar os carros (opcional)

// =======================================
// FUNCAO QUE CADA CARRO (THREAD) VAI EXECUTAR
// =======================================
void* carro(void* arg) {
    int id = *(int*)arg;     // id do carro (1, 2, 3...)
    int cabine_id = -1;      // qual cabine ele vai usar
    
    // --- 1. CARRO CHEGA AO PEDAGIO ---
    printf("Carro %d chegou ao pedagio\n", id);
    fflush(stdout);
    
    // --- 2. PEGAR UMA CABINE LIVRE ---
    pthread_mutex_lock(&mutex);   // entro na regiao critica
    
    // enquanto nao tiver cabine livre, espero
    while (1) {
        // procura uma cabine livre
        for (int i = 0; i < C; i++) {
            if (cabines[i] == 0) {
                cabine_id = i;
                cabines[i] = 1;   // ocupo a cabine
                break;
            }
        }
        
        if (cabine_id != -1) {
            break;   // achei uma cabine, saio do loop
        }
        
        // se nao achei cabine livre, espero
        printf("Carro %d aguardando cabine livre\n", id);
        fflush(stdout);
        pthread_cond_wait(&cond, &mutex);  // suspende ate alguem liberar
    }
    
    pthread_mutex_unlock(&mutex);  // saio da regiao critica
    
    // --- 3. USAR A CABINE (PAGAR PEDAGIO) ---
    printf("Carro %d usando cabine %d\n", id, cabine_id + 1);
    fflush(stdout);
    
    sleep(2);   // atendimento demora um pouco (2 segundos)
    
    // --- 4. LIBERAR A CABINE ---
    pthread_mutex_lock(&mutex);   // entro na regiao critica
    
    cabines[cabine_id] = 0;       // libero a cabine
    printf("Carro %d terminou pagamento e liberou cabine %d\n", id, cabine_id + 1);
    fflush(stdout);
    
    pthread_cond_signal(&cond);   // aviso que uma cabine ficou livre
    pthread_mutex_unlock(&mutex); // saio da regiao critica
    
    // --- 5. CARRO SEGUE VIAGEM ---
    printf("Carro %d terminou pagamento e seguiu viagem\n", id);
    fflush(stdout);
    
    return NULL;
}

// ============================================
// MAIN
// ============================================
int main() {
    pthread_t threads[T];
    int ids[T];
    
    // --- INICIALIZAR AS CABINES (TUDO LIVRE) ---
    for (int i = 0; i < C; i++) {
        cabines[i] = 0;   // 0 = livre
    }
    
    // --- INICIALIZAR MUTEX E VARIAVEL DE CONDICAO ---
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    
    printf("=== SIMULACAO DE PEDAGIO COM %d CABINES E %d CARROS ===\n", C, T);
    printf("========================================================\n\n");
    
    // --- CRIAR AS THREADS (CARROS) ---
    for (int i = 0; i < T; i++) {
        ids[i] = i + 1;   // carro 1, 2, 3...
        pthread_create(&threads[i], NULL, carro, &ids[i]);
        usleep(500000);   // atraso de 0.5 seg entre chegadas 
    }
    
    // --- ESPERAR TODOS OS CARROS TERMINAREM ---
    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // --- DESTRUIR MUTEX E VARIAVEL DE CONDICAO ---
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    printf("\n========================================================\n");
    printf("=== SIMULACAO FINALIZADA! TODOS OS CARROS PASSARAM ===\n");
    
    return 0;
}