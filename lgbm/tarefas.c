#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "tarefas.h"

static tarefa_t **tarefas = NULL;
static size_t tarefas_cap = 0;
static size_t tarefas_count = 0;
static size_t tarefas_next = 0;
static pthread_mutex_t tarefas_mtx = PTHREAD_MUTEX_INITIALIZER;

static char *remover_nova_linha(char *s) {
    if (!s) return s;
    char *p = strchr(s, '\n');
    if (p) *p = '\0';
    return s;
}

int tarefas_carregar_de_arquivo(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) return -1;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int id = 1;

    while ((read = getline(&line, &len, f)) != -1) {
        remover_nova_linha(line);
        if (read == 0 || line[0] == '\0') continue;

        if (tarefas_count + 1 > tarefas_cap) {
            size_t newcap = tarefas_cap ? tarefas_cap * 2 : 16;
            tarefa_t **tmp = realloc(tarefas, newcap * sizeof(tarefa_t*));
            if (!tmp) { free(line); fclose(f); return -1; }
            tarefas = tmp;
            tarefas_cap = newcap;
        }

        tarefas[tarefas_count++] = tarefa_criar(id++, line);
    }

    free(line);
    fclose(f);
    return 0;
}

tarefa_t *tarefas_proxima(void) {
    pthread_mutex_lock(&tarefas_mtx);
    tarefa_t *t = NULL;
    if (tarefas_next < tarefas_count) {
        t = tarefas[tarefas_next++];
    }
    pthread_mutex_unlock(&tarefas_mtx);
    return t;
}

void tarefas_limpar(void) {
    if (tarefas) {
        for (size_t i = tarefas_next; i < tarefas_count; ++i) {
            tarefa_liberar(tarefas[i]);
        }
        free(tarefas);
        tarefas = NULL;
    }
    tarefas_cap = tarefas_count = tarefas_next = 0;
}
