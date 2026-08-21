#include "task.h"
#include <string.h>

int task_cadastrar(Task *task, const char *nome, int argc, char *argv[])
{
    if (task == NULL || nome == NULL || argv == NULL) {
        return -1;// Ponteiro nulo
    }

    if (nome[0] == '\0' || argc < 1 || argc > TASK_MAX_ARGS) {// Nome vazio ou número de argumentos inválido
        return -1;
    }

    for (int i = 0; i < argc; i++) {// Verifica se algum argumento é nulo
        if (argv[i] == NULL) {
            return -1;
        }
    }

    strncpy(task->nome, nome, TASK_NAME_SIZE - 1);// Copia o nome da tarefa para a estrutura, garantindo que não ultrapasse o tamanho máximo
    task->nome[TASK_NAME_SIZE - 1] = '\0';

    task->argc = argc;

    for (int i = 0; i < argc; i++) {
        // Copia o argumento para a estrutura, garantindo que não ultrapasse o tamanho máximo
        task->argv[i] = argv[i];
    }

    task->argv[argc] = NULL;

    return 0;
}

Task *task_buscar(Task *task, const char *nome)// Busca uma tarefa pelo nome
{
    if (task == NULL || nome == NULL) {     // Verifica se o ponteiro da tarefa ou o nome fornecido são nulos
        return NULL;
    }

    if (strcmp(task->nome, nome) == 0) {// Compara o nome da tarefa com o nome fornecido
        return task;// Retorna a tarefa encontrada
    }

    return NULL;// Retorna NULL se a tarefa não for encontrada
}