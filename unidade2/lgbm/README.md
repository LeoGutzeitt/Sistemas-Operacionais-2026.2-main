# METROREC - Sincronização de Vagões e Embarque de Passageiros (LGBM)

> Atualização de 04/11/2026: `main.c` foi substituído pelo teste fornecido
> `sync_car-runner.c`. O Makefile compila esse runner. Execute `make test`
> no Linux ou `mingw32-make test` no Windows com MinGW. O runner não oferece
> os sete cenários nomeados nem o modo CLI descritos na documentação anterior
> abaixo; ele testa 100 passageiros com capacidades aleatórias, vagões sem
> vagas, estação vazia e espera pela confirmação de embarque.
> Consulte `evidencias.log` para os resultados efetivamente executados.
> No Windows, o runner desativa `alarm()`, portanto use um timeout externo
> para detectar travamentos. Os pacotes e relatórios anteriores a esta
> atualização ainda precisam ser regenerados antes da entrega.

**Disciplina:** Sistemas Operacionais (2026.2)  
**Aluno:** Leonardo Gutzeit Borgmann Matos  
**E-mail / Login:** `lgbm@cesar.school` (`lgbm`)  
**Identificador da Entrega:** `lgbm`

---

## 1. Descrição do Projeto

Este projeto implementa o mecanismo de sincronização de threads para o sistema automatizado de vagões e embarque de passageiros do METROREC (baseado no problema clássico de sincronização da Caltrain).

Cada vagão e cada passageiro são gerenciados por threads concorrentes independentes. A sincronização garante:
1. **Embarque ordenado e satisfatório:** O vagão (`station_load_car`) só retorna quando todos os passageiros autorizados estiverem devidamente sentados (`station_on_board`), e ou o vagão está completamente lotado ou todos os passageiros que aguardavam na estação já embarcaram.
2. **Capacidade respeitada:** Nenhum passageiro embarca em um vagão sem vaga.
3. **Embarque simultâneo (Requisito 5):** Permite que múltiplos passageiros chamem e retornem de `station_wait_for_car` concorrentemente antes de qualquer um deles chamar `station_on_board`.
4. **Ausência de Espera Ocupada (Busy-Wait):** Todas as esperas utilizam variáveis de condição POSIX (`pthread_cond_wait`), com liberação atômica do mutex.
5. **Restrição de Lock:** Apenas **1 único lock** (`pthread_mutex_t lock`) é utilizado na estrutura `struct station`. Não são utilizados semáforos nem outras primitivas de sincronização.

---

## 2. Estrutura dos Arquivos

- **`sync_car.h`**: Declaração da estrutura `struct station` e protótipos das 4 funções da API de sincronização:
  - `void station_init(struct station *station);`
  - `void station_load_car(struct station *station, int count);`
  - `void station_wait_for_car(struct station *station);`
  - `void station_on_board(struct station *station);`
- **`sync_car.c`**: Implementação das funções de sincronização com mutex e variáveis de condição (`pthread_cond_broadcast`, `pthread_cond_signal`, `pthread_cond_wait`).
- **`main.c`**: Programa principal de testes e simulação com bateria automatizada de 7 testes de validação das rubricas e modo CLI interativo.
- **`Makefile`**: Script de automação para compilação (`make`), execução de testes (`make test`), limpeza (`make clean`) e geração do pacote (`make tar`).
- **`README.md`**: Este documento descritivo.

---

## 3. Ambiente e Sistema Operacional

- **Sistemas Suportados:** Linux (Ubuntu 22.04 LTS / Debian), Unix, macOS e Windows (MinGW-w64 / MSYS2 / WSL).
- **Compilador:** GCC com padrão C99 (`-std=c99`) ou C11.
- **Bibliotecas:** POSIX Threads (`pthread`).

---

## 4. Instruções de Compilação e Execução

### 4.1 Compilação

Para compilar o projeto gerando o executável `sync_car`:

```bash
make
```

### 4.2 Execução da Bateria de Testes Automatizada

Para executar todos os 7 testes automatizados de validação:

```bash
make test
```
ou diretamente:
```bash
./sync_car
```

### 4.3 Execução em Modo Customizado

Você pode rodar cenários personalizados passando o número total de passageiros e a capacidade de assentos de cada vagão:

```bash
./sync_car <numero_passageiros> <capacidade_por_vagao>
```

Exemplo (20 passageiros e vagões com capacidade para 6 pessoas):
```bash
./sync_car 20 6
```

### 4.4 Limpeza dos Arquivos Compilados

Para remover arquivos objeto (`.o`), executáveis e arquivos temporários:

```bash
make clean
```

### 4.5 Geração do Arquivo de Entrega (`lgbm.tar`)

Para gerar o arquivo compactado `lgbm.tar` contendo o diretório `lgbm/`:

```bash
make tar
```

---

## 5. Resumo da Bateria de Testes Automatizados

O executável executa 7 cenários que cobrem todas as exigências das rubricas:

1. **Capacidade Igual:** Vagão com capacidade exatamente igual à quantidade de passageiros aguardando (10 assentos, 10 passageiros).
2. **Vagão Maior que a Demanda:** Vagão chega com mais vagas do que passageiros (15 assentos, 6 passageiros). O vagão parte assim que os 6 sentam, sem aguardar infinitamente.
3. **Vagão Menor que a Demanda:** Vagão chega com menos vagas do que passageiros (4 assentos, 10 passageiros). 4 passageiros embarcam e os outros 6 permanecem na estação para o próximo vagão.
4. **Múltiplos Vagões Consecutivos:** Atendimento de 15 passageiros através de 3 vagões sequenciais (capacidades 4, 5 e 6).
5. **Casos Limite:** Vagão chega à estação sem nenhum passageiro aguardando (0 passageiros, 10 assentos). Parte imediatamente sem deadlock.
6. **Embarque Simultâneo (Requisito 5):** Validação estrita de que múltiplos passageiros retornam de `station_wait_for_car` antes que qualquer um deles execute `station_on_board`.
7. **Stress Test:** Simulação concorrente de 50 passageiros e 4 vagões sob rajadas de chegadas com atrasos variados.
