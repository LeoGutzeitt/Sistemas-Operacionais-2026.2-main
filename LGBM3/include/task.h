#ifndef TASK_H
#define TASK_H

#include <time.h>

typedef struct {
    char *name;           /* Nome da tarefa */
    int period;           /* Período de chegada (P) */
    int deadline;         /* Prazo relativo (D) */
    int burst;            /* Tempo de CPU necessário (C) */
    int id;               /* ID da tarefa (ordem no arquivo de entrada) */
} TaskDef;

typedef struct {
    TaskDef *def;         /* Ponteiro para definição de tarefa */
    int remaining_burst;  /* Tempo de CPU restante para instância atual */
    int absolute_deadline;/* Prazo absoluto para instância atual */
    int next_arrival;     /* Tempo da próxima chegada */
    int instance;         /* Número da instância atual */
    int complete;         /* 1 se instância atual está completa, 0 caso contrário */
    int arrived;          /* 1 se esta instância chegou */
} Task;

Task* initialize_tasks(TaskDef *task_defs, int num_tasks);
void reset_task_instance(Task *task, int current_time);
int has_arrived(Task *task);
int is_ready(Task *task, int current_time);
int is_past_deadline(Task *task, int current_time);

#endif /* TASK_H */
