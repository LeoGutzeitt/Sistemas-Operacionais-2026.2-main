#include <string.h>
#include "tarefa.h"

tarefa_t *tarefa_criar(int id, const char *cmd) {
    tarefa_t *t = malloc(sizeof(tarefa_t));
    if (!t) return NULL;
    t->id = id;
    if (cmd) {
        t->comando = strdup(cmd);
        if (!t->comando) { free(t); return NULL; }
    } else {
        t->comando = NULL;
    }
    return t;
}

void tarefa_liberar(tarefa_t *t) {
    if (!t) return;
    free(t->comando);
    free(t);
}
