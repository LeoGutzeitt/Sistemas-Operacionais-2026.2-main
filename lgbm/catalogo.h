#ifndef LGBM_CATALOGO_H
#define LGBM_CATALOGO_H

#include "task.h"

#define CATALOGO_MAX_TAREFAS 50

typedef struct {
    Task tarefas[CATALOGO_MAX_TAREFAS];
    int quantidade;
} Catalogo;

void catalogo_inicializar(Catalogo *catalogo);
int catalogo_cadastrar(Catalogo *catalogo, const char *nome, int argc, char *argv[]);
Task *catalogo_buscar(Catalogo *catalogo, const char *nome);

#endif
