#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define MAX_LINE 256
#define MAX_TASKS 100

void print_error(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

Schedule* parse_input(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        print_error("Cannot open input file");
        return NULL;
    }
    
    Schedule *schedule = (Schedule *)malloc(sizeof(Schedule));
    if (!schedule) {
        print_error("Memory allocation failed");
        fclose(file);
        return NULL;
    }
    
    char line[MAX_LINE];
    
    /* Read total time */
    if (!fgets(line, MAX_LINE, file)) {
        print_error("File is empty or malformed");
        free(schedule);
        fclose(file);
        return NULL;
    }
    
    if (sscanf(line, "%d", &schedule->total_time) != 1 || schedule->total_time <= 0) {
        print_error("Invalid or missing total time");
        free(schedule);
        fclose(file);
        return NULL;
    }
    
    /* Allocate space for tasks */
    schedule->tasks = (TaskDef *)malloc(MAX_TASKS * sizeof(TaskDef));
    if (!schedule->tasks) {
        print_error("Memory allocation failed");
        free(schedule);
        fclose(file);
        return NULL;
    }
    
    schedule->num_tasks = 0;
    
    /* Read tasks */
    while (fgets(line, MAX_LINE, file) && schedule->num_tasks < MAX_TASKS) {
        /* Skip empty lines */
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        char name[256];
        int period, deadline, burst;
        
        if (sscanf(line, "%s %d %d %d", name, &period, &deadline, &burst) != 4) {
            print_error("Malformed task line");
            free(schedule->tasks);
            free(schedule);
            fclose(file);
            return NULL;
        }
        
        if (period <= 0 || deadline <= 0 || burst <= 0) {
            print_error("Task values must be positive");
            free(schedule->tasks);
            free(schedule);
            fclose(file);
            return NULL;
        }
        
        if (burst > deadline || deadline > period) {
            print_error("Invalid task constraints: C <= D <= P violated");
            free(schedule->tasks);
            free(schedule);
            fclose(file);
            return NULL;
        }
        
        TaskDef *task = &schedule->tasks[schedule->num_tasks];
        task->name = (char *)malloc(strlen(name) + 1);
        if (!task->name) {
            print_error("Memory allocation failed");
            free(schedule->tasks);
            free(schedule);
            fclose(file);
            return NULL;
        }
        
        strcpy(task->name, name);
        task->period = period;
        task->deadline = deadline;
        task->burst = burst;
        task->id = schedule->num_tasks;
        
        schedule->num_tasks++;
    }
    
    fclose(file);
    
    if (schedule->num_tasks == 0) {
        print_error("No tasks found in file");
        free(schedule->tasks);
        free(schedule);
        return NULL;
    }
    
    return schedule;
}

void free_schedule(Schedule *schedule) {
    if (schedule) {
        if (schedule->tasks) {
            for (int i = 0; i < schedule->num_tasks; i++) {
                free(schedule->tasks[i].name);
            }
            free(schedule->tasks);
        }
        free(schedule);
    }
}
