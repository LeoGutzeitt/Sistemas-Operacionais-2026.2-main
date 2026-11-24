#include "gerenciador_vm.h"
#include <ctype.h>
#include <errno.h>

static int textos_iguais_sem_caixa(const char *a, const char *b) {
    if (a == NULL || b == NULL) return 0;
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static int interpretar_politica(const char *texto, PoliticaSubstituicao *saida) {
    if (textos_iguais_sem_caixa(texto, "fifo")) {
        *saida = POLITICA_FIFO;
        return 0;
    }
    if (textos_iguais_sem_caixa(texto, "lru")) {
        *saida = POLITICA_LRU;
        return 0;
    }
    return -1;
}

static void mostrar_uso(const char *nome_programa) {
    fprintf(stderr, "Uso: %s <arquivo_enderecos> <politica_pagina> <politica_tlb>\n", nome_programa);
    fprintf(stderr, "  <arquivo_enderecos>: arquivo texto com inteiros de 0 a 65535\n");
    fprintf(stderr, "  <politica_pagina>  : 'fifo' ou 'lru'\n");
    fprintf(stderr, "  <politica_tlb>     : 'fifo' ou 'lru'\n");
    fprintf(stderr, "Exemplo: %s address.txt lru fifo\n", nome_programa);
}

static int carregar_enderecos(const char *caminho, int **lista_out, size_t *qtd_out) {
    FILE *arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo de enderecos '%s': %s\n",
                caminho, strerror(errno));
        return -1;
    }

    size_t capacidade = 1024;
    size_t quantidade = 0;
    int *lista = (int *)malloc(capacidade * sizeof(int));
    if (lista == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para carregar os enderecos.\n");
        fclose(arquivo);
        return -1;
    }

    char linha[256];
    size_t num_linha = 0;

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        num_linha++;

        size_t comp = strlen(linha);
        if (comp == sizeof(linha) - 1 && linha[comp - 1] != '\n') {
            fprintf(stderr, "Erro de formatacao na linha %zu de '%s': linha longa demais.\n",
                    num_linha, caminho);
            free(lista);
            fclose(arquivo);
            return -1;
        }

        char *inicio = linha;
        while (*inicio != '\0' && isspace((unsigned char)*inicio)) inicio++;
        if (*inicio == '\0') continue;

        char *fim = inicio + strlen(inicio) - 1;
        while (fim > inicio && isspace((unsigned char)*fim)) {
            *fim = '\0';
            fim--;
        }

        for (const char *p = inicio; *p != '\0'; p++) {
            if (!isdigit((unsigned char)*p)) {
                fprintf(stderr, "Erro de formatacao na linha %zu de '%s': '%s' invalido.\n",
                        num_linha, caminho, inicio);
                free(lista);
                fclose(arquivo);
                return -1;
            }
        }

        char *resto = NULL;
        errno = 0;
        long valor = strtol(inicio, &resto, 10);
        if (errno != 0 || *resto != '\0' || valor < 0 || valor > 65535) {
            fprintf(stderr, "Erro de formatacao na linha %zu de '%s': '%s' fora de [0, 65535].\n",
                    num_linha, caminho, inicio);
            free(lista);
            fclose(arquivo);
            return -1;
        }

        if (quantidade == capacidade) {
            capacidade *= 2;
            int *nova_lista = (int *)realloc(lista, capacidade * sizeof(int));
            if (nova_lista == NULL) {
                fprintf(stderr, "Erro: falha ao redimensionar vetor de enderecos.\n");
                free(lista);
                fclose(arquivo);
                return -1;
            }
            lista = nova_lista;
        }

        lista[quantidade++] = (int)valor;
    }

    fclose(arquivo);

    if (quantidade == 0) {
        fprintf(stderr, "Erro: o arquivo '%s' nao contem enderecos validos.\n", caminho);
        free(lista);
        return -1;
    }

    *lista_out = lista;
    *qtd_out = quantidade;
    return 0;
}

static int executar_traducoes(ContextoMemoria *ctx, const char *arq_entrada, const char *arq_saida) {
    int *enderecos = NULL;
    size_t total = 0;

    if (carregar_enderecos(arq_entrada, &enderecos, &total) != 0) {
        return -1;
    }

    FILE *saida = fopen(arq_saida, "w");
    if (saida == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s': %s\n", arq_saida, strerror(errno));
        free(enderecos);
        return -1;
    }

    for (size_t i = 0; i < total; i++) {
        ResultadoTraducao res;
        if (traduzir_endereco(ctx, enderecos[i], &res) != 0) {
            fprintf(stderr, "Erro: falha ao traduzir endereco %d (linha %zu).\n",
                    enderecos[i], i + 1);
            fclose(saida);
            free(enderecos);
            return -1;
        }

        if (ctx->tlb_ativa) {
            fprintf(saida, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",
                    enderecos[i], res.indice_tlb, res.endereco_fisico, (int)res.valor_byte);
        } else {
            fprintf(saida, "Virtual address: %d Physical address: %d Value: %d\n",
                    enderecos[i], res.endereco_fisico, (int)res.valor_byte);
        }
    }

    double taxa_faltas = (double)ctx->faltas_pagina / (double)ctx->total_traducoes;
    double taxa_tlb = (double)ctx->acertos_tlb / (double)ctx->total_traducoes;

    fprintf(saida, "Number of Translated Addresses = %llu\n",
            (unsigned long long)ctx->total_traducoes);
    fprintf(saida, "Page Faults = %llu\n",
            (unsigned long long)ctx->faltas_pagina);
    fprintf(saida, "Page Fault Rate = %.3f\n", taxa_faltas);
    fprintf(saida, "TLB Hits = %llu\n",
            (unsigned long long)ctx->acertos_tlb);
    fprintf(saida, "TLB Hit Rate = %.3f\n", taxa_tlb);

    fclose(saida);
    free(enderecos);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Erro: quantidade incorreta de argumentos (recebido: %d, esperado: 3).\n",
                argc - 1);
        mostrar_uso(argv[0]);
        return 1;
    }

    PoliticaSubstituicao pol_pagina;
    PoliticaSubstituicao pol_tlb;

    if (interpretar_politica(argv[2], &pol_pagina) != 0) {
        fprintf(stderr, "Erro: politica de pagina invalida '%s'. Use 'fifo' ou 'lru'.\n", argv[2]);
        mostrar_uso(argv[0]);
        return 1;
    }

    if (interpretar_politica(argv[3], &pol_tlb) != 0) {
        fprintf(stderr, "Erro: politica de TLB invalida '%s'. Use 'fifo' ou 'lru'.\n", argv[3]);
        mostrar_uso(argv[0]);
        return 1;
    }

    ContextoMemoria ctx;
    if (iniciar_gerenciador(&ctx, VM_NUM_FRAMES, pol_pagina, pol_tlb,
                            VM_USE_TLB != 0, VM_USE_THREADS != 0, ARQUIVO_DISCO) != 0) {
        return 1;
    }

    if (executar_traducoes(&ctx, argv[1], ARQUIVO_SAIDA) != 0) {
        encerrar_gerenciador(&ctx);
        return 1;
    }

    printf("Traducao concluida. Resultado gravado em '%s'.\n", ARQUIVO_SAIDA);
    printf("Number of Translated Addresses = %llu\n", (unsigned long long)ctx.total_traducoes);
    printf("Page Faults = %llu\n", (unsigned long long)ctx.faltas_pagina);
    printf("Page Fault Rate = %.3f\n", (double)ctx.faltas_pagina / (double)ctx.total_traducoes);
    printf("TLB Hits = %llu\n", (unsigned long long)ctx.acertos_tlb);
    printf("TLB Hit Rate = %.3f\n", (double)ctx.acertos_tlb / (double)ctx.total_traducoes);

    encerrar_gerenciador(&ctx);
    return 0;
}
