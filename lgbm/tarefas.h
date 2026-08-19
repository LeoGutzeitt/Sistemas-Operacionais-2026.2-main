#ifndef TAREFAS_H
#define TAREFAS_H

#include "tarefa.h"

int tarefas_carregar_de_arquivo(const char *caminho);
tarefa_t *tarefas_proxima(void);
void tarefas_limpar(void);

#endif // TAREFAS_H
