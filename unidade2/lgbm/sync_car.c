#include "sync_car.h"

/* Inicializa a estacao. */
void station_init(struct station *station) {
    pthread_mutex_init(&station->lock, NULL);
    pthread_cond_init(&station->train_arrived, NULL);
    pthread_cond_init(&station->train_loaded, NULL);
    station->waiting_passengers = 0;
    station->available_seats = 0;
    station->boarding_passengers = 0;
}

/* Espera o embarque terminar antes de liberar o vagao. */
void station_load_car(struct station *station, int count) {
    pthread_mutex_lock(&station->lock);

    station->available_seats = count;

    /* Avisa que o vagao chegou. */
    pthread_cond_broadcast(&station->train_arrived);

    /* Espera enquanto houver passageiros para embarcar ou sentar. */
    while ((station->waiting_passengers > 0 && station->available_seats > 0) ||
           (station->boarding_passengers > 0)) {
        pthread_cond_wait(&station->train_loaded, &station->lock);
    }

    /* Fecha o embarque antes da partida. */
    station->available_seats = 0;

    pthread_mutex_unlock(&station->lock);
}

/* Espera uma vaga no vagao. */
void station_wait_for_car(struct station *station) {
    pthread_mutex_lock(&station->lock);

    station->waiting_passengers++;

    /* Confere as vagas novamente a cada despertar. */
    while (station->available_seats == 0) {
        pthread_cond_wait(&station->train_arrived, &station->lock);
    }

    /* Reserva uma vaga e registra o inicio do embarque. */
    station->waiting_passengers--;
    station->available_seats--;
    station->boarding_passengers++;

    pthread_mutex_unlock(&station->lock);
}

/* Registra que o passageiro sentou. */
void station_on_board(struct station *station) {
    pthread_mutex_lock(&station->lock);

    station->boarding_passengers--;

    /* Avisa o vagao quando todos sentaram e ele pode partir. */
    if (station->boarding_passengers == 0 &&
        (station->available_seats == 0 || station->waiting_passengers == 0)) {
        pthread_cond_signal(&station->train_loaded);
    }

    pthread_mutex_unlock(&station->lock);
}
