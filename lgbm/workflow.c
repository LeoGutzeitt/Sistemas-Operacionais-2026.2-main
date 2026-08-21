#include "workflow.h"
#include "catalogo.h"
#include "comando.h"
#include <stdio.h>
#include <string.h>


//validar caminho do arquivo de workflow
int validar_caminho(const char *caminho) {
    FILE *arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        return 0; // Caminho inválido
    }
    fclose(arquivo);
    return 1; // Caminho válido
}       
int workflow_executar(const char *caminho, Catalogo *catalogo){
    
    if(validar_caminho(caminho)){
        FILE *arquivo = fopen(caminho, "r");
        if (arquivo == NULL) {
            printf("Erro ao abrir o arquivo de workflow.\n");
            return 0;
        }
        char linha[256];
        while (fgets(linha, sizeof(linha), arquivo) != NULL) {
            // Remover o caractere de nova linha, se presente
            linha[strcspn(linha, "\n")] = '\0';
            // Processar a linha do workflow
            printf("Executando comando: %s\n", linha);

        char *argumentos[TASK_MAX_ARGS + 2];
        int quantidade = 0;

        char *token = strtok(linha, " \t");

        while (token != NULL && quantidade < TASK_MAX_ARGS + 1) {
            argumentos[quantidade++] = token;
            token = strtok(NULL, " \t");
        }

        argumentos[quantidade] = NULL;

        if (quantidade > 0) {
            if (comando_executar(catalogo, quantidade, argumentos) == COMANDO_SAIR) {
                break;
            }
        }
    }
        fclose(arquivo);
        return 1; // Workflow executado com sucesso
    }   
    else {
        printf("Caminho inválido para o arquivo de workflow.\n");
        return 0; // Caminho inválido
        
    }
}