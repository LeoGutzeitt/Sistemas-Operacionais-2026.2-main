#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CUSTOMER_FILE "customer.txt"
#define COMMAND_FILE "commands.txt"
#define RESULT_FILE "result.txt"
#define MAX_LINE_LENGTH 65536

struct banker_state {
    size_t customer_count;
    size_t resource_count;
    int *total;
    int *available;
    int *maximum;
    int *allocation;
    int *need;
};

static void free_state(struct banker_state *state)
{
    free(state->total);
    free(state->available);
    free(state->maximum);
    free(state->allocation);
    free(state->need);
    memset(state, 0, sizeof(*state));
}

static int checked_calloc(void **out, size_t count, size_t size)
{
    if (size != 0 && count > SIZE_MAX / size) {
        return 0;
    }
    *out = calloc(count, size);
    return *out != NULL;
}

static int parse_nonnegative_int(const char *text, int *value)
{
    char *end = NULL;
    long parsed;

    if (text == NULL || *text == '\0') {
        return 0;
    }
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed < 0 || parsed > INT_MAX) {
        return 0;
    }
    *value = (int) parsed;
    return 1;
}

static int parse_resource_count(const char *text, int *value)
{
    if (!parse_nonnegative_int(text, value) || *value == 0) {
        return 0;
    }
    return 1;
}

static int read_line(FILE *file, char *line, size_t capacity, size_t *line_number,
                     const char *filename)
{
    size_t length;

    if (fgets(line, (int) capacity, file) == NULL) {
        return 0;
    }
    ++*line_number;
    length = strlen(line);
    if (length > 0 && line[length - 1] != '\n' && !feof(file)) {
        int ch;
        while ((ch = fgetc(file)) != '\n' && ch != EOF) {
            /* Drain overlong line so the next read starts at a new record. */
        }
        fprintf(stderr, "Erro: linha %zu muito longa em %s.\n", *line_number, filename);
        return -1;
    }
    return 1;
}

static char *skip_spaces(char *cursor)
{
    while (isspace((unsigned char) *cursor)) {
        ++cursor;
    }
    return cursor;
}

static int parse_customer_line(char *line, size_t resource_count, int *values)
{
    char *cursor = skip_spaces(line);
    size_t i;

    for (i = 0; i < resource_count; ++i) {
        char *end;
        long parsed;

        cursor = skip_spaces(cursor);
        errno = 0;
        parsed = strtol(cursor, &end, 10);
        if (end == cursor || errno != 0 || parsed < 0 || parsed > INT_MAX) {
            return 0;
        }
        values[i] = (int) parsed;
        cursor = skip_spaces(end);
        if (i + 1 < resource_count) {
            if (*cursor != ',') {
                return 0;
            }
            ++cursor;
        } else {
            cursor = skip_spaces(cursor);
            if (*cursor != '\0') {
                return 0;
            }
        }
    }
    return 1;
}

static int load_customers(struct banker_state *state)
{
    FILE *file = fopen(CUSTOMER_FILE, "r");
    char line[MAX_LINE_LENGTH];
    size_t line_number = 0;
    size_t capacity = 0;
    int *row = NULL;
    int status;

    if (file == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s: %s\n", CUSTOMER_FILE, strerror(errno));
        return 0;
    }
    if (!checked_calloc((void **) &row, state->resource_count, sizeof(*row))) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        fclose(file);
        return 0;
    }

    while ((status = read_line(file, line, sizeof(line), &line_number, CUSTOMER_FILE)) > 0) {
        int *grown;
        size_t next_capacity;

        if (skip_spaces(line)[0] == '\0' || skip_spaces(line)[0] == '\n' ||
            skip_spaces(line)[0] == '\r') {
            fprintf(stderr, "Erro: linha vazia em %s:%zu.\n", CUSTOMER_FILE, line_number);
            free(row);
            fclose(file);
            return 0;
        }
        if (!parse_customer_line(line, state->resource_count, row)) {
            fprintf(stderr, "Erro: formato invalido em %s:%zu; esperados %zu valores separados por virgula.\n",
                    CUSTOMER_FILE, line_number, state->resource_count);
            free(row);
            fclose(file);
            return 0;
        }
        {
            size_t resource;
            for (resource = 0; resource < state->resource_count; ++resource) {
                if (row[resource] > state->total[resource]) {
                    fprintf(stderr,
                            "Erro: maximo do cliente excede o total do recurso %zu em %s:%zu.\n",
                            resource, CUSTOMER_FILE, line_number);
                    free(row);
                    fclose(file);
                    return 0;
                }
            }
        }
        if (state->customer_count == capacity) {
            next_capacity = capacity == 0 ? 8 : capacity * 2;
            if (next_capacity < capacity || next_capacity > SIZE_MAX / state->resource_count ||
                next_capacity * state->resource_count > SIZE_MAX / sizeof(int)) {
                fprintf(stderr, "Erro: arquivo de clientes grande demais.\n");
                free(row);
                fclose(file);
                return 0;
            }
            grown = realloc(state->maximum, next_capacity * state->resource_count * sizeof(int));
            if (grown == NULL) {
                fprintf(stderr, "Erro: memoria insuficiente.\n");
                free(row);
                fclose(file);
                return 0;
            }
            state->maximum = grown;
            capacity = next_capacity;
        }
        memcpy(state->maximum + state->customer_count * state->resource_count,
               row, state->resource_count * sizeof(int));
        ++state->customer_count;
    }
    free(row);
    if (status < 0 || ferror(file)) {
        fclose(file);
        return 0;
    }
    fclose(file);
    if (state->customer_count == 0) {
        fprintf(stderr, "Erro: %s nao contem clientes.\n", CUSTOMER_FILE);
        return 0;
    }

    if (!checked_calloc((void **) &state->allocation,
                        state->customer_count * state->resource_count, sizeof(int)) ||
        !checked_calloc((void **) &state->need,
                        state->customer_count * state->resource_count, sizeof(int))) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        return 0;
    }
    memcpy(state->need, state->maximum,
           state->customer_count * state->resource_count * sizeof(int));
    return 1;
}

static int parse_client_id(const char *text, size_t *customer_count, size_t *client)
{
    char *end = NULL;
    unsigned long long parsed;

    if (text == NULL || *text == '\0' || *text == '-') {
        return 0;
    }
    errno = 0;
    parsed = strtoull(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed >= *customer_count) {
        return 0;
    }
    *client = (size_t) parsed;
    return 1;
}

static int parse_vector(char **tokens, size_t token_count, size_t resource_count,
                        int *values)
{
    size_t i;
    if (token_count != resource_count) {
        return 0;
    }
    for (i = 0; i < resource_count; ++i) {
        if (!parse_nonnegative_int(tokens[i], &values[i])) {
            return 0;
        }
    }
    return 1;
}

static int is_safe(const struct banker_state *state)
{
    int *work = NULL;
    unsigned char *finished = NULL;
    size_t completed = 0;
    size_t customer;
    size_t resource;
    int progress;
    int safe = 0;

    if (!checked_calloc((void **) &work, state->resource_count, sizeof(*work)) ||
        !checked_calloc((void **) &finished, state->customer_count, sizeof(*finished))) {
        free(work);
        free(finished);
        return 0;
    }
    memcpy(work, state->available, state->resource_count * sizeof(*work));

    do {
        progress = 0;
        for (customer = 0; customer < state->customer_count; ++customer) {
            int can_finish = 1;
            if (finished[customer]) {
                continue;
            }
            for (resource = 0; resource < state->resource_count; ++resource) {
                size_t index = customer * state->resource_count + resource;
                if (state->need[index] > work[resource]) {
                    can_finish = 0;
                    break;
                }
            }
            if (can_finish) {
                for (resource = 0; resource < state->resource_count; ++resource) {
                    work[resource] += state->allocation[customer * state->resource_count + resource];
                }
                finished[customer] = 1;
                ++completed;
                progress = 1;
            }
        }
    } while (progress);

    safe = completed == state->customer_count;
    free(work);
    free(finished);
    return safe;
}

static void print_vector(FILE *out, const int *values, size_t count)
{
    size_t i;
    for (i = 0; i < count; ++i) {
        fprintf(out, "%s%d", i == 0 ? "" : " ", values[i]);
    }
}

static int print_state(FILE *out, const struct banker_state *state)
{
    size_t customer;

    if (fprintf(out, "MAXIMUM | ALLOCATION | NEED\n") < 0) {
        return 0;
    }
    for (customer = 0; customer < state->customer_count; ++customer) {
        const size_t offset = customer * state->resource_count;
        print_vector(out, state->maximum + offset, state->resource_count);
        fprintf(out, " | ");
        print_vector(out, state->allocation + offset, state->resource_count);
        fprintf(out, " | ");
        print_vector(out, state->need + offset, state->resource_count);
        fprintf(out, "\n");
    }
    fprintf(out, "AVAILABLE ");
    print_vector(out, state->available, state->resource_count);
    fprintf(out, "\n");
    return !ferror(out);
}

static int process_command_line(char *line, size_t line_number,
                                struct banker_state *state, FILE *out)
{
    char *tokens[MAX_LINE_LENGTH / 2];
    char *cursor = line;
    size_t token_count = 0;
    size_t customer;
    size_t resource;
    int *request = NULL;
    int result = 1;

    while (*(cursor = skip_spaces(cursor)) != '\0' && *cursor != '\n' && *cursor != '\r') {
        if (token_count == sizeof(tokens) / sizeof(tokens[0])) {
            fprintf(stderr, "Erro: comando muito longo em %s:%zu.\n", COMMAND_FILE, line_number);
            return 0;
        }
        tokens[token_count++] = cursor;
        while (*cursor != '\0' && *cursor != '\n' && *cursor != '\r' &&
               !isspace((unsigned char) *cursor)) {
            ++cursor;
        }
        if (*cursor != '\0') {
            *cursor++ = '\0';
        }
    }
    if (token_count == 0) {
        fprintf(stderr, "Erro: linha vazia em %s:%zu.\n", COMMAND_FILE, line_number);
        return 0;
    }
    if (strcmp(tokens[0], "*") == 0) {
        if (token_count != 1) {
            fprintf(stderr, "Erro: comando '*' invalido em %s:%zu.\n", COMMAND_FILE, line_number);
            return 0;
        }
        return print_state(out, state);
    }
    if (strcmp(tokens[0], "RQ") != 0 && strcmp(tokens[0], "RL") != 0) {
        fprintf(stderr, "Erro: comando desconhecido '%s' em %s:%zu.\n",
                tokens[0], COMMAND_FILE, line_number);
        return 0;
    }
    if (token_count < 2 || !parse_client_id(tokens[1], &state->customer_count, &customer)) {
        fprintf(stderr, "Erro: cliente invalido em %s:%zu.\n", COMMAND_FILE, line_number);
        return 0;
    }
    if (!checked_calloc((void **) &request, state->resource_count, sizeof(*request))) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        return 0;
    }
    if (!parse_vector(tokens + 2, token_count - 2, state->resource_count, request)) {
        fprintf(stderr, "Erro: vetor de recursos invalido em %s:%zu; esperados %zu valores inteiros nao negativos.\n",
                COMMAND_FILE, line_number, state->resource_count);
        free(request);
        return 0;
    }

    if (strcmp(tokens[0], "RQ") == 0) {
        for (resource = 0; resource < state->resource_count; ++resource) {
            size_t index = customer * state->resource_count + resource;
            if (request[resource] > state->need[index]) {
                fprintf(out, "The customer %zu request ", customer);
                print_vector(out, request, state->resource_count);
                fprintf(out, " was denied because exceed its maximum allocation\n");
                free(request);
                return !ferror(out);
            }
        }
        for (resource = 0; resource < state->resource_count; ++resource) {
            if (request[resource] > state->available[resource]) {
                fprintf(out, "The resources ");
                print_vector(out, state->available, state->resource_count);
                fprintf(out, " are not enough to customer %zu request ", customer);
                print_vector(out, request, state->resource_count);
                fprintf(out, "\n");
                free(request);
                return !ferror(out);
            }
        }

        for (resource = 0; resource < state->resource_count; ++resource) {
            size_t index = customer * state->resource_count + resource;
            state->available[resource] -= request[resource];
            state->allocation[index] += request[resource];
            state->need[index] -= request[resource];
        }
        if (is_safe(state)) {
            fprintf(out, "Allocate to customer %zu the resources ", customer);
            print_vector(out, request, state->resource_count);
            fprintf(out, "\n");
        } else {
            for (resource = 0; resource < state->resource_count; ++resource) {
                size_t index = customer * state->resource_count + resource;
                state->available[resource] += request[resource];
                state->allocation[index] -= request[resource];
                state->need[index] += request[resource];
            }
            fprintf(out, "The customer %zu request ", customer);
            print_vector(out, request, state->resource_count);
            fprintf(out, " was denied because result in an unsafe state\n");
        }
    } else {
        for (resource = 0; resource < state->resource_count; ++resource) {
            size_t index = customer * state->resource_count + resource;
            if (request[resource] > state->allocation[index]) {
                fprintf(stderr, "Erro: RL excede a alocacao do cliente %zu em %s:%zu.\n",
                        customer, COMMAND_FILE, line_number);
                result = 0;
                goto cleanup;
            }
        }
        for (resource = 0; resource < state->resource_count; ++resource) {
            size_t index = customer * state->resource_count + resource;
            state->available[resource] += request[resource];
            state->allocation[index] -= request[resource];
            state->need[index] += request[resource];
        }
        fprintf(out, "Release from customer %zu the resources ", customer);
        print_vector(out, request, state->resource_count);
        fprintf(out, "\n");
    }

    result = !ferror(out);
cleanup:
    free(request);
    return result;
}

static int process_commands(struct banker_state *state)
{
    FILE *input = fopen(COMMAND_FILE, "r");
    FILE *output = NULL;
    char line[MAX_LINE_LENGTH];
    size_t line_number = 0;
    int status;
    int success = 0;

    if (input == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s: %s\n", COMMAND_FILE, strerror(errno));
        return 0;
    }
    output = fopen(RESULT_FILE, "w");
    if (output == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar %s: %s\n", RESULT_FILE, strerror(errno));
        fclose(input);
        return 0;
    }

    while ((status = read_line(input, line, sizeof(line), &line_number, COMMAND_FILE)) > 0) {
        if (!process_command_line(line, line_number, state, output)) {
            goto cleanup;
        }
    }
    if (status < 0 || ferror(input)) {
        goto cleanup;
    }
    if (fflush(output) != 0 || ferror(output)) {
        fprintf(stderr, "Erro: falha ao gravar %s.\n", RESULT_FILE);
        goto cleanup;
    }
    success = 1;

cleanup:
    if (fclose(input) != 0) {
        success = 0;
    }
    if (fclose(output) != 0) {
        success = 0;
    }
    return success;
}

int main(int argc, char *argv[])
{
    struct banker_state state = {0};
    size_t i;
    int success = 0;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <instancias-recurso-1> [instancias-recurso-2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }
    state.resource_count = (size_t) (argc - 1);
    if (!checked_calloc((void **) &state.total, state.resource_count, sizeof(*state.total)) ||
        !checked_calloc((void **) &state.available, state.resource_count, sizeof(*state.available))) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        free_state(&state);
        return EXIT_FAILURE;
    }
    for (i = 0; i < state.resource_count; ++i) {
        if (!parse_resource_count(argv[i + 1], &state.total[i])) {
            fprintf(stderr, "Erro: quantidade de recursos invalida: '%s'\n", argv[i + 1]);
            goto cleanup;
        }
        state.available[i] = state.total[i];
    }
    if (!load_customers(&state)) {
        goto cleanup;
    }
    success = process_commands(&state);

cleanup:
    free_state(&state);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
