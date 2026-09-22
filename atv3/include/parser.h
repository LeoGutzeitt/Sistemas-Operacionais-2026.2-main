#ifndef PARSER_H
#define PARSER_H

#include "task.h"

typedef struct {
    int total_time;    /* Tempo total de simulação */
    TaskDef *tasks;    /* Lista de definições de tarefas */
    int num_tasks;     /* Número de tarefas */
} Schedule;

Schedule* parse_input(const char *filename);
void free_schedule(Schedule *schedule);
void print_error(const char *msg);

#endif /* PARSER_H */
