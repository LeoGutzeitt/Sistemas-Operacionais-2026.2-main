#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int interativo(int argc, char *argv[]) {
    char linha[256];
    char *args[20];


    while (1) {
        printf("ProcessFlow> ");

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            break;
        }

        linha[strcspn(linha, "\n")] = '\0';

        int i = 0;
        char *token = strtok(linha, " ");

        while (token != NULL && i < 19) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        args[i] = NULL;

        if (args[0] == NULL) {
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            execvp(args[0], args);

            perror("execvp");
            exit(1);
        } else {
            wait(NULL);
        }
    }

    printf("Saindo do ProcessFlow\n");
    return 0;
}
