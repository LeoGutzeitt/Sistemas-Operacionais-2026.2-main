#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "parser.h"

typedef struct {
    int lost_deadlines;  /* Prazos perdidos */
    int complete;        /* Instâncias concluídas */
    int killed;          /* Instâncias não concluídas */
} TaskStats;

typedef struct {
    Task *tasks;         /* Lista de tarefas */
    int num_tasks;       /* Número de tarefas */
    TaskStats *stats;    /* Estatísticas por tarefa */
    int current_time;    /* Tempo atual da simulação */
    int total_time;      /* Tempo total de simulação */
} SchedulerState;

void execute_rate_monotonic(Schedule *schedule, const char *output_file);
void execute_edf(Schedule *schedule, const char *output_file);

#endif /* SCHEDULER_H */
