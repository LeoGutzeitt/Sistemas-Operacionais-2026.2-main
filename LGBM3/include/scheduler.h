#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "parser.h"

typedef struct {
    int lost_deadlines;
    int complete;
    int killed;
} TaskStats;

typedef struct {
    Task *tasks;
    int num_tasks;
    TaskStats *stats;
    int current_time;
    int total_time;
} SchedulerState;

void execute_rate_monotonic(Schedule *schedule, const char *output_file);
void execute_edf(Schedule *schedule, const char *output_file);

#endif /* SCHEDULER_H */
