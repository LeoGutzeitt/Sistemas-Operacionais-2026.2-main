#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void *write_to_file(void *arg) {
    FILE *file = (FILE *)arg;
    const char *message = "Hello from thread!\n";
    fputs(message, file);
    return NULL;
}