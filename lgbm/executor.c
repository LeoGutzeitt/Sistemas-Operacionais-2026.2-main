#define _POSIX_C_SOURCE 200809L

#include "executor.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static char *executor_workdir = NULL;

void executor_set_workdir(const char *diretorio)
{
    if (diretorio == NULL || diretorio[0] == '\0') {
        fprintf(stderr, "Diretorio invalido.\n");
        return;
    }

    if (chdir(diretorio) != 0) {
        fprintf(stderr, "Diretorio '%s' nao existe ou nao pode ser acessado.\n", diretorio);
        return;
    }

    free(executor_workdir);
    executor_workdir = strdup(diretorio);
    if (executor_workdir == NULL) {
        fprintf(stderr, "Erro ao registrar diretorio de trabalho.\n");
    }
}

static int executor_aplicar_redirecionamentos(const Task *task)
{
    if (task == NULL) {
        return -1;
    }

    if (task->input_path != NULL) {
        int input_fd = open(task->input_path, O_RDONLY);
        if (input_fd < 0) {
            fprintf(stderr, "Erro ao abrir arquivo de entrada '%s': %s\n", task->input_path, strerror(errno));
            return -1;
        }
        if (dup2(input_fd, STDIN_FILENO) < 0) {
            perror("dup2 stdin");
            close(input_fd);
            return -1;
        }
        close(input_fd);
    }

    if (task->output_path != NULL) {
        int flags = O_WRONLY | O_CREAT;
        if (task->output_append) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }

        int output_fd = open(task->output_path, flags, 0644);
        if (output_fd < 0) {
            fprintf(stderr, "Erro ao abrir arquivo de saida '%s': %s\n", task->output_path, strerror(errno));
            return -1;
        }
        if (dup2(output_fd, STDOUT_FILENO) < 0) {
            perror("dup2 stdout");
            close(output_fd);
            return -1;
        }
        close(output_fd);
    }

    return 0;
}

static int executor_run_exec(char *const argv[], const Task *task)
{
    if (argv == NULL || argv[0] == NULL) {
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        if (executor_aplicar_redirecionamentos(task) != 0) {
            _exit(127);
        }

        if (executor_workdir != NULL && chdir(executor_workdir) != 0) {
            fprintf(stderr, "Nao foi possivel mudar para o diretorio '%s'.\n", executor_workdir);
            _exit(127);
        }

        execvp(argv[0], argv);
        perror("execvp");
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}

int executor_run_argv(char *const argv[])
{
    return executor_run_exec(argv, NULL);
}

int executor_run_task(Task *task)
{
    if (task == NULL || task->argv[0] == NULL) {
        return -1;
    }

    return executor_run_exec(task->argv, task);
}

int executor_run_sequential(Task *tasks[], int quantidade)
{
    if (tasks == NULL || quantidade < 1) {
        return -1;
    }

    int resultado = 0;

    for (int i = 0; i < quantidade; i++) {
        if (tasks[i] == NULL) {
            return -1;
        }

        resultado = executor_run_task(tasks[i]);
    }

    return resultado;
}

int executor_run_parallel(Task *tasks[], int quantidade)
{
    if (tasks == NULL || quantidade < 1) {
        return -1;
    }

    pid_t pids[quantidade];
    int criados = 0;

    for (int i = 0; i < quantidade; i++) {
        if (tasks[i] == NULL) {
            return -1;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            break;
        }

        if (pid == 0) {
            if (executor_aplicar_redirecionamentos(tasks[i]) != 0) {
                _exit(127);
            }

            if (executor_workdir != NULL && chdir(executor_workdir) != 0) {
                fprintf(stderr, "Nao foi possivel mudar para o diretorio '%s'.\n", executor_workdir);
                _exit(127);
            }

            execvp(tasks[i]->argv[0], tasks[i]->argv);
            perror("execvp");
            _exit(127);
        }

        pids[criados++] = pid;
    }

    int resultado = 0;

    for (int i = 0; i < criados; i++) {
        int status;

        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            resultado = -1;
            continue;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            resultado = WEXITSTATUS(status);
        } else if (!WIFEXITED(status)) {
            resultado = -1;
        }
    }

    return resultado;
}

int executor_run_pipe(Task *tasks[], int quantidade)
{
    if (tasks == NULL || quantidade < 1) {
        return -1;
    }

    int in_fd = -1;
    pid_t pids[quantidade];
    int criados = 0;

    for (int i = 0; i < quantidade; i++) {
        int pipefd[2] = {-1, -1};

        if (i < quantidade - 1) {
            if (pipe(pipefd) != 0) {
                perror("pipe");
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {
            if (in_fd != -1) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (i < quantidade - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }

            if (executor_aplicar_redirecionamentos(tasks[i]) != 0) {
                _exit(127);
            }

            if (executor_workdir != NULL && chdir(executor_workdir) != 0) {
                fprintf(stderr, "Nao foi possivel mudar para o diretorio '%s'.\n", executor_workdir);
                _exit(127);
            }

            execvp(tasks[i]->argv[0], tasks[i]->argv);
            perror("execvp");
            _exit(127);
        }

        if (in_fd != -1) {
            close(in_fd);
        }

        if (i < quantidade - 1) {
            close(pipefd[1]);
            in_fd = pipefd[0];
        }

        pids[criados++] = pid;
    }

    int resultado = 0;
    for (int i = 0; i < criados; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            resultado = -1;
            continue;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            resultado = WEXITSTATUS(status);
        } else if (!WIFEXITED(status)) {
            resultado = -1;
        }
    }

    return resultado;
}
