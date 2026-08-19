// Esqueleto mínimo: carrega jobs, inicia executor e aguarda conclusão

#include <stdio.h>
#include <stdlib.h>

#include "tarefas.h"
#include "executador.h"

int main(int argc, char **argv) {
    const char *jobs_file = (argc > 1) ? argv[1] : "tasks.txt";

    if (tarefas_carregar_de_arquivo(jobs_file) != 0) {
        fprintf(stderr, "Erro ao carregar tarefas de %s\n", jobs_file);
        return 1;
    }

    // Inicia executador com 2 trabalhadores (ajuste conforme necessidade)
    executador_iniciar(2);
    executador_esperar();

    tarefas_limpar();
    return 0;
}