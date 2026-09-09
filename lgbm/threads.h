#ifndef THREADS_H
#define THREADS_H

#include <stdbool.h>
#include <stdint.h>

#define LOGIN_MANDELBROT "lgbm"

typedef struct {
    int largura;
    int altura;
    int max_iteracoes;
    int numero_threads;
    uint8_t *buffer;
} ConfiguracaoMandelbrot;

int calcular_iteracoes_pixel(int x, int y, int largura, int altura, int max_iteracoes);
uint8_t normalizar_intensidade_pixel(int iteracoes, int max_iteracoes);
int analisar_e_validar_argumentos(int argc, char **argv, ConfiguracaoMandelbrot *configuracao);
void calcular_imagem_serial(ConfiguracaoMandelbrot *configuracao);
void calcular_imagem_openmp(ConfiguracaoMandelbrot *configuracao);
void calcular_imagem_pthreads_estatico(ConfiguracaoMandelbrot *configuracao);
void calcular_imagem_pthreads_dinamico(ConfiguracaoMandelbrot *configuracao);
void escrever_arquivo_imagem(const char *nome_arquivo, const uint8_t *buffer, int largura, int altura);
void escrever_arquivo_tempo(const char *nome_arquivo, const char *rotulo, double segundos);

#endif