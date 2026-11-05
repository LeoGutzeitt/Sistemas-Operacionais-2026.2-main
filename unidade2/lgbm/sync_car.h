#ifndef SYNC_CAR_H
#define SYNC_CAR_H

#include <pthread.h>

/* Estado da estacao, protegido por um unico mutex. */
struct station {
    pthread_mutex_t lock;           /* Mutex da estacao */
    pthread_cond_t train_arrived;   /* Avisa sobre vagas */
    pthread_cond_t train_loaded;    /* Avisa o fim do embarque */
    int waiting_passengers;         /* Passageiros na fila */
    int available_seats;            /* Vagas livres */
    int boarding_passengers;        /* Passageiros que ainda nao sentaram */
};

/* Inicializa a estacao. */
void station_init(struct station *station);

/* Recebe o vagao e aguarda o fim do embarque. */
void station_load_car(struct station *station, int count);

/* Espera e reserva uma vaga. */
void station_wait_for_car(struct station *station);

/* Confirma que o passageiro sentou. */
void station_on_board(struct station *station);

#endif /* SYNC_CAR_H */
