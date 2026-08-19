#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "executador.h"
#include "tarefas.h"

static pthread_t *trabalhadores = NULL;
static int trabalhadores_qt = 0;

static void *trabalhador_main(void *arg) {
    (void)arg;
    for (;;) {
        tarefa_t *t = tarefas_proxima();
        if (!t) break;
        printf("[trabalhador] executando tarefa %d: %s\n", t->id, t->comando ? t->comando : "(sem comando)");
        if (t->comando) {
            int rc = system(t->comando);
            printf("[trabalhador] tarefa %d finalizada com código %d\n", t->id, rc);
        }
        tarefa_liberar(t);
    }
    return NULL;
}

void executador_iniciar(int n_trabalhadores) {
    if (n_trabalhadores <= 0) n_trabalhadores = 1;
    trabalhadores_qt = n_trabalhadores;
    trabalhadores = calloc(trabalhadores_qt, sizeof(pthread_t));
    for (int i = 0; i < trabalhadores_qt; ++i) {
        pthread_create(&trabalhadores[i], NULL, trabalhador_main, NULL);
    }
}

void executador_esperar(void) {
    for (int i = 0; i < trabalhadores_qt; ++i) {
        pthread_join(trabalhadores[i], NULL);
    }
    free(trabalhadores);
    trabalhadores = NULL;
    trabalhadores_qt = 0;
}
