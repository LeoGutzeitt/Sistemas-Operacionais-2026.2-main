#include "executor.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int executor_run_argv(char *const argv[])
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

        resultado = executor_run_argv(tasks[i]->argv);
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
