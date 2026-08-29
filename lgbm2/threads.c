#include "threads.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Estruturas auxiliares para organizar o trabalho em paralelo.
 * TarefaLinha: representa um bloco de linhas que uma thread deve calcular.
 * FilaTrabalho: controla quais linhas ainda faltam para serem processadas,
 *               em um modelo dinâmico com mutex para sincronizar o acesso.
 */
typedef struct {
    uint8_t *buffer;
    int largura;
    int altura;
    int linha_inicial;
    int linha_final;
    int max_iteracoes;
} TarefaLinha;

typedef struct {
    uint8_t *buffer;
    int largura;
    int altura;
    int max_iteracoes;
    int proxima_linha;
    int tamanho_bloco;
    pthread_mutex_t mutex;
} FilaTrabalho;

/*
 * Mostra como executar o programa corretamente.
 * O usuário passa: largura altura max_iteracoes num_threads.
 */
static void imprimir_uso(const char *nome_programa){
    fprintf(stderr, "Uso: %s [largura] [altura] [max_iteracoes] [num_threads]\n", nome_programa);
}

/*
 * Lê um inteiro positivo a partir de uma string.
 * Serve para validar os argumentos da linha de comando.
 */
static bool ler_inteiro_positivo(const char *texto, int *valor){
    char *fim = NULL;
    long numero;
    if(!texto || !*texto) return false;
    errno = 0;
    numero = strtol(texto, &fim, 10);
    if(errno != 0 || fim == texto || *fim != '\0' || numero < 1 || numero > INT_MAX) return false;
    *valor = (int)numero;
    return true;
}

/*
 * Converte a coordenada x da imagem para o plano complexo real.
 * A área do conjunto de Mandelbrot é mapeada entre -2.0 e 1.0.
 */
static double coordenada_real(int x, int largura){
    return largura <= 1 ? -2.0 : -2.0 + (double)x * (3.0 / (double)(largura - 1));
}

/*
 * Converte a coordenada y da imagem para o plano complexo imaginário.
 * A área vai de 1.5 para -1.5, invertendo a ordem da imagem.
 */
static double coordenada_imaginaria(int y, int altura){
    return altura <= 1 ? 1.5 : 1.5 - (double)y * (3.0 / (double)(altura - 1));
}

/*
 * Calcula quantas iterações um ponto do plano leva para escapar da sequência
 * z_{n+1} = z_n^2 + c.
 * Se atingir max_iteracoes sem escapar, considera-se que o ponto pertence ao conjunto.
 */
int calcular_iteracoes_pixel(int x, int y, int largura, int altura, int max_iteracoes){
    double cr = coordenada_real(x, largura);
    double ci = coordenada_imaginaria(y, altura);
    double zr = 0.0, zi = 0.0;
    int iteracoes = 0;
    while(iteracoes < max_iteracoes){
        double zr2 = zr * zr;
        double zi2 = zi * zi;
        if(zr2 + zi2 > 4.0) break;
        zi = 2.0 * zr * zi + ci;
        zr = zr2 - zi2 + cr;
        iteracoes++;
    }
    return iteracoes;
}

/*
 * Transforma o número de iterações em uma intensidade de 0 a 255.
 * Pontos fora do conjunto recebem zero, e a parte interna fica mais escura.
 */
uint8_t normalizar_intensidade_pixel(int iteracoes, int max_iteracoes){
    if(iteracoes >= max_iteracoes) return 0u;
    return (uint8_t)((double)iteracoes / (double)max_iteracoes * 255.0);
}

/*
 * Aloca o buffer em memória para guardar a imagem gerada.
 * O buffer é um vetor linear, onde cada posição representa um pixel.
 */
static uint8_t *alocar_buffer(const ConfiguracaoMandelbrot *cfg, const char *tipo){
    size_t total = (size_t)cfg->largura * (size_t)cfg->altura;
    uint8_t *buffer = (uint8_t *)malloc(total * sizeof(uint8_t));
    if(!buffer){
        fprintf(stderr, "Erro: falha na alocacao do buffer %s.\n", tipo);
        exit(EXIT_FAILURE);
    }
    return buffer;
}

/*
 * Processa um intervalo de linhas da imagem.
 * É a rotina central de cálculo: para cada pixel, calcula as iterações e grava o valor no buffer.
 */
static void processar_intervalo(int linha_inicial, int linha_final, const ConfiguracaoMandelbrot *cfg, uint8_t *buffer){
    for(int y = linha_inicial; y < linha_final; ++y){
        for(int x = 0; x < cfg->largura; ++x){
            int iteracoes = calcular_iteracoes_pixel(x, y, cfg->largura, cfg->altura, cfg->max_iteracoes);
            buffer[(size_t)y * cfg->largura + x] = normalizar_intensidade_pixel(iteracoes, cfg->max_iteracoes);
        }
    }
}

/*
 * Valida os argumentos recebidos pelo programa.
 * Se tudo estiver correto, retorna 1; se não, exibe erro e retorna 0.
 */
int analisar_e_validar_argumentos(int argc, char **argv, ConfiguracaoMandelbrot *configuracao){
    if(argc != 5){
        imprimir_uso(argv[0]);
        return 0;
    }
    if(!configuracao){
        fprintf(stderr, "Erro: ponteiro de configuracao invalido.\n");
        return 0;
    }
    if(!ler_inteiro_positivo(argv[1], &configuracao->largura) ||
       !ler_inteiro_positivo(argv[2], &configuracao->altura) ||
       !ler_inteiro_positivo(argv[3], &configuracao->max_iteracoes) ||
       !ler_inteiro_positivo(argv[4], &configuracao->numero_threads)){
        fprintf(stderr, "Erro: largura, altura, max_iteracoes e num_threads devem ser inteiros positivos.\n");
        return 0;
    }
    if(configuracao->largura > 10000 || configuracao->altura > 10000){
        fprintf(stderr, "Erro: largura e altura devem ser menores ou iguais a 10000.\n");
        return 0;
    }
    if(configuracao->max_iteracoes > 1000000){
        fprintf(stderr, "Erro: max_iteracoes deve ser menor ou igual a 1000000.\n");
        return 0;
    }
    if(configuracao->numero_threads > 256){
        fprintf(stderr, "Erro: num_threads deve ser menor ou igual a 256.\n");
        return 0;
    }
    configuracao->buffer = NULL;
    return 1;
}

/*
 * Versão serial: calcula toda a imagem em um único fluxo.
 * Simples, fácil de entender, mas mais lenta para imagens grandes.
 */
void calcular_imagem_serial(ConfiguracaoMandelbrot *cfg){
    cfg->buffer = alocar_buffer(cfg, "serial");
    processar_intervalo(0, cfg->altura, cfg, cfg->buffer);
}

/*
 * Versão com OpenMP.
 * Cada iteração do laço pode ser distribuída entre threads do ambiente OpenMP.
 */
void calcular_imagem_openmp(ConfiguracaoMandelbrot *cfg){
    cfg->buffer = alocar_buffer(cfg, "OpenMP");
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
    for(int y = 0; y < cfg->altura; ++y){
        for(int x = 0; x < cfg->largura; ++x){
            int iteracoes = calcular_iteracoes_pixel(x, y, cfg->largura, cfg->altura, cfg->max_iteracoes);
            cfg->buffer[(size_t)y * cfg->largura + x] = normalizar_intensidade_pixel(iteracoes, cfg->max_iteracoes);
        }
    }
#else
    fprintf(stderr, "Erro: compilacao OpenMP nao disponivel.\n");
    exit(EXIT_FAILURE);
#endif
}

/*
 * Função executada por cada thread na versão estática.
 * Cada thread recebe um intervalo fixo de linhas para calcular.
 */
static void *processar_linhas_estaticas(void *arg){
    TarefaLinha *tarefa = (TarefaLinha *)arg;
    ConfiguracaoMandelbrot cfg = {.largura = tarefa->largura, .altura = tarefa->altura, .max_iteracoes = tarefa->max_iteracoes};
    processar_intervalo(tarefa->linha_inicial, tarefa->linha_final, &cfg, tarefa->buffer);
    return NULL;
}

/*
 * Versão com pthreads em divisão estática.
 * O programa divide igualmente as linhas entre as threads antes de iniciar o cálculo.
 */
void calcular_imagem_pthreads_estatico(ConfiguracaoMandelbrot *cfg){
    int numero_threads = cfg->numero_threads > cfg->altura ? cfg->altura : cfg->numero_threads;
    cfg->buffer = alocar_buffer(cfg, "Pthreads estatico");
    pthread_t *threads = (pthread_t *)malloc((size_t)numero_threads * sizeof(pthread_t));
    TarefaLinha *tarefas = (TarefaLinha *)malloc((size_t)numero_threads * sizeof(TarefaLinha));
    if(!threads || !tarefas){
        fprintf(stderr, "Erro: falha na alocacao de estruturas de threads.\n");
        free(threads); free(tarefas);
        exit(EXIT_FAILURE);
    }
    int linhas_por_thread = cfg->altura / numero_threads;
    int resto = cfg->altura % numero_threads;
    int linha_atual = 0;
    for(int i = 0; i < numero_threads; ++i){
        int inicio = linha_atual;
        int fim = inicio + linhas_por_thread + (i < resto ? 1 : 0);
        linha_atual = fim;
        tarefas[i] = (TarefaLinha){.buffer = cfg->buffer, .largura = cfg->largura, .altura = cfg->altura, .linha_inicial = inicio, .linha_final = fim, .max_iteracoes = cfg->max_iteracoes};
        if(pthread_create(&threads[i], NULL, processar_linhas_estaticas, &tarefas[i]) != 0){
            fprintf(stderr, "Erro: falha na criacao de thread Pthreads estatico.\n");
            for(int j = 0; j < i; ++j) pthread_join(threads[j], NULL);
            free(threads); free(tarefas);
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 0; i < numero_threads; ++i) pthread_join(threads[i], NULL);
    free(threads); free(tarefas);
}

/*
 * Função executada por cada thread na versão dinâmica.
 * Ela pega o próximo bloco de linhas e processa até acabarem as linhas disponíveis.
 */
static void *processar_linhas_dinamicas(void *arg){
    FilaTrabalho *fila = (FilaTrabalho *)arg;
    while(1){
        pthread_mutex_lock(&fila->mutex);
        int inicio = fila->proxima_linha;
        if(inicio >= fila->altura){
            pthread_mutex_unlock(&fila->mutex);
            break;
        }
        int disponiveis = fila->altura - inicio;
        int quantidade = disponiveis < fila->tamanho_bloco ? disponiveis : fila->tamanho_bloco;
        fila->proxima_linha += quantidade;
        pthread_mutex_unlock(&fila->mutex);
        ConfiguracaoMandelbrot cfg = {.largura = fila->largura, .altura = fila->altura, .max_iteracoes = fila->max_iteracoes};
        processar_intervalo(inicio, inicio + quantidade, &cfg, fila->buffer);
    }
    return NULL;
}

/*
 * Versão dinâmicacom pthreads.
 * As threads pegam blocos de linhas sob demanda, evitando que uma thread fique ociosa.
 */
void calcular_imagem_pthreads_dinamico(ConfiguracaoMandelbrot *cfg){
    int numero_threads = cfg->numero_threads > cfg->altura ? cfg->altura : cfg->numero_threads;
    cfg->buffer = alocar_buffer(cfg, "Pthreads dinamico");
    FilaTrabalho fila = {.buffer = cfg->buffer, .largura = cfg->largura, .altura = cfg->altura, .max_iteracoes = cfg->max_iteracoes, .proxima_linha = 0, .tamanho_bloco = (cfg->altura + numero_threads - 1) / numero_threads, .mutex = PTHREAD_MUTEX_INITIALIZER};
    pthread_t *threads = (pthread_t *)malloc((size_t)numero_threads * sizeof(pthread_t));
    if(!threads){
        fprintf(stderr, "Erro: falha na alocacao de threads dinamicas.\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < numero_threads; ++i){
        if(pthread_create(&threads[i], NULL, processar_linhas_dinamicas, &fila) != 0){
            fprintf(stderr, "Erro: falha na criacao de thread Pthreads dinamico.\n");
            for(int j = 0; j < i; ++j) pthread_join(threads[j], NULL);
            free(threads);
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 0; i < numero_threads; ++i) pthread_join(threads[i], NULL);
    free(threads);
}

/*
 * Escreve a imagem em um arquivo em formato simples de valores por linha.
 * Cada posição do buffer vira um número inteiro e a linha final é separada por espaços.
 */
void escrever_arquivo_imagem(const char *nome_arquivo, const uint8_t *buffer, int largura, int altura){
    FILE *arquivo = fopen(nome_arquivo, "w");
    if(!arquivo){
        fprintf(stderr, "Erro: falha ao abrir %s para escrita.\n", nome_arquivo);
        exit(EXIT_FAILURE);
    }
    for(int y = 0; y < altura; ++y){
        for(int x = 0; x < largura; ++x){
            if(x > 0) fputc(' ', arquivo);
            fprintf(arquivo, "%u", (unsigned int)buffer[(size_t)y * largura + x]);
        }
        fputc('\n', arquivo);
    }
    fclose(arquivo);
}

/*
 * Mede o tempo de execução de uma função de cálculo.
 * Usa clock_gettime para maior precisão.
 */
static double medir_tempo_execucao(void (*funcao)(ConfiguracaoMandelbrot *), ConfiguracaoMandelbrot *cfg){
    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    funcao(cfg);
    clock_gettime(CLOCK_MONOTONIC, &fim);
    return (double)(fim.tv_sec - inicio.tv_sec) + (double)(fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;
}

/*
 * Cria um relatório final com os tempos de cada versão do programa.
 * O arquivo é usado como evidência para comparação de desempenho.
 */
static void registrar_relatorio(const char *nome_arquivo, const char *titulo, double serial, double openmp, double pthreads1, double pthreads2, int largura, int altura, int max_iteracoes, int num_threads){
    FILE *arquivo = fopen(nome_arquivo, "w");
    if(!arquivo){
        fprintf(stderr, "Erro: falha ao criar %s.\n", nome_arquivo);
        exit(EXIT_FAILURE);
    }
    fprintf(arquivo, "%s\n", titulo);
    fprintf(arquivo, "largura=%d altura=%d max_iteracoes=%d num_threads=%d\n", largura, altura, max_iteracoes, num_threads);
    fprintf(arquivo, "serial: %.6f segundos\nopenmp: %.6f segundos\npthreads1: %.6f segundos\npthreads2: %.6f segundos\n", serial, openmp, pthreads1, pthreads2);
    fclose(arquivo);
}

/*
 * Função principal: coordena todo o fluxo do programa.
 * 1. valida os argumentos;
 * 2. gera a imagem em serial;
 * 3. gera a imagem em OpenMP;
 * 4. gera a imagem em pthreads estático;
 * 5. gera a imagem em pthreads dinâmico;
 * 6. salva os tempos em arquivos de evidência.
 */
int main(int argc, char **argv){
    ConfiguracaoMandelbrot cfg = {0};
    if(!analisar_e_validar_argumentos(argc, argv, &cfg)) return EXIT_FAILURE;

    double t_serial = medir_tempo_execucao(calcular_imagem_serial, &cfg);
    escrever_arquivo_imagem("mandelbrot_lgbm_serial.pgm", cfg.buffer, cfg.largura, cfg.altura);
    free(cfg.buffer); cfg.buffer = NULL;

    double t_openmp = medir_tempo_execucao(calcular_imagem_openmp, &cfg);
    escrever_arquivo_imagem("mandelbrot_lgbm_openmp.pgm", cfg.buffer, cfg.largura, cfg.altura);
    free(cfg.buffer); cfg.buffer = NULL;

    double t_pthreads1 = medir_tempo_execucao(calcular_imagem_pthreads_estatico, &cfg);
    escrever_arquivo_imagem("mandelbrot_lgbm_pthreads1.pgm", cfg.buffer, cfg.largura, cfg.altura);
    free(cfg.buffer); cfg.buffer = NULL;

    double t_pthreads2 = medir_tempo_execucao(calcular_imagem_pthreads_dinamico, &cfg);
    escrever_arquivo_imagem("mandelbrot_lgbm_pthreads2.pgm", cfg.buffer, cfg.largura, cfg.altura);
    free(cfg.buffer); cfg.buffer = NULL;

    registrar_relatorio("evidencias.log", "Execucao do programa Mandelbrot", t_serial, t_openmp, t_pthreads1, t_pthreads2, cfg.largura, cfg.altura, cfg.max_iteracoes, cfg.numero_threads);
    registrar_relatorio("times.txt", "tempo", t_serial, t_openmp, t_pthreads1, t_pthreads2, cfg.largura, cfg.altura, cfg.max_iteracoes, cfg.numero_threads);
    return EXIT_SUCCESS;
}
