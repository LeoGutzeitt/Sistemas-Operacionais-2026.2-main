#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "scheduler.h"

char* extract_login() {
    /* Placeholder: replace with actual login extraction logic */
    /* For this implementation, we assume login is provided or use "mla" as default */
    return "mla";
}

char* build_output_filename(const char *algorithm) {
    char *login = extract_login();
    char *output = (char *)malloc(256);
    if (!output) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }
    
    sprintf(output, "%s_%s.out", algorithm, login);
    return output;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <algorithm> <input_file>\n", argv[0]);
        fprintf(stderr, "  algorithm: rate or edf\n");
        return 1;
    }
    
    const char *algorithm = argv[1];
    const char *input_file = argv[2];
    
    /* Validate algorithm */
    if (strcmp(algorithm, "rate") != 0 && strcmp(algorithm, "edf") != 0) {
        fprintf(stderr, "Error: Algorithm must be 'rate' or 'edf'\n");
        return 1;
    }
    
    /* Parse input */
    Schedule *schedule = parse_input(input_file);
    if (!schedule) {
        return 1;
    }
    
    /* Build output filename */
    char *output_file = build_output_filename(algorithm);
    if (!output_file) {
        free_schedule(schedule);
        return 1;
    }
    
    /* Run scheduler */
    if (strcmp(algorithm, "rate") == 0) {
        execute_rate_monotonic(schedule, output_file);
    } else {
        execute_edf(schedule, output_file);
    }
    
    /* Cleanup */
    free(output_file);
    free_schedule(schedule);
    
    return 0;
}
