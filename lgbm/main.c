// Esqueleto mínimo: carrega jobs, inicia executor e aguarda conclusão

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tarefas.h"
#include "interativo.h"



int main(int argc, char *argv[]) {


    printf("Iniciando ProcessFlow...\n");
  // Receber o imput para ir pro interativo ou workflow

    if(argc==1){
        printf("Modo interativo\n");
        interativo(argc, argv);
    }
  else if(argc==2){
      printf("Modo workflow\n");
      //   Workflow(argc, argv);  
      //
    }
    else{
        printf("Número de argumentos inválido\n");
        exit(1);
    }

 return 0;
}