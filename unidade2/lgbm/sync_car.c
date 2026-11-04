#include "sync_car.h"

/*
 * Inicializa a struct station.
 * Configura o único mutex e as variáveis de condição,
 * além de zerar os contadores de passageiros e assentos.
 */
void station_init(struct station *station) {
    pthread_mutex_init(&station->lock, NULL);
    pthread_cond_init(&station->train_arrived, NULL);
    pthread_cond_init(&station->train_loaded, NULL);
    station->waiting_passengers = 0;
    station->available_seats = 0;
    station->boarding_passengers = 0;
}

/*
 * Chamada quando um vagão chega à estação com 'count' assentos disponíveis.
 * 
 * Regra: Não retorna até que o vagão esteja satisfatoriamente preenchido:
 * todos os passageiros sentados, e:
 * - ou o vagão está cheio (available_seats == 0);
 * - ou todos os passageiros que estavam esperando embarcaram (waiting_passengers == 0).
 */
void station_load_car(struct station *station, int count) {
    pthread_mutex_lock(&station->lock);

    station->available_seats = count;

    /*
     * Acorda todos os passageiros que aguardam na plataforma.
     * pthread_cond_broadcast é utilizado para notificar todos concorrentemente.
     */
    pthread_cond_broadcast(&station->train_arrived);

    /*
     * O vagão bloqueia e aguarda enquanto:
     * 1) Ainda existirem vagas disponíveis E passageiros esperando para embarcar; OU
     * 2) Existirem passageiros que receberam permissão de embarque mas ainda não
     *    concluíram a chamada station_on_board (boarding_passengers > 0).
     * 
     * Isso assegura ausência total de busy-waiting.
     */
    while ((station->waiting_passengers > 0 && station->available_seats > 0) ||
           (station->boarding_passengers > 0)) {
        pthread_cond_wait(&station->train_loaded, &station->lock);
    }

    /*
     * Vagão pronto para partir.
     * Zera as vagas do vagão para que passageiros futuros não tentem embarcar nele.
     */
    station->available_seats = 0;

    pthread_mutex_unlock(&station->lock);
}

/*
 * Chamada quando um passageiro chega à estação.
 * 
 * Regra: Bloqueia até que haja um vagão presente com pelo menos um assento livre.
 * Permite que múltiplos passageiros retornem antes de chamar station_on_board.
 */
void station_wait_for_car(struct station *station) {
    pthread_mutex_lock(&station->lock);

    station->waiting_passengers++;

    /*
     * Aguarda enquanto não houver assentos disponíveis.
     * O loop while protege contra acordares espúrios (spurious wakeups).
     */
    while (station->available_seats == 0) {
        pthread_cond_wait(&station->train_arrived, &station->lock);
    }

    /*
     * Reserva o assento:
     * O passageiro sai da fila de espera e entra no estado de embarque ativo.
     */
    station->waiting_passengers--;
    station->available_seats--;
    station->boarding_passengers++;

    pthread_mutex_unlock(&station->lock);
}

/*
 * Chamada pelo passageiro imediatamente após embarcar e ocupar seu assento.
 * Informa ao vagão que o passageiro completou o embarque.
 */
void station_on_board(struct station *station) {
    pthread_mutex_lock(&station->lock);

    station->boarding_passengers--;

    /*
     * Se não há mais ninguém no meio do embarque (boarding_passengers == 0),
     * e o vagão atingiu a condição de partida (lotado ou estação vazia),
     * sinaliza o vagão para que continue sua viagem.
     */
    if (station->boarding_passengers == 0 &&
        (station->available_seats == 0 || station->waiting_passengers == 0)) {
        pthread_cond_signal(&station->train_loaded);
    }

    pthread_mutex_unlock(&station->lock);
}
