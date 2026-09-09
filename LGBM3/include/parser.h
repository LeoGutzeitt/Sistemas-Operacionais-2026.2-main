#ifndef PARSER_H
#define PARSER_H

#include "task.h"

typedef struct {
    int total_time;
    TaskDef *tasks;
    int num_tasks;
} Schedule;

Schedule* parse_input(const char *filename);
void free_schedule(Schedule *schedule);
void print_error(const char *msg);

#endif /* PARSER_H */
