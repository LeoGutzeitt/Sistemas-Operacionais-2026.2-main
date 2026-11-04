#ifndef SYNC_CAR_H
#define SYNC_CAR_H

#include <pthread.h>

/*
 * Estrutura da Estação de Metrô (METROREC)
 * 
 * Restrições do projeto:
 * - Apenas UM único lock (pthread_mutex_t) permitido por struct station.
 * - Proibido o uso de semáforos ou outras primitivas.
 * - Deve permitir que passageiros embarquem simultaneamente.
 * - Livre de espera ocupada (busy-wait).
 */
struct station {
    pthread_mutex_t lock;           /* Único mutex da estação */
    pthread_cond_t train_arrived;   /* Condição para passageiros aguardando vagão com vagas */
    pthread_cond_t train_loaded;    /* Condição para o vagão aguardando embarque completo */
    int waiting_passengers;         /* Quantidade de passageiros aguardando na plataforma */
    int available_seats;            /* Assentos disponíveis no vagão presente */
    int boarding_passengers;        /* Passageiros autorizados que estão no processo de embarque */
};

/*
 * Inicializa a struct station e suas variáveis de sincronização.
 */
void station_init(struct station *station);

/*
 * Chamada quando um vagão chega à estação com 'count' assentos livres.
 * Não retorna até o vagão estar satisfatoriamente preenchido:
 * todos os passageiros sentados, e ou o vagão está cheio ou todos os
 * passageiros que estavam esperando embarcaram.
 */
void station_load_car(struct station *station, int count);

/*
 * Chamada quando um passageiro chega à estação.
 * Não retorna até haver um vagão na estação com vaga livre para ele.
 */
void station_wait_for_car(struct station *station);

/*
 * Chamada pelo passageiro após embarcar e ocupar seu assento.
 * Informa ao vagão que o passageiro sentou-se.
 */
void station_on_board(struct station *station);

#endif /* SYNC_CAR_H */
