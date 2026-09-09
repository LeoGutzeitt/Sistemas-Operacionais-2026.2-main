#ifndef TASK_H
#define TASK_H

#include <time.h>

typedef struct {
    char *name;           /* Task name */
    int period;           /* Arrival period (P) */
    int deadline;         /* Relative deadline (D) */
    int burst;            /* CPU time needed (C) */
    int id;               /* Task ID (order in input file) */
} TaskDef;

typedef struct {
    TaskDef *def;         /* Pointer to task definition */
    int remaining_burst;  /* Remaining CPU time for current instance */
    int absolute_deadline;/* Absolute deadline for current instance */
    int next_arrival;     /* Time of next arrival */
    int instance;         /* Current instance number */
    int complete;         /* 1 if current instance is complete, 0 otherwise */
    int arrived;          /* 1 if this instance has arrived */
} Task;

Task* initialize_tasks(TaskDef *task_defs, int num_tasks);
void reset_task_instance(Task *task, int current_time);
int has_arrived(Task *task, int current_time);
int is_ready(Task *task, int current_time);
int is_past_deadline(Task *task, int current_time);

#endif /* TASK_H */
