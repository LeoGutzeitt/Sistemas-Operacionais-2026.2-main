#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "scheduler.h"

char* extract_login() {
    /* Placeholder: substitua pela lógica real de extração de login */
    /* Para esta implementação, assumimos login fornecido ou usamos "mla" como padrão */
    return "mla";
}

char* build_output_filename(const char *algorithm) {
    char *login = extract_login();
    char *output = (char *)malloc(256);
    if (!output) {
        fprintf(stderr, "Erro: Falha na alocação de memória\n");
        return NULL;
    }
    
    sprintf(output, "%s_%s.out", algorithm, login);
    return output;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <algoritmo> <arquivo_entrada>\n", argv[0]);
        fprintf(stderr, "  algoritmo: rate ou edf\n");
        return 1;
    }
    
    const char *algorithm = argv[1];
    const char *input_file = argv[2];
    
    /* Validar algoritmo */
    if (strcmp(algorithm, "rate") != 0 && strcmp(algorithm, "edf") != 0) {
        fprintf(stderr, "Erro: Algoritmo deve ser 'rate' ou 'edf'\n");
        return 1;
    }
    
    /* Fazer parsing da entrada */
    Schedule *schedule = parse_input(input_file);
    if (!schedule) {
        return 1;
    }
    
    /* Construir nome do arquivo de saída */
    char *output_file = build_output_filename(algorithm);
    if (!output_file) {
        free_schedule(schedule);
        return 1;
    }
    
    /* Executar escalonador */
    if (strcmp(algorithm, "rate") == 0) {
        execute_rate_monotonic(schedule, output_file);
    } else {
        execute_edf(schedule, output_file);
    }
    
    /* Limpeza */
    free(output_file);
    free_schedule(schedule);
    
    return 0;
}
