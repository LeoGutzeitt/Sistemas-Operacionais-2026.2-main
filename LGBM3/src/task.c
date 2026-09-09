#include <stdlib.h>
#include <string.h>
#include "task.h"

/* Task management functions */

Task* initialize_tasks(TaskDef *task_defs, int num_tasks) {
    Task *tasks = (Task *)malloc(num_tasks * sizeof(Task));
    if (!tasks) return NULL;
    
    for (int i = 0; i < num_tasks; i++) {
        tasks[i].def = &task_defs[i];
        tasks[i].remaining_burst = task_defs[i].burst;
        tasks[i].absolute_deadline = task_defs[i].deadline;
        tasks[i].next_arrival = task_defs[i].period;
        tasks[i].instance = 0;
        tasks[i].complete = 0;
        tasks[i].arrived = 1;  /* All arrive at time 0 */
    }
    
    return tasks;
}

void reset_task_instance(Task *task, int current_time) {
    task->instance++;
    task->remaining_burst = task->def->burst;
    task->absolute_deadline = current_time + task->def->deadline;
    task->next_arrival = current_time + task->def->period;
    task->complete = 0;
    task->arrived = 1;
}

int has_arrived(Task *task, int current_time) {
    return task->arrived;
}

int is_ready(Task *task, int current_time) {
    return task->arrived && !task->complete && current_time < task->absolute_deadline;
}

int is_past_deadline(Task *task, int current_time) {
    return task->arrived && !task->complete && current_time >= task->absolute_deadline;
}
