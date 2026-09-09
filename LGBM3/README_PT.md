# Escalonador de Tarefas - Implementação Rate-Monotonic e EDF

## Visão Geral
Este projeto implementa um simulador de escalonador de tarefas em tempo real que compara dois algoritmos clássicos de escalonamento:
- **Rate-Monotonic (RM)**: Atribui prioridades com base no período da tarefa (período menor = prioridade maior)
- **Earliest-Deadline-First (EDF)**: Atribui prioridades com base no prazo absoluto (prazo anterior = prioridade maior)

Ambos os algoritmos são preemptivos e suportam detecção de prazo.

## Estrutura do Projeto

```
LGBM3/
├── include/              # Arquivos de cabeçalho
│   ├── parser.h         # Parsing de arquivo de entrada
│   ├── scheduler.h      # Interface do escalonador
│   └── task.h           # Definições de tarefa
├── src/                 # Arquivos de origem
│   ├── main.c          # Ponto de entrada do programa principal
│   ├── parser.c        # Implementação do parser de arquivo de entrada
│   ├── scheduler.c     # Implementação dos algoritmos do escalonador
│   └── task.c          # Funções de gerenciamento de tarefas
├── tests/              # Arquivos de teste
│   └── voo.txt        # Arquivo de entrada de exemplo
├── Makefile            # Configuração de compilação
└── README.md           # Este arquivo
```

## Compilação

Para compilar o projeto, simplesmente execute:

```bash
make
```

Isso produzirá um executável chamado `scheduler` no diretório atual.

Para limpar objetos compilados e executáveis:

```bash
make clean
```

## Uso

### Sintaxe Básica
```bash
./scheduler <algoritmo> <arquivo_entrada>
```

### Algoritmos
- `rate`: Escalonamento rate-monotonic
- `edf`: Escalonamento earliest-deadline-first

### Exemplo
```bash
./scheduler rate tests/voo.txt
./scheduler edf tests/voo.txt
```

## Formato do Arquivo de Entrada

O arquivo de entrada deve conter:
- **Linha 1**: Tempo total de simulação (inteiro > 0)
- **Linhas Subsequentes**: Uma tarefa por linha com formato:
  ```
  NOME PERIODO PRAZO RAJADA
  ```

Onde:
- `NOME`: Identificador da tarefa (string)
- `PERIODO` (P): Tempo entre chegadas de tarefas
- `PRAZO` (D): Tempo máximo permitido desde chegada até conclusão
- `RAJADA` (C): Tempo de CPU necessário

**Restrição**: C ≤ D ≤ P (deve ser satisfeita)

### Exemplo (voo.txt)
```
100
ATT 20 12 8
NAV 50 30 15
```

Isso define:
- Simulação por 100 unidades de tempo
- ATT: chega a cada 20 unidades, precisa de 8 unidades de CPU, deve ser concluída em até 12 unidades
- NAV: chega a cada 50 unidades, precisa de 15 unidades de CPU, deve ser concluída em até 30 unidades

## Formato de Saída

O programa gera um arquivo de saída chamado `<algoritmo>_<login>.out` contendo:

1. **Rastreamento de execução**: Mostra cada execução de tarefa ou período ocioso
   - `[NOME_TAREFA] durante X unidades - F`: Tarefa concluída
   - `[NOME_TAREFA] durante X unidades - H`: Tarefa ainda tem trabalho pendente
   - `ocioso durante X unidades`: CPU estava ocioso

2. **Estatísticas**:
   - **PRAZOS PERDIDOS**: Contagem de prazos perdidos por tarefa
   - **EXECUÇÃO COMPLETA**: Contagem de instâncias concluídas com sucesso por tarefa
   - **ELIMINADAS**: Contagem de tarefas incompletas quando a simulação termina

## Tratamento de Erros

O programa valida entrada e relata erros em stderr:
- Número incorreto de argumentos
- Algoritmo inválido (deve ser 'rate' ou 'edf')
- Arquivo de entrada não encontrado ou ilegível
- Entrada malformada (campos faltando, valores não numéricos)
- Restrições inválidas (C > D ou D > P)
- Valores não positivos

## Testes

Um arquivo de teste é fornecido em `tests/voo.txt`. Execute:

```bash
./scheduler rate tests/voo.txt
./scheduler edf tests/voo.txt
```

Em seguida, examine `rate_mla.out` e `edf_mla.out` para ver os resultados.

## Notas de Implementação

- Todas as tarefas chegam no tempo 0
- Preempção é permitida: uma tarefa de maior prioridade que chega interromperá a tarefa atual
- Atrasos de prazo fazem com que a instância atual seja descartada; a próxima instância chega conforme programado
- Desempate: quando as prioridades são iguais, a tarefa que aparece primeiro no arquivo de entrada vence
- O tempo avança em incrementos de 1 unidade

## Requisitos do Sistema

- Compilador GCC (ou compilador C99 compatível)
- Sistema operacional baseado em Linux
- Biblioteca C padrão

## Notas do Autor

Esta implementação segue a especificação do projeto para o curso de Sistemas Operacionais.
A simulação do escalonador suporta ambos os algoritmos preemptivos com rastreamento adequado de prazo e instância.
