//fazer threads que redirecionam para 3 arquivos de saida diferentes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <arquivo1> <arquivo2> <arquivo3>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *arquivos[3];
    for (int i = 0; i < 3; i++) {
        arquivos[i] = fopen(argv[i + 1], "w");
        if (arquivos[i] == NULL) {
            fprintf(stderr, "Erro ao abrir o arquivo %s\n", argv[i + 1]);
            return EXIT_FAILURE;
        }
    }

    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        int *index = malloc(sizeof(int));
        *index = i;
        pthread_create(&threads[i], NULL, (void *(*)(void *))fputs, arquivos[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
        fclose(arquivos[i]);
    }

    return EXIT_SUCCESS;
}