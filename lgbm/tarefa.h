#ifndef TAREFA_H
#define TAREFA_H

#include <stdlib.h>

typedef struct {
    int id;
    char *comando;
} tarefa_t;

tarefa_t *tarefa_criar(int id, const char *cmd);
void tarefa_liberar(tarefa_t *t);

#endif // TAREFA_H
