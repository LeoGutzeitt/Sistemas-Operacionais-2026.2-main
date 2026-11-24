#include "gerenciador_vm.h"

/* Funcao executada por cada thread para inspecionar uma unica entrada da TLB */
static void *trabalhador_busca_tlb(void *parametro) {
    TarefaBuscaTlb *tarefa = (TarefaBuscaTlb *)parametro;
    const RegistroTlb *reg = tarefa->registro;

    if (reg != NULL && reg->ativo && reg->pagina == tarefa->pagina_alvo) {
        tarefa->encontrou = true;
        tarefa->quadro_achado = reg->quadro;
    } else {
        tarefa->encontrou = false;
        tarefa->quadro_achado = -1;
    }
    return NULL;
}

void inicializar_tlb(ContextoMemoria *ctx) {
    for (int i = 0; i < TAMANHO_TLB; i++) {
        ctx->tlb[i].pagina = -1;
        ctx->tlb[i].quadro = -1;
        ctx->tlb[i].ativo = false;
        ctx->tlb[i].instante_carga = 0;
        ctx->tlb[i].ultimo_uso = 0;
    }
}

/* Pesquisa paralela (1 thread por entrada) ou linear na TLB */
bool consultar_tlb(ContextoMemoria *ctx, int num_pagina, int *quadro_out, int *indice_out) {
    if (!ctx->tlb_ativa) {
        return false;
    }

    int posicao_acerto = -1;
    int quadro_encontrado = -1;

    if (ctx->usar_threads) {
        pthread_t threads[TAMANHO_TLB];
        bool thread_iniciada[TAMANHO_TLB];
        TarefaBuscaTlb tarefas[TAMANHO_TLB];

        for (int i = 0; i < TAMANHO_TLB; i++) {
            tarefas[i].indice = i;
            tarefas[i].pagina_alvo = num_pagina;
            tarefas[i].registro = &ctx->tlb[i];
            tarefas[i].encontrou = false;
            tarefas[i].quadro_achado = -1;

            if (pthread_create(&threads[i], NULL, trabalhador_busca_tlb, &tarefas[i]) == 0) {
                thread_iniciada[i] = true;
            } else {
                thread_iniciada[i] = false;
                trabalhador_busca_tlb(&tarefas[i]);
            }
        }

        for (int i = 0; i < TAMANHO_TLB; i++) {
            if (thread_iniciada[i]) {
                pthread_join(threads[i], NULL);
            }
            if (tarefas[i].encontrou && posicao_acerto == -1) {
                posicao_acerto = i;
                quadro_encontrado = tarefas[i].quadro_achado;
            }
        }
    } else {
        for (int i = 0; i < TAMANHO_TLB; i++) {
            if (ctx->tlb[i].ativo && ctx->tlb[i].pagina == num_pagina) {
                posicao_acerto = i;
                quadro_encontrado = ctx->tlb[i].quadro;
                break;
            }
        }
    }

    if (posicao_acerto == -1) {
        return false;
    }

    *quadro_out = quadro_encontrado;
    if (indice_out != NULL) {
        *indice_out = posicao_acerto;
    }

    if (ctx->politica_tlb == POLITICA_LRU) {
        ctx->tlb[posicao_acerto].ultimo_uso = ++ctx->relogio;
    }
    if (ctx->politica_pagina == POLITICA_LRU) {
        ctx->quadros[quadro_encontrado].ultimo_uso = ++ctx->relogio;
    }

    return true;
}

static int escolher_vitima_tlb(const ContextoMemoria *ctx) {
    int vitima = 0;
    uint64_t menor_marca = UINT64_MAX;

    for (int i = 0; i < TAMANHO_TLB; i++) {
        uint64_t marca = (ctx->politica_tlb == POLITICA_FIFO)
                             ? ctx->tlb[i].instante_carga
                             : ctx->tlb[i].ultimo_uso;
        if (marca < menor_marca) {
            menor_marca = marca;
            vitima = i;
        }
    }
    return vitima;
}

int registrar_na_tlb(ContextoMemoria *ctx, int num_pagina, int quadro) {
    if (!ctx->tlb_ativa) {
        return -1;
    }

    /* Atualiza diretamente caso a pagina ja esteja mapeada na TLB */
    for (int i = 0; i < TAMANHO_TLB; i++) {
        if (ctx->tlb[i].ativo && ctx->tlb[i].pagina == num_pagina) {
            ctx->tlb[i].quadro = quadro;
            ctx->tlb[i].ultimo_uso = ++ctx->relogio;
            return i;
        }
    }

    /* Procura primeiro espaco livre; se cheia, aplica FIFO ou LRU */
    int destino = -1;
    for (int i = 0; i < TAMANHO_TLB; i++) {
        if (!ctx->tlb[i].ativo) {
            destino = i;
            break;
        }
    }

    if (destino == -1) {
        destino = escolher_vitima_tlb(ctx);
    }

    ctx->tlb[destino].pagina = num_pagina;
    ctx->tlb[destino].quadro = quadro;
    ctx->tlb[destino].ativo = true;
    ctx->tlb[destino].instante_carga = ++ctx->relogio;
    ctx->tlb[destino].ultimo_uso = ctx->relogio;

    return destino;
}

void invalidar_pagina_tlb(ContextoMemoria *ctx, int num_pagina) {
    if (!ctx->tlb_ativa) {
        return;
    }

    for (int i = 0; i < TAMANHO_TLB; i++) {
        if (ctx->tlb[i].ativo && ctx->tlb[i].pagina == num_pagina) {
            ctx->tlb[i].ativo = false;
            ctx->tlb[i].pagina = -1;
            ctx->tlb[i].quadro = -1;
            break;
        }
    }
}
