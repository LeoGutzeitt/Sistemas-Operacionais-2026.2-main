#include "comando.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "executor.h"

static int comando_configurar_arquivo(Task *task, const char *tipo, const char *caminho, int append)
{
    if (task == NULL || tipo == NULL || caminho == NULL) {
        return -1;
    }

    char *nova = strdup(caminho);
    if (nova == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para '%s'.\n", tipo);
        return -1;
    }

    if (strcmp(tipo, "input") == 0) {
        free(task->input_path);
        task->input_path = nova;
        return 0;
    }

    if (strcmp(tipo, "output") == 0 || strcmp(tipo, "append") == 0) {
        free(task->output_path);
        task->output_path = nova;
        task->output_append = append;
        return 0;
    }

    free(nova);
    fprintf(stderr, "Tipo de redirecionamento invalido: %s\n", tipo);
    return -1;
}

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

    if (strcmp(argv[0], "input") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Uso: input <nome> <arquivo>\n");
            return -1;
        }

        Task *task = catalogo_buscar(catalogo, argv[1]);
        if (task == NULL) {
            fprintf(stderr, "Tarefa '%s' nao encontrada.\n", argv[1]);
            return -1;
        }

        return comando_configurar_arquivo(task, "input", argv[2], 0);
    }

    if (strcmp(argv[0], "output") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Uso: output <nome> <arquivo>\n");
            return -1;
        }

        Task *task = catalogo_buscar(catalogo, argv[1]);
        if (task == NULL) {
            fprintf(stderr, "Tarefa '%s' nao encontrada.\n", argv[1]);
            return -1;
        }

        return comando_configurar_arquivo(task, "output", argv[2], 0);
    }

    if (strcmp(argv[0], "append") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Uso: append <nome> <arquivo>\n");
            return -1;
        }

        Task *task = catalogo_buscar(catalogo, argv[1]);
        if (task == NULL) {
            fprintf(stderr, "Tarefa '%s' nao encontrada.\n", argv[1]);
            return -1;
        }

        return comando_configurar_arquivo(task, "append", argv[2], 1);
    }

    if (strcmp(argv[0], "workdir") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Uso: workdir <diretorio>\n");
            return -1;
        }

        executor_set_workdir(argv[1]);
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

        if (argc >= 2 && strcmp(argv[1], "pipe") == 0) {
            if (argc < 4) {
                fprintf(stderr, "Uso: run pipe <tarefa> [tarefas...]\n");
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

            return executor_run_pipe(tasks, quantidade);
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

        return executor_run_task(task);
    }

    return executor_run_argv(argv);
}
