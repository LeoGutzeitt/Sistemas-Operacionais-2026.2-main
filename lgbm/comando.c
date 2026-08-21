#include "comando.h"

#include <stdio.h>
#include <string.h>

#include "executor.h"

int comando_executar(Catalogo *catalogo, int argc, char *argv[])
{
    if (catalogo == NULL || argv == NULL || argc < 1) {
        return -1;
    }

    if (strcmp(argv[0], "exit") == 0) {
        if (argc != 1) {
            fprintf(stderr, "Uso: exit\n");
            return -1;
        }

        return COMANDO_SAIR;
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
        if (argc >= 2 && strcmp(argv[1], "sequential") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Uso: run sequential <tarefa> [tarefas...]\n");
                return -1;
            }

            Task *tasks[TASK_MAX_ARGS + 1];
            int quantidade = argc - 2;

            if (quantidade > TASK_MAX_ARGS + 1) {
                fprintf(stderr, "Numero maximo de tarefas excedido.\n");
                return -1;
            }

            for (int i = 0; i < quantidade; i++) {
                tasks[i] = catalogo_buscar(catalogo, argv[i + 2]);

                if (tasks[i] == NULL) {
                    fprintf(stderr, "Tarefa '%s' nao encontrada.\n", argv[i + 2]);
                    return -1;
                }
            }

            return executor_run_sequential(tasks, quantidade);
        }

        if (argc >= 2 && strcmp(argv[1], "parallel") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Uso: run parallel <tarefa> [tarefas...]\n");
                return -1;
            }

            Task *tasks[TASK_MAX_ARGS + 1];
            int quantidade = argc - 2;

            if (quantidade > TASK_MAX_ARGS + 1) {
                fprintf(stderr, "Numero maximo de tarefas excedido.\n");
                return -1;
            }

            for (int i = 0; i < quantidade; i++) {
                tasks[i] = catalogo_buscar(catalogo, argv[i + 2]);

                if (tasks[i] == NULL) {
                    fprintf(stderr, "Tarefa '%s' nao encontrada.\n", argv[i + 2]);
                    return -1;
                }
            }

            return executor_run_parallel(tasks, quantidade);
        }

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
