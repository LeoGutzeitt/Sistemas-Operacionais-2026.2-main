#ifndef GERENCIADOR_VM_H
#define GERENCIADOR_VM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define TAMANHO_PAGINA 256
#define TOTAL_PAGINAS 256
#define MAX_QUADROS 256
#define TAMANHO_TLB 16

#ifndef VM_NUM_FRAMES
#define VM_NUM_FRAMES 128
#endif

#ifndef VM_USE_TLB
#define VM_USE_TLB 1
#endif

#ifndef VM_USE_THREADS
#define VM_USE_THREADS 1
#endif

#define ARQUIVO_DISCO "BACKING_STORE.bin"
#define ARQUIVO_SAIDA "correct.txt"

typedef enum {
    POLITICA_FIFO = 0,
    POLITICA_LRU = 1
} PoliticaSubstituicao;

typedef struct {
    int pagina;
    int quadro;
    bool ativo;
    uint64_t instante_carga;
    uint64_t ultimo_uso;
} RegistroTlb;

typedef struct {
    int indice;
    int pagina_alvo;
    const RegistroTlb *registro;
    bool encontrou;
    int quadro_achado;
} TarefaBuscaTlb;

typedef struct {
    int quadro;
    bool presente;
} EntradaPagina;

typedef struct {
    int pagina_alocada;
    uint64_t instante_carga;
    uint64_t ultimo_uso;
} InfoQuadro;

typedef struct {
    int indice_tlb;
    int endereco_fisico;
    signed char valor_byte;
} ResultadoTraducao;

typedef struct {
    int qtd_quadros;
    PoliticaSubstituicao politica_pagina;
    PoliticaSubstituicao politica_tlb;
    bool tlb_ativa;
    bool usar_threads;

    uint64_t total_traducoes;
    uint64_t faltas_pagina;
    uint64_t acertos_tlb;
    uint64_t relogio;

    RegistroTlb tlb[TAMANHO_TLB];
    EntradaPagina tabela_paginas[TOTAL_PAGINAS];
    InfoQuadro quadros[MAX_QUADROS];
    signed char ram[MAX_QUADROS][TAMANHO_PAGINA];

    int prox_quadro_livre;
    FILE *disco;
} ContextoMemoria;

/* Modulo TLB (tlb.c) */
void inicializar_tlb(ContextoMemoria *ctx);
bool consultar_tlb(ContextoMemoria *ctx, int num_pagina, int *quadro_out, int *indice_out);
int registrar_na_tlb(ContextoMemoria *ctx, int num_pagina, int quadro);
void invalidar_pagina_tlb(ContextoMemoria *ctx, int num_pagina);

/* Modulo Paginacao e Memoria Fisica (paginacao.c) */
int iniciar_gerenciador(ContextoMemoria *ctx, int qtd_quadros,
                        PoliticaSubstituicao pol_pagina, PoliticaSubstituicao pol_tlb,
                        bool tlb_ativa, bool usar_threads, const char *caminho_disco);
void encerrar_gerenciador(ContextoMemoria *ctx);
int traduzir_endereco(ContextoMemoria *ctx, int endereco_logico, ResultadoTraducao *resultado);

#endif /* GERENCIADOR_VM_H */
