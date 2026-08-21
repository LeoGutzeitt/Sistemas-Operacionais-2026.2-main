#include <stdio.h>
#include <stdlib.h>

#include "interativo.h"
#include "workflow.h"

int main(int argc, char *argv[])
{
    printf("Iniciando ProcessFlow...\n");

    if (argc == 1) {
        printf("Modo interativo\n");
        interativo(argc, argv);
    } else if (argc == 2) {
    printf("Modo workflow\n");

    Catalogo catalogo;
    catalogo_inicializar(&catalogo);

    int resultado = workflow_executar(argv[1], &catalogo);

    catalogo_liberar(&catalogo);

    if (resultado == 0) {
        return EXIT_FAILURE;
    }
    } else {
        printf("Número de argumentos inválido\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}