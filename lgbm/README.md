# Sistemas-Operacionais-2026.2

# Process Flow

Process Flow é um sistema de execução de tarefas e workflows desenvolvido como projeto da disciplina de Sistemas Operacionais. O objetivo do projeto é simular a criação, organização e execução de processos em um ambiente de linha de comando, permitindo ao usuário registrar tarefas, executá-las de forma sequencial ou paralela, controlar redirecionamentos de entrada e saída, gerenciar diretórios de trabalho e automatizar execuções por meio de arquivos de workflow.

O projeto foi pensado para praticar conceitos fundamentais da disciplina, como:

- criação e controle de processos;
- execução de comandos via `fork` e `exec`;
- manipulação de pipes e redirecionamentos;
- gerenciamento de filas e tarefas em background;
- organização de execução de workflows por arquivo.

---

## Visão geral

O Process Flow funciona como um interpretador de comandos simples, permitindo que o usuário:

- cadastre tarefas;
- execute tarefas individualmente;
- execute várias tarefas em paralelo;
- utilize pipes entre comandos;
- redirecionar entradas e saídas para arquivos;
- alterar o diretório de execução;
- executar workflows em arquivos `.pf`.

A aplicação foi implementada em linguagem C e compila para um binário chamado `processflow`.

---

## Como executar

### 1. Compilar o projeto

Na raiz do projeto, execute:

```bash
make
```

Isso gera o executável:

```bash
./processflow
```

### 2. Executar em modo interativo

Após a compilação, basta executar:

```bash
./processflow
```

A partir daí, o usuário pode digitar comandos no prompt do sistema, como por exemplo:

```bash
run tarefa
start tarefa
jobs
wait 1
exit
```

### 3. Executar um workflow em arquivo

O projeto também aceita a execução automática de um workflow salvo em um arquivo com extensão `.pf`.

Exemplos:

```bash
./processflow teste.pf
./processflow teste_sequencial.pf
./processflow teste_paralelo.pf
./processflow teste_exit.pf
```

Também é possível passar o arquivo via `make`:

```bash
make run WORKFLOW=teste.pf
```

---

## Estrutura de arquivos

A estrutura principal do projeto inclui:

- `Makefile`: responsável pela compilação e execução;
- `processflow`: binário gerado pela compilação;
- `lgbm/`: diretório contendo a implementação do sistema;
- arquivos de teste com extensão `.pf`: exemplos de workflows executáveis.

---

## Makefile

O `Makefile` automatiza a compilação, a execução de workflows e a limpeza do binário.

Comandos principais:

```bash
make
make run WORKFLOW=teste.pf
make clean
```

Conteúdo completo do `Makefile`:

```makefile
CC := gcc
TARGET := processflow
SRC_DIR := ponto\ c
INC_DIR := ponto\ h

SRCS := \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/intertivo.c \
	$(SRC_DIR)/task.c \
	$(SRC_DIR)/catalogo.c \
	$(SRC_DIR)/executor.c \
	$(SRC_DIR)/workflow.c \
	$(SRC_DIR)/comando.c

CFLAGS := -Wall -Wextra -g -O0 -std=c11 \
	-D_POSIX_C_SOURCE=200809L \
	-I$(INC_DIR)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@

run: $(TARGET)
	./$(TARGET) $(WORKFLOW)

clean:
	rm -f $(TARGET)
```

---

## Exemplos de workflows

Os arquivos `.pf` presentes no projeto servem como cenários para testar as funcionalidades do Process Flow. Eles demonstram:

- execução sequencial;
- execução paralela;
- uso de comandos de saída e entrada;
- encerramento da aplicação através de `exit`.

---

## Disciplina

Este projeto foi desenvolvido no contexto da disciplina de Sistemas Operacionais, com foco na aplicação prática de conceitos relacionados a:

- processos;
- sincronização;
- execução de programas;
- controle de entrada e saída;
- gerenciamento de tarefas em ambiente Unix/Linux.

---

## Agradecimentos

Agradecemos à professora e ao ambiente acadêmico que possibilitaram o desenvolvimento deste projeto, bem como a todos que contribuíram com apoio, orientação e revisão ao longo do processo.

---

## Conclusão

O Process Flow representa uma aplicação prática para a compreensão de mecanismos fundamentais de Sistemas Operacionais, permitindo a execução de tarefas em um ambiente controlado e didático. O projeto combina conceitos de linha de comando, gerenciamento de processos e automação de workflows em uma solução simples e funcional.
