#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "scheduler.h"
#include "task.h"

typedef struct {
    FILE *output;
    Task *tasks;
    int num_tasks;
    TaskStats *stats;
} Context;

void write_execution(Context *ctx, const char *task_name, int duration, char status) {
    fprintf(ctx->output, "[%s] for %d units - %c\n", task_name, duration, status);
}

void write_idle(Context *ctx, int duration) {
    fprintf(ctx->output, "idle for %d units\n", duration);
}

void write_stats(Context *ctx) {
    fprintf(ctx->output, "LOST DEADLINES\n");
    for (int i = 0; i < ctx->num_tasks; i++) {
        fprintf(ctx->output, "[%s] %d\n", ctx->tasks[i].def->name, ctx->stats[i].lost_deadlines);
    }
    
    fprintf(ctx->output, "COMPLETE EXECUTION\n");
    for (int i = 0; i < ctx->num_tasks; i++) {
        fprintf(ctx->output, "[%s] %d\n", ctx->tasks[i].def->name, ctx->stats[i].complete);
    }
    
    fprintf(ctx->output, "KILLED\n");
    for (int i = 0; i < ctx->num_tasks; i++) {
        fprintf(ctx->output, "[%s] %d\n", ctx->tasks[i].def->name, ctx->stats[i].killed);
    }
}

int find_highest_priority_task_rate(Context *ctx, int current_time) {
    int best = -1;
    int best_priority = INT_MAX;
    
    for (int i = 0; i < ctx->num_tasks; i++) {
        if (!is_ready(&ctx->tasks[i], current_time)) continue;
        
        /* Rate-monotonic: smaller period = higher priority */
        if (ctx->tasks[i].def->period < best_priority ||
            (ctx->tasks[i].def->period == best_priority && best >= 0 && i < best)) {
            best = i;
            best_priority = ctx->tasks[i].def->period;
        }
    }
    
    return best;
}

int find_highest_priority_task_edf(Context *ctx, int current_time) {
    int best = -1;
    int best_deadline = INT_MAX;
    
    for (int i = 0; i < ctx->num_tasks; i++) {
        if (!is_ready(&ctx->tasks[i], current_time)) continue;
        
        /* EDF: earlier deadline = higher priority */
        if (ctx->tasks[i].absolute_deadline < best_deadline ||
            (ctx->tasks[i].absolute_deadline == best_deadline && best >= 0 && i < best)) {
            best = i;
            best_deadline = ctx->tasks[i].absolute_deadline;
        }
    }
    
    return best;
}

void run_scheduler(Schedule *schedule, const char *output_file,
                   int (*priority_func)(Context *, int), const char *algo_name) {
    FILE *output = fopen(output_file, "w");
    if (!output) {
        print_error("Cannot create output file");
        return;
    }
    
    fprintf(output, "EXECUTION BY %s\n", algo_name);
    
    Task *tasks = initialize_tasks(schedule->tasks, schedule->num_tasks);
    TaskStats *stats = (TaskStats *)calloc(schedule->num_tasks, sizeof(TaskStats));
    
    Context ctx = { output, tasks, schedule->num_tasks, stats };
    
    int current_time = 0;
    while (current_time < schedule->total_time) {
        /* Check for new arrivals */
        for (int i = 0; i < schedule->num_tasks; i++) {
            if (current_time > 0 && current_time == tasks[i].next_arrival && tasks[i].complete) {
                reset_task_instance(&tasks[i], current_time);
            }
        }
        
        /* Check for missed deadlines */
        for (int i = 0; i < schedule->num_tasks; i++) {
            if (is_past_deadline(&tasks[i], current_time)) {
                stats[i].lost_deadlines++;
                tasks[i].complete = 1;
                tasks[i].remaining_burst = 0;
            }
        }
        
        /* Find task to execute */
        int task_idx = priority_func(&ctx, current_time);
        
        if (task_idx == -1) {
            /* Idle */
            write_idle(&ctx, 1);
            current_time++;
        } else {
            /* Execute for 1 unit or until preemption/completion */
            Task *current_task = &tasks[task_idx];
            current_task->remaining_burst--;
            
            if (current_task->remaining_burst == 0) {
                write_execution(&ctx, current_task->def->name, 1, 'F');
                current_task->complete = 1;
                stats[task_idx].complete++;
            } else {
                write_execution(&ctx, current_task->def->name, 1, 'H');
            }
            
            current_time++;
        }
    }
    
    /* Count killed tasks */
    for (int i = 0; i < schedule->num_tasks; i++) {
        if (!tasks[i].complete) {
            stats[i].killed++;
        }
    }
    
    write_stats(&ctx);
    
    free(tasks);
    free(stats);
    fclose(output);
}

void execute_rate_monotonic(Schedule *schedule, const char *output_file) {
    run_scheduler(schedule, output_file, find_highest_priority_task_rate, "RATE");
}

void execute_edf(Schedule *schedule, const char *output_file) {
    run_scheduler(schedule, output_file, find_highest_priority_task_edf, "EDF");
}
