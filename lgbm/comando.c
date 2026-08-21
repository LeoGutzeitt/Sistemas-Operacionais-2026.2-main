#include "comando.h"

#include <stdio.h>
#include <string.h>

#include "executor.h"

int comando_executar(Catalogo *catalogo, int argc, char *argv[])
{
    if (catalogo == NULL || argv == NULL || argc < 1) {
        return -1;
    }

    if (strcmp(argv[0], "task") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Uso: task <nome> <executavel> [argumentos...]\n");
            return -1;
        }

        if (catalogo_cadastrar(catalogo, argv[1], argc - 2, &argv[2]) != 0) {
            fprintf(stderr, "Erro ao cadastrar tarefa '%s'\n", argv[1]);
            return -1;
        }

        printf("Tarefa '%s' cadastrada.\n", argv[1]);
        return 0;
    }

    if (strcmp(argv[0], "run") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Uso: run <nome>\n");
            return -1;
        }

        Task *task = catalogo_buscar(catalogo, argv[1]);
        if (task == NULL) {
            fprintf(stderr, "Tarefa '%s' nao encontrada.\n", argv[1]);
            return -1;
        }

        return executor_run_argv(task->argv);
    }

    return executor_run_argv(argv);
}
