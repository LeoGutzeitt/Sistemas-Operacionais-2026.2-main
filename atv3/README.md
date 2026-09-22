# Escalonador de CPU em Tempo Real

Simulador em C para comparar os algoritmos Rate-Monotonic (`rate`) e
Earliest-Deadline-First (`edf`) em tarefas periódicas e preemptivas.

## Estrutura

- `src/main.c`: valida os argumentos e inicia a execução.
- `src/parser.c`: lê e valida o arquivo de entrada.
- `src/task.c`: controla chegadas, instâncias, rajadas e deadlines.
- `src/scheduler.c`: implementa Rate-Monotonic, EDF, preempção e estatísticas.
- `include/`: declara as estruturas e funções do projeto.
- `tests/`: contém entradas de teste e o script automatizado.
- `Makefile`: compila e limpa o projeto.

## Compilação

Execute os comandos a partir deste diretório:

```bash
make clean
make
```

O resultado é o executável `scheduler`. O projeto foi implementado e testado
em Linux com GCC e C99.

## Makefile

```
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
SRCDIR = src
INCDIR = include
BINDIR = .

SOURCES = $(SRCDIR)/main.c $(SRCDIR)/parser.c $(SRCDIR)/scheduler.c $(SRCDIR)/task.c
HEADERS = $(INCDIR)/parser.h $(INCDIR)/scheduler.h $(INCDIR)/task.h
OBJECTS = $(SOURCES:.c=.o)
TARGET = $(BINDIR)/scheduler

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f *_*.out

.PHONY: all clean

```

## Execução

```bash
./scheduler rate tests/voo.txt
./scheduler edf tests/voo.txt
```

Os arquivos gerados são `rate_mla.out` e `edf_mla.out`. O login utilizado nesta
versão é `mla`.

## Formato da entrada

```text
[TEMPO_TOTAL]
[NOME] [PERIODO] [DEADLINE] [BURST]
```

Todas as tarefas chegam inicialmente no instante 0. Os valores devem ser
inteiros positivos e satisfazer `C <= D <= P`.

Exemplo:

```text
100
ATT 20 12 8
NAV 50 30 15
```

## Algoritmos

- **Rate-Monotonic:** a tarefa com menor período possui maior prioridade fixa.
- **EDF:** a tarefa com o deadline absoluto mais próximo possui maior prioridade.
- Ambos são preemptivos. Em caso de empate, vence a tarefa que aparece primeiro
  no arquivo de entrada.
- Uma instância que chega ao deadline sem terminar perde o restante da rajada;
  ela não é concluída atrasada e só volta na próxima chegada.

## Saída

O arquivo de saída registra a execução em unidades de tempo, períodos ociosos e
as estatísticas de cada tarefa:

- `PRAZOS PERDIDOS`: instâncias que não terminaram até o deadline.
- `EXECUÇÃO COMPLETA`: instâncias concluídas dentro do prazo.
- `ELIMINADAS`: instâncias ainda incompletas quando a simulação termina.

As mensagens desta implementação estão em português, por exemplo
`EXECUÇÃO COM RATE`, `ocioso`, `PRAZOS PERDIDOS`, `EXECUÇÃO COMPLETA` e
`ELIMINADAS`.

## Tratamento de erros

O programa escreve mensagens em `stderr`, retorna código diferente de zero e
não cria uma nova saída quando encontra:

- número incorreto de argumentos;
- algoritmo diferente de `rate` ou `edf`;
- arquivo inexistente ou ilegível;
- entrada malformada ou valor não numérico;
- valor não positivo;
- violação de `C <= D <= P`.

## Testes

Execute todos os testes com:

```bash
bash tests/run_tests.sh
```

O script compila o programa, executa Rate-Monotonic e EDF, testa uma segunda
entrada e verifica algoritmo inválido e arquivo inexistente. Também é possível
executar manualmente os algoritmos e analisar `rate_mla.out` e `edf_mla.out`.

## Limpeza

```bash
make clean
```

Esse comando remove os arquivos objeto, o executável e os arquivos de saída.