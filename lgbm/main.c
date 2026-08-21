#include <stdio.h>
#include <stdlib.h>

#include "interativo.h"

int main(int argc, char *argv[])
{
    printf("Iniciando ProcessFlow...\n");

    if (argc == 1) {
        printf("Modo interativo\n");
        interativo(argc, argv);
    } else if (argc == 2) {
        printf("Modo workflow\n");
    }
    else {
        printf("Número de argumentos inválido\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}