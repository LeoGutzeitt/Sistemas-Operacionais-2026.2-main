#include "gerenciador_vm.h"
#include <errno.h>

static FILE *abrir_disco_backing(const char *caminho) {
    FILE *arq = fopen(caminho, "rb");
    if (arq != NULL) {
        return arq;
    }

    char caminho_alternativo[512];
    snprintf(caminho_alternativo, sizeof(caminho_alternativo), "../%s", caminho);
    return fopen(caminho_alternativo, "rb");
}

int iniciar_gerenciador(ContextoMemoria *ctx, int qtd_quadros,
                        PoliticaSubstituicao pol_pagina, PoliticaSubstituicao pol_tlb,
                        bool tlb_ativa, bool usar_threads, const char *caminho_disco) {
    if (ctx == NULL || qtd_quadros <= 0 || qtd_quadros > MAX_QUADROS) {
        return -1;
    }

    memset(ctx, 0, sizeof(ContextoMemoria));
    ctx->qtd_quadros = qtd_quadros;
    ctx->politica_pagina = pol_pagina;
    ctx->politica_tlb = pol_tlb;
    ctx->tlb_ativa = tlb_ativa;
    ctx->usar_threads = usar_threads;

    inicializar_tlb(ctx);

    for (int i = 0; i < TOTAL_PAGINAS; i++) {
        ctx->tabela_paginas[i].quadro = -1;
        ctx->tabela_paginas[i].presente = false;
    }

    for (int i = 0; i < MAX_QUADROS; i++) {
        ctx->quadros[i].pagina_alocada = -1;
        ctx->quadros[i].instante_carga = 0;
        ctx->quadros[i].ultimo_uso = 0;
    }

    const char *nome_disco = (caminho_disco != NULL) ? caminho_disco : ARQUIVO_DISCO;
    ctx->disco = abrir_disco_backing(nome_disco);
    if (ctx->disco == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s': %s\n", nome_disco, strerror(errno));
        return -1;
    }

    if (fseek(ctx->disco, 0, SEEK_END) != 0) {
        fprintf(stderr, "Erro: falha ao consultar tamanho de '%s'.\n", nome_disco);
        encerrar_gerenciador(ctx);
        return -1;
    }

    long tamanho_disco = ftell(ctx->disco);
    if (tamanho_disco < (long)(TOTAL_PAGINAS * TAMANHO_PAGINA)) {
        fprintf(stderr, "Erro: arquivo '%s' possui %ld bytes (minimo esperado: %d bytes).\n",
                nome_disco, tamanho_disco, TOTAL_PAGINAS * TAMANHO_PAGINA);
        encerrar_gerenciador(ctx);
        return -1;
    }
    rewind(ctx->disco);

    return 0;
}

void encerrar_gerenciador(ContextoMemoria *ctx) {
    if (ctx == NULL) {
        return;
    }
    if (ctx->disco != NULL) {
        fclose(ctx->disco);
        ctx->disco = NULL;
    }
}

static int selecionar_quadro_vitima(const ContextoMemoria *ctx) {
    int quadro_vitima = 0;
    uint64_t menor_marca = UINT64_MAX;

    for (int q = 0; q < ctx->qtd_quadros; q++) {
        uint64_t marca = (ctx->politica_pagina == POLITICA_FIFO)
                             ? ctx->quadros[q].instante_carga
                             : ctx->quadros[q].ultimo_uso;
        if (marca < menor_marca) {
            menor_marca = marca;
            quadro_vitima = q;
        }
    }
    return quadro_vitima;
}

static int resolver_falta_pagina(ContextoMemoria *ctx, int num_pagina) {
    ctx->faltas_pagina++;
    int quadro = -1;

    if (ctx->prox_quadro_livre < ctx->qtd_quadros) {
        quadro = ctx->prox_quadro_livre++;
    } else {
        quadro = selecionar_quadro_vitima(ctx);
        int pagina_removida = ctx->quadros[quadro].pagina_alocada;

        if (pagina_removida >= 0 && pagina_removida < TOTAL_PAGINAS) {
            ctx->tabela_paginas[pagina_removida].presente = false;
            ctx->tabela_paginas[pagina_removida].quadro = -1;
            invalidar_pagina_tlb(ctx, pagina_removida);
        }
    }

    long deslocamento_disco = (long)num_pagina * TAMANHO_PAGINA;
    if (fseek(ctx->disco, deslocamento_disco, SEEK_SET) != 0) {
        fprintf(stderr, "Erro: falha de posicionamento no disco para pagina %d.\n", num_pagina);
        return -1;
    }

    size_t lidos = fread(ctx->ram[quadro], sizeof(signed char), TAMANHO_PAGINA, ctx->disco);
    if (lidos != TAMANHO_PAGINA) {
        fprintf(stderr, "Erro: leitura incompleta da pagina %d (%zu bytes).\n", num_pagina, lidos);
        return -1;
    }

    ctx->quadros[quadro].pagina_alocada = num_pagina;
    ctx->quadros[quadro].instante_carga = ++ctx->relogio;
    ctx->quadros[quadro].ultimo_uso = ctx->relogio;

    ctx->tabela_paginas[num_pagina].quadro = quadro;
    ctx->tabela_paginas[num_pagina].presente = true;

    return quadro;
}

int traduzir_endereco(ContextoMemoria *ctx, int endereco_logico, ResultadoTraducao *resultado) {
    if (ctx == NULL || resultado == NULL) {
        return -1;
    }
    if (endereco_logico < 0 || endereco_logico > 65535) {
        return -1;
    }

    int num_pagina = (endereco_logico >> 8) & 0xFF;
    int deslocamento = endereco_logico & 0xFF;
    int quadro = -1;
    int indice_tlb = -1;

    ctx->total_traducoes++;

    if (consultar_tlb(ctx, num_pagina, &quadro, &indice_tlb)) {
        ctx->acertos_tlb++;
    } else {
        if (ctx->tabela_paginas[num_pagina].presente) {
            quadro = ctx->tabela_paginas[num_pagina].quadro;
            if (ctx->politica_pagina == POLITICA_LRU) {
                ctx->quadros[quadro].ultimo_uso = ++ctx->relogio;
            }
        } else {
            quadro = resolver_falta_pagina(ctx, num_pagina);
            if (quadro < 0) {
                return -1;
            }
        }
        indice_tlb = registrar_na_tlb(ctx, num_pagina, quadro);
    }

    resultado->indice_tlb = indice_tlb;
    resultado->endereco_fisico = (quadro << 8) | deslocamento;
    resultado->valor_byte = ctx->ram[quadro][deslocamento];
    return 0;
}
