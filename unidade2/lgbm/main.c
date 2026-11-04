#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "sync_car.h"

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

typedef struct {
    int id;
    struct station *station;
    int *boarded_count;
    pthread_mutex_t *count_lock;
    int simulate_delay;
} PassengerArg;

static void* passenger_thread(void *arg) {
    PassengerArg *p = (PassengerArg*)arg;

    station_wait_for_car(p->station);

    if (p->simulate_delay) {
        /* Simula tempo de deslocamento físico até o assento */
        sleep_ms(5);
    }

    if (p->boarded_count && p->count_lock) {
        pthread_mutex_lock(p->count_lock);
        (*p->boarded_count)++;
        pthread_mutex_unlock(p->count_lock);
    }

    station_on_board(p->station);

    free(p);
    return NULL;
}

static int run_test_scenario(const char *name, int num_passengers, int car_capacities[], int num_cars, int simulate_delay) {
    printf("============================================================\n");
    printf("TESTE: %s\n", name);
    printf("Passageiros totais: %d | Carros programados: %d\n", num_passengers, num_cars);
    fflush(stdout);

    struct station station;
    station_init(&station);

    int boarded_count = 0;
    pthread_mutex_t count_lock;
    pthread_mutex_init(&count_lock, NULL);

    pthread_t *pass_threads = (pthread_t*)malloc(sizeof(pthread_t) * (num_passengers > 0 ? num_passengers : 1));

    /* Criação das threads de passageiros */
    for (int i = 0; i < num_passengers; i++) {
        PassengerArg *arg = (PassengerArg*)malloc(sizeof(PassengerArg));
        arg->id = i + 1;
        arg->station = &station;
        arg->boarded_count = &boarded_count;
        arg->count_lock = &count_lock;
        arg->simulate_delay = simulate_delay;
        pthread_create(&pass_threads[i], NULL, passenger_thread, arg);
    }

    /* Pausa para assegurar que os passageiros chegaram à estação */
    sleep_ms(25);

    /* Processamento sequencial dos carros (um carro por vez na estação) */
    int total_seats = 0;
    for (int c = 0; c < num_cars; c++) {
        int seats = car_capacities[c];
        total_seats += seats;
        printf(" -> Carro #%d chegou a estacao com %d assentos.\n", c + 1, seats);
        fflush(stdout);

        int before = boarded_count;
        station_load_car(&station, seats);
        int after = boarded_count;

        printf(" <- Carro #%d partiu com %d passageiros embarcados nesta viagem.\n", c + 1, (after - before));
        fflush(stdout);
        sleep_ms(15);
    }

    int expected_boarded = num_passengers < total_seats ? num_passengers : total_seats;
    if (total_seats == 0) expected_boarded = 0;

    int success = (boarded_count == expected_boarded);
    printf("Resultado parcial: Embarcados = %d / Esperado = %d -> %s\n",
           boarded_count, expected_boarded, success ? "SUCESSO [PASS]" : "FALHA [FAIL]");
    fflush(stdout);

    /* Se restaram passageiros na estação (ex: teste de carro menor que demanda),
       despacha um carro de limpeza para permitir que todas as threads finalizem */
    if (boarded_count < num_passengers) {
        int remaining = num_passengers - boarded_count;
        printf(" -> Despachando vagao de recolhimento para os %d passageiros restantes...\n", remaining);
        fflush(stdout);
        station_load_car(&station, remaining);
    }

    /* Aguarda término de todas as threads criadas */
    for (int i = 0; i < num_passengers; i++) {
        pthread_join(pass_threads[i], NULL);
    }

    free(pass_threads);
    pthread_mutex_destroy(&count_lock);
    printf("============================================================\n\n");
    fflush(stdout);
    return success;
}

/*
 * Teste específico para validar embarque simultâneo (Requisito 5):
 * Diversos passageiros chamam station_wait_for_car, e a função DEVE retornar
 * para cada passageiro antes de qualquer um chamar station_on_board.
 */
typedef struct {
    struct station *station;
    int id;
    int *wait_returned_count;
    pthread_mutex_t *sync_lock;
    pthread_cond_t *can_board_cond;
    int *can_board_flag;
} SimPassengerArg;

static void* sim_passenger_thread(void *arg) {
    SimPassengerArg *p = (SimPassengerArg*)arg;

    station_wait_for_car(p->station);

    /* Notifica que retornou com sucesso de station_wait_for_car */
    pthread_mutex_lock(p->sync_lock);
    (*p->wait_returned_count)++;
    while (!(*p->can_board_flag)) {
        pthread_cond_wait(p->can_board_cond, p->sync_lock);
    }
    pthread_mutex_unlock(p->sync_lock);

    /* Chama station_on_board apenas após todos os 5 terem retornado de wait_for_car */
    station_on_board(p->station);

    free(p);
    return NULL;
}

static void* sim_car_thread(void *arg) {
    station_load_car((struct station*)arg, 5);
    return NULL;
}

static int test_simultaneous_boarding() {
    printf("============================================================\n");
    printf("TESTE: Embarque Simultaneo (Requisito 5)\n");
    printf("Verifica se passageiros retornam de wait_for_car antes do on_board\n");
    fflush(stdout);

    struct station station;
    station_init(&station);

    int num_passengers = 5;
    int wait_returned_count = 0;
    int can_board_flag = 0;
    pthread_mutex_t sync_lock;
    pthread_cond_t can_board_cond;
    pthread_mutex_init(&sync_lock, NULL);
    pthread_cond_init(&can_board_cond, NULL);

    pthread_t threads[5];
    for (int i = 0; i < num_passengers; i++) {
        SimPassengerArg *p = (SimPassengerArg*)malloc(sizeof(SimPassengerArg));
        p->station = &station;
        p->id = i + 1;
        p->wait_returned_count = &wait_returned_count;
        p->sync_lock = &sync_lock;
        p->can_board_cond = &can_board_cond;
        p->can_board_flag = &can_board_flag;
        pthread_create(&threads[i], NULL, sim_passenger_thread, p);
    }

    sleep_ms(25);

    /* Thread do vagão que chama load_car com 5 assentos */
    pthread_t car_tid;
    pthread_create(&car_tid, NULL, sim_car_thread, &station);

    /* Aguarda até que todos os 5 passageiros tenham retornado de station_wait_for_car */
    int timeout_ms = 0;
    while (timeout_ms < 2000) {
        pthread_mutex_lock(&sync_lock);
        int c = wait_returned_count;
        pthread_mutex_unlock(&sync_lock);
        if (c == 5) break;
        sleep_ms(10);
        timeout_ms += 10;
    }

    pthread_mutex_lock(&sync_lock);
    int returned = wait_returned_count;
    pthread_mutex_unlock(&sync_lock);

    int all_returned_first = (returned == 5);
    printf("Passageiros que retornaram de wait_for_car simultaneamente: %d / 5\n", returned);
    fflush(stdout);

    /* Libera todos os passageiros para chamarem station_on_board */
    pthread_mutex_lock(&sync_lock);
    can_board_flag = 1;
    pthread_cond_broadcast(&can_board_cond);
    pthread_mutex_unlock(&sync_lock);

    /* Aguarda o vagão partir e as threads finalizarem */
    pthread_join(car_tid, NULL);
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&sync_lock);
    pthread_cond_destroy(&can_board_cond);

    printf("Resultado: %s\n", all_returned_first ? "SUCESSO [PASS]" : "FALHA [FAIL]");
    printf("============================================================\n\n");
    fflush(stdout);
    return all_returned_first;
}

int main(int argc, char *argv[]) {
    printf("############################################################\n");
    printf("#  METROREC - Sistema de Sincronizacao de Vagoes (LGBM)   #\n");
    printf("#  Sistemas Operacionais 2026.2                           #\n");
    printf("############################################################\n\n");
    fflush(stdout);

    /* Modo interativo / customizado */
    if (argc == 3 && strcmp(argv[1], "test") != 0) {
        int passengers = atoi(argv[1]);
        int capacity = atoi(argv[2]);
        if (passengers <= 0 || capacity <= 0) {
            fprintf(stderr, "Uso: %s <numero_passageiros> <capacidade_carro>\n", argv[0]);
            return 1;
        }
        int num_cars = (passengers + capacity - 1) / capacity;
        int *caps = (int*)malloc(sizeof(int) * num_cars);
        for (int i = 0; i < num_cars; i++) caps[i] = capacity;

        int ok = run_test_scenario("Cenario Customizado", passengers, caps, num_cars, 1);
        free(caps);
        return ok ? 0 : 1;
    }

    /* Bateria completa de testes automatizados */
    int total_tests = 7;
    int passed_tests = 0;

    /* Teste 1: Capacidade == Passageiros */
    int caps1[] = {10};
    passed_tests += run_test_scenario("Capacidade Igual ao Numero de Passageiros (10 x 10)", 10, caps1, 1, 1);

    /* Teste 2: Mais assentos que passageiros */
    int caps2[] = {15};
    passed_tests += run_test_scenario("Vagao com Mais Vagas que Passageiros (15 vagas, 6 passageiros)", 6, caps2, 1, 1);

    /* Teste 3: Menos assentos que passageiros */
    int caps3[] = {4};
    passed_tests += run_test_scenario("Vagao com Menos Vagas (4 vagas, 10 passageiros -> sobram 6)", 10, caps3, 1, 1);

    /* Teste 4: Multiplos carros sequenciais ate esvaziar estacao */
    int caps4[] = {4, 5, 6};
    passed_tests += run_test_scenario("Multiplos Carros Sequenciais (15 passageiros em 3 carros: 4, 5 e 6)", 15, caps4, 3, 1);

    /* Teste 5: Estacao vazia ou carro sem vagas (Tratamento de casos limites) */
    int caps5[] = {10};
    passed_tests += run_test_scenario("Carro Chega com Estacao Vazia (0 passageiros, 10 vagas)", 0, caps5, 1, 0);

    /* Teste 6: Embarque simultaneo */
    passed_tests += test_simultaneous_boarding();

    /* Teste 7: Stress test com 50 passageiros e multiplos vagoes */
    int caps7[] = {8, 12, 15, 15};
    passed_tests += run_test_scenario("Stress Test (50 passageiros em 4 vagoes: 8, 12, 15, 15)", 50, caps7, 4, 1);

    printf("************************************************************\n");
    printf("RESUMO DOS TESTES: %d de %d APROVADOS (%s)\n",
           passed_tests, total_tests, (passed_tests == total_tests) ? "100% OK" : "HOUVE FALHAS");
    printf("************************************************************\n");
    fflush(stdout);

    return (passed_tests == total_tests) ? 0 : 1;
}
