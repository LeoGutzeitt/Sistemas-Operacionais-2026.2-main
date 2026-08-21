#include <stdio.h>
#include <string.h>

#include "comando.h"

int interativo(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    char linha[256];
    char *args[TASK_MAX_ARGS + 2];
    Catalogo catalogo;

    catalogo_inicializar(&catalogo);


    while (1) {
        printf("ProcessFlow> ");

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            break;
        }

        linha[strcspn(linha, "\n")] = '\0';

        int i = 0;
        char *token = strtok(linha, " \t");

        while (token != NULL && i < TASK_MAX_ARGS + 1) {
            args[i++] = token;
            token = strtok(NULL, " \t");
        }

        args[i] = NULL;

        if (args[0] == NULL) {
            continue;
        }

        comando_executar(&catalogo, i, args);
    }

    catalogo_liberar(&catalogo);
    printf("Saindo do ProcessFlow\n");
    return 0;
}
