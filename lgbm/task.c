#include "task.h"

#include <stdlib.h>
#include <string.h>

int task_cadastrar(Task *task, const char *nome, int argc, char *argv[])
{
    if (task == NULL || nome == NULL || argv == NULL) {
        return -1;
    }

    if (nome[0] == '\0' || argc < 1 || argc > TASK_MAX_ARGS) {
        return -1;
    }

    for (int i = 0; i < argc; i++) {
        if (argv[i] == NULL) {
            return -1;
        }
    }

    strncpy(task->nome, nome, TASK_NAME_SIZE - 1);
    task->nome[TASK_NAME_SIZE - 1] = '\0';

    task->argc = argc;

    for (int i = 0; i < argc; i++) {
        task->argv[i] = strdup(argv[i]);
        if (task->argv[i] == NULL) {
            task_liberar(task);
            return -1;
        }
    }

    task->argv[argc] = NULL;

    return 0;
}

Task *task_buscar(Task *task, const char *nome)
{
    if (task == NULL || nome == NULL) {
        return NULL;
    }

    if (strcmp(task->nome, nome) == 0) {
        return task;
    }

    return NULL;
}

void task_liberar(Task *task)
{
    if (task == NULL) {
        return;
    }

    for (int i = 0; i < task->argc; i++) {
        free(task->argv[i]);
        task->argv[i] = NULL;
    }

    task->argc = 0;
    task->nome[0] = '\0';
}