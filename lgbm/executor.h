#ifndef LGBM_EXECUTOR_H
#define LGBM_EXECUTOR_H

#include "task.h"

int executor_run_argv(char *const argv[]);
int executor_run_sequential(Task *tasks[], int quantidade);
int executor_run_parallel(Task *tasks[], int quantidade);

#endif
