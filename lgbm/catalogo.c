#include "catalogo.h"

#include <stddef.h>

void catalogo_inicializar(Catalogo *catalogo)
{
    if (catalogo == NULL) {
        return;
    }

    catalogo->quantidade = 0;
}

int catalogo_cadastrar(
    Catalogo *catalogo,
    const char *nome,
    int argc,
    char *argv[]
)
{
    if (catalogo == NULL || catalogo->quantidade >= CATALOGO_MAX_TAREFAS) {
        return -1;
    }

    if (catalogo_buscar(catalogo, nome) != NULL) {
        return -1;
    }

    if (task_cadastrar(
            &catalogo->tarefas[catalogo->quantidade],
            nome,
            argc,
            argv
        ) != 0) {
        return -1;
    }

    catalogo->quantidade++;
    return 0;
}

Task *catalogo_buscar(Catalogo *catalogo, const char *nome)
{
    if (catalogo == NULL || nome == NULL) {
        return NULL;
    }

    for (int i = 0; i < catalogo->quantidade; i++) {
        Task *task = task_buscar(&catalogo->tarefas[i], nome);

        if (task != NULL) {
            return task;
        }
    }

    return NULL;
}

void catalogo_liberar(Catalogo *catalogo)
{
    if (catalogo == NULL) {
        return;
    }

    for (int i = 0; i < catalogo->quantidade; i++) {
        task_liberar(&catalogo->tarefas[i]);
    }

    catalogo->quantidade = 0;
}
