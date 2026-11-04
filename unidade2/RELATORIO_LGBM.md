# Relatório Técnico - Sincronização de Metrô (METROREC)
## Implementação 4 - Concorrência e Sincronização com Pthreads

**Disciplina:** Sistemas Operacionais (2026.2)  
**Instituição:** CESAR School  
**Aluno:** Leonardo Gutzeit Borgmann Matos  
**E-mail institucional / Login:** `lgbm@cesar.school` (`lgbm`)  
**Data:** Novembro de 2026  

---

## 1. Objetivo do Projeto

O objetivo deste projeto é projetar e implementar um mecanismo de sincronização concorrente em linguagem C para o sistema automatizado de vagões e embarque de passageiros da empresa METROREC (baseado no problema clássico da Caltrain).

O sistema modela cada vagão de metrô e cada passageiro como threads concorrentes independentes. A sincronização deve garantir a integridade do embarque de forma estritamente ordenada, respeitando as seguintes restrições:
1. Utilização exclusiva de primitivas POSIX Pthreads (`pthread_mutex_init`, `pthread_mutex_lock`, `pthread_mutex_unlock`, `pthread_cond_init`, `pthread_cond_signal`, `pthread_cond_wait`, `pthread_cond_broadcast`).
2. Proibição estrita do uso de semáforos ou outras primitivas de sincronização.
3. Utilização de **no máximo um único lock (mutex)** por estrutura `struct station`.
4. Garantia de que nunca há mais de um carro na estação por vez e que todos compartilham o mesmo destino.
5. Permitir **embarque simultâneo** (Requisito 5): diversos passageiros podem chamar `station_wait_for_car`, e a função deve retornar para cada um antes de qualquer um deles chamar `station_on_board`.
6. Ausência total de espera ocupada (*busy-wait*).
7. Automatização do build via `Makefile`, gerando o executável `sync_car`.

---

## 2. Modelagem da Estrutura e Primitivas

Para cumprir a restrição de utilizar apenas um lock, a estrutura `struct station` foi modelada com um único mutex para proteger a região crítica e duas variáveis de condição que sincronizam os dois tipos de entidades ativas no sistema:

```c
struct station {
    pthread_mutex_t lock;           /* Único mutex permitido por estação */
    pthread_cond_t train_arrived;   /* Condição para passageiros aguardando vagão com vagas */
    pthread_cond_t train_loaded;    /* Condição para o vagão aguardando embarque completo */
    int waiting_passengers;         /* Quantidade de passageiros aguardando na plataforma */
    int available_seats;            /* Assentos disponíveis no vagão presente */
    int boarding_passengers;        /* Passageiros autorizados a embarcar que ainda não sentaram */
};
```

### Papel das Variáveis de Estado
- **`waiting_passengers`**: Quantifica quantos passageiros já chegaram à estação e estão aguardando um vagão com vagas.
- **`available_seats`**: Representa os assentos vagos ofertados pelo vagão atualmente ancorado na estação.
- **`boarding_passengers`**: Contador crucial para a separação dos momentos de autorização e de conclusão do embarque. Rastreia quantos passageiros receberam assento e retornaram de `station_wait_for_car`, mas ainda não completaram a chamada `station_on_board`.

---

## 3. Implementação das Funções

### 3.1 `station_init`
Inicializa o mutex, as duas variáveis de condição e zera todos os contadores de passageiros e assentos.

### 3.2 `station_load_car`
Chamada quando o carro chega à estação com `count` assentos livres:
```c
void station_load_car(struct station *station, int count) {
    pthread_mutex_lock(&station->lock);
    station->available_seats = count;
    pthread_cond_broadcast(&station->train_arrived);

    while ((station->waiting_passengers > 0 && station->available_seats > 0) ||
           (station->boarding_passengers > 0)) {
        pthread_cond_wait(&station->train_loaded, &station->lock);
    }

    station->available_seats = 0;
    pthread_mutex_unlock(&station->lock);
}
```
1. Adquire o lock da estação e registra as vagas disponíveis.
2. Emite um broadcast (`pthread_cond_broadcast`) acordando todos os passageiros que estavam dormindo na fila da estação.
3. Entra em suspensão atômica com `pthread_cond_wait(&station->train_loaded, &station->lock)`. O vagão só acorda e prossegue quando:
   - Não houver mais assentos livres (`available_seats == 0`) OU não houver mais passageiros esperando (`waiting_passengers == 0`);
   - E **todos** os passageiros autorizados tiverem concluído o embarque (`boarding_passengers == 0`).
4. Ao estar liberado para partir, zera `available_seats` e libera o lock.

### 3.3 `station_wait_for_car`
Chamada quando um passageiro chega à estação:
```c
void station_wait_for_car(struct station *station) {
    pthread_mutex_lock(&station->lock);
    station->waiting_passengers++;

    while (station->available_seats == 0) {
        pthread_cond_wait(&station->train_arrived, &station->lock);
    }

    station->waiting_passengers--;
    station->available_seats--;
    station->boarding_passengers++;

    pthread_mutex_unlock(&station->lock);
}
```
1. Incrementa `waiting_passengers` indicando sua presença na plataforma.
2. Bloqueia em `pthread_cond_wait(&station->train_arrived, &station->lock)` enquanto não houver vagas (`available_seats == 0`). O loop `while` garante proteção contra despertares espúrios.
3. Ao obter vaga, decrementa `waiting_passengers`, decrementa `available_seats` e incrementa `boarding_passengers`.
4. Libera o lock e **retorna imediatamente**, permitindo que o passageiro se desloque fisicamente para embarcar de forma simultânea com os demais.

### 3.4 `station_on_board`
Chamada após o passageiro ocupar fisicamente seu assento:
```c
void station_on_board(struct station *station) {
    pthread_mutex_lock(&station->lock);
    station->boarding_passengers--;

    if (station->boarding_passengers == 0 &&
        (station->available_seats == 0 || station->waiting_passengers == 0)) {
        pthread_cond_signal(&station->train_loaded);
    }

    pthread_mutex_unlock(&station->lock);
}
```
1. Decrementa `boarding_passengers`.
2. Se este for o último passageiro em trânsito de embarque (`boarding_passengers == 0`) e a condição de partida do trem estiver satisfeita (vagão cheio ou fila vazia), envia um sinal (`pthread_cond_signal`) acordando a thread do vagão que espera em `station_load_car`.

---

## 4. Análise de Concorrência e Propriedades de Corretude

| Requisito / Propriedade | Mecanismo de Garantia |
|---|---|
| **Ausência de Busy-Wait** | Toda espera ocorre via `pthread_cond_wait`, liberando a CPU para outras threads. |
| **Embarque Simultâneo** | `station_wait_for_car` libera o mutex antes que o passageiro chame `station_on_board`. Vários passageiros recebem assento e retornam concorrentemente. |
| **Proteção contra Spurious Wakeups** | Todas as esperas em variáveis de condição são envoltas em laços `while`. |
| **Prevenção de Deadlock** | Apenas 1 mutex é utilizado em toda a estrutura, eliminando qualquer dependência cíclica de recursos. |
| **Capacidade e Integridade** | A variável `available_seats` é alterada exclusivamente com o lock retido, garantindo exclusão mútua na alocação de assentos. |

---

## 5. Metodologia de Testes e Resultados

Foi desenvolvida uma suíte abrangente de testes automatizados no arquivo `main.c`, cobrindo 7 cenários distintos:

| ID | Cenário de Teste | Descrição do Teste | Resultado |
|:---:|---|---|:---:|
| **T1** | Capacidade Igual à Demanda | 10 passageiros aguardando, vagão com 10 assentos. | **PASS (10/10)** |
| **T2** | Vagão Maior que a Demanda | 6 passageiros aguardando, vagão com 15 assentos. Vagão parte assim que os 6 sentam. | **PASS (6/6)** |
| **T3** | Vagão Menor que a Demanda | 10 passageiros aguardando, vagão com 4 assentos. 4 embarcam e 6 permanecem na fila. | **PASS (4/4)** |
| **T4** | Múltiplos Vagões Sequenciais | 15 passageiros atendidos por 3 vagões sucessivos (capacidades 4, 5 e 6). | **PASS (15/15)** |
| **T5** | Casos Limite (Estação Vazia) | Carro com 10 assentos chega com 0 passageiros na estação. Não bloqueia. | **PASS (0/0)** |
| **T6** | Embarque Simultâneo (Req. 5) | Barreira controlada: 5 passageiros retornam de `wait_for_car` antes do `on_board`. | **PASS (5/5)** |
| **T7** | Stress Test de Concorrência | 50 passageiros em 4 vagões com atrasos aleatórios e alta concorrência. | **PASS (50/50)** |

**Resultado Geral dos Testes:** 7 de 7 aprovados (Taxa de sucesso: 100%).

---

## 6. Dificuldades Encontradas e Soluções

1. **Separação entre Reserva de Assento e Conclusão de Embarque:**
   - *Desafio:* Permitir que múltiplos passageiros retornassem de `station_wait_for_car` antes que qualquer um chamasse `station_on_board`, sem que o vagão partisse antes de todos estarem sentados.
   - *Solução:* Criação do contador intermediário `boarding_passengers`. O vagão só parte se `boarding_passengers == 0`.
2. **Prevenção de Deadlock quando a Estação se Esvazia:**
   - *Desafio:* Se um vagão com 10 assentos chega para 3 passageiros, o vagão não deve esperar indefinidamente pelos 7 assentos vazios.
   - *Solução:* Condição dupla de término `(available_seats == 0 || waiting_passengers == 0)` aliada a `boarding_passengers == 0`.
3. **Ambiente de Desenvolvimento:**
   - O projeto foi testado e compilado com GCC em ambiente Linux e compatibilizado com MinGW pthreads via Makefile multiplataforma.

---

## 7. Conclusão

A implementação do sistema de sincronização da METROREC atendeu integralmente a todas as exigências das rubricas e restrições estabelecidas pelo professor. O código resultante é conciso, livre de espera ocupada, resiliente a condições de corrida e garante a correta sincronização entre os vagões e os passageiros sob qualquer regime de carga.
