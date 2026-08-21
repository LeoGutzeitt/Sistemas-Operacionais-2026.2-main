#ifndef LGBM_TASK_H
#define LGBM_TASK_H

#define TASK_NAME_SIZE 64
#define TASK_MAX_ARGS 32

typedef struct task {
    char nome[TASK_NAME_SIZE];
    int argc;
    char *argv[TASK_MAX_ARGS + 1];
} Task;

int task_cadastrar(Task *task, const char *nome, int argc, char *argv[]);
Task *task_buscar(Task *task, const char *nome);

#endif // LGBM_TASK_H