#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define L 7       // Quantidade de linhas na tela
#define N_ARQS 5  // Total de arquivos a serem lidos
#define T 3       // Número de threads

// Mutexes: um para a fila de arquivos e um array para as linhas da tela
pthread_mutex_t mutex_fila;
pthread_mutex_t mutex_tela[L];

int proximo_arquivo = 1; // Contador para saber qual arquivo ler (trem1, trem2...)

// Cores ANSI para cada linha (índice 0 a 6)
char* cores[] = {"\033[1;31m", "\033[1;33m", "\033[1;34m", "\033[1;35m", 
                 "\033[1;32m", "\033[1;37m", "\033[1;36m"};

void* thread_operaria(void* arg) {
    while (1) {
        int arq_id;

        // --- SEÇÃO CRÍTICA 1: Pegar o número do próximo arquivo ---
        pthread_mutex_lock(&mutex_fila);
        if (proximo_arquivo > N_ARQS) {
            pthread_mutex_unlock(&mutex_fila);
            break; // Fim dos arquivos
        }
        arq_id = proximo_arquivo;
        proximo_arquivo++;
        pthread_mutex_unlock(&mutex_fila);

        // --- LEITURA DO ARQUIVO ---
        char nome_arquivo[20];
        sprintf(nome_arquivo, "trem%d.txt", arq_id);

        FILE *f = fopen(nome_arquivo, "r");
        if (f == NULL) {
            continue; // Se o arquivo não existir, pula pro próximo
        }

        int linha;
        char id[15], destino[30], horario[10];
        
        // fscanf: Lê o conteúdo do arquivo formatado
        if (fscanf(f, "%d %s %s %s", &linha, id, destino, horario) == 4) {
            int idx = linha - 1; // Ajusta para índice do array (0 a 6)

            if (idx >= 0 && idx < L) {
                // --- SEÇÃO CRÍTICA 2 (REFINADA): Trava apenas a linha específica ---
                pthread_mutex_lock(&mutex_tela[idx]);

                // ANSI: Mover cursor, limpar linha e imprimir colorido
                // \033[%d;1H -> vai para a linha 'n', coluna 1
                // \033[K    -> limpa o que tinha antes na linha
                printf("\033[%d;1H\033[K%s%s\t%-15s\t%s\033[0m", 
                       linha, cores[idx], id, destino, horario);
                fflush(stdout);

                sleep(2); // Pausa obrigatória de 2 segundos por linha

                pthread_mutex_unlock(&mutex_tela[idx]);
            }
        }
        fclose(f);
    }
    return NULL;
}

int main() {
    pthread_t threads[T];

    // Inicialização
    printf("\033[2J"); // Limpa a tela toda
    pthread_mutex_init(&mutex_fila, NULL);
    for (int i = 0; i < L; i++) {
        pthread_mutex_init(&mutex_tela[i], NULL);
    }

    // Criação das threads
    for (int i = 0; i < T; i++) {
        pthread_create(&threads[i], NULL, thread_operaria, NULL);
    }

    // Espera conclusão
    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }

    // Move o cursor para o fim da tabela para não bagunçar o terminal
    printf("\033[%d;1H\nProcessamento concluído.\n", L + 1);

    // Destruir mutexes
    pthread_mutex_destroy(&mutex_fila);
    for (int i = 0; i < L; i++) pthread_mutex_destroy(&mutex_tela[i]);

    return 0;
}