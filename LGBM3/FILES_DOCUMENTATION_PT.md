# Documentação de Arquivos de Origem

## Visão Geral
Este documento descreve todos os arquivos .c (e .h) usados no projeto do escalonador.

## Arquivos de Origem (src/)

### main.c
**Propósito**: Ponto de entrada do programa e interface de linha de comando

**Funções**:
- `extract_login()`: Extrai login do usuário (placeholder para implementação real)
- `build_output_filename()`: Constrói nome de arquivo de saída a partir do nome do algoritmo
- `main()`: Processa argumentos, valida entrada, chama escalonador apropriado

**Responsabilidades**:
- Validar argumentos de linha de comando (tipo de algoritmo e arquivo de entrada)
- Garantir especificação correta do algoritmo (rate ou edf)
- Gerar nome de arquivo de saída seguindo convenção de nomenclatura
- Chamar o algoritmo de escalonamento correto

**Dependências**: parser.h, scheduler.h

---

### parser.c
**Propósito**: Análise e validação de arquivo de entrada

**Funções**:
- `print_error()`: Escreve mensagem de erro em stderr
- `parse_input()`: Lê e valida arquivo de entrada
- `free_schedule()`: Libera memória alocada

**Responsabilidades**:
- Ler arquivo de entrada linha por linha
- Extrair tempo total de simulação da primeira linha
- Fazer parsing de definições de tarefas (nome, período, prazo, rajada)
- Validar todas as restrições:
  - C > 0, D > 0, P > 0 (todos positivos)
  - C ≤ D ≤ P (satisfação de restrições)
- Reportar condições de erro específicas para stderr
- Retornar estrutura Schedule analisada ou NULL em erro

**Tratamento de Erros**:
- Arquivo não encontrado/ilegível
- Linhas malformadas (campos faltando)
- Valores não numéricos
- Valores não positivos
- Violações de restrições (C > D ou D > P)

**Dependências**: parser.h

---

### scheduler.c
**Propósito**: Implementa ambos os algoritmos de escalonamento

**Funções Principais**:
- `execute_rate_monotonic()`: Ponto de entrada do escalonador rate-monotonic
- `execute_edf()`: Ponto de entrada do escalonador earliest-deadline-first
- `run_scheduler()`: Loop genérico do escalonador com parâmetro de função de prioridade
- `find_highest_priority_task_rate()`: Seleção de prioridade rate-monotonic
- `find_highest_priority_task_edf()`: Seleção de prioridade EDF

**Funções Auxiliares**:
- `write_execution()`: Registra execução de tarefa no arquivo de saída
- `write_idle()`: Registra período ocioso no arquivo de saída
- `write_stats()`: Escreve estatísticas finais (prazos perdidos, concluídos, eliminados)

**Responsabilidades**:
- Simular comportamento do sistema durante o tempo total de simulação
- Manter estado da tarefa (chegada, prazo, rajada restante)
- Detectar e tratar atrasos de prazo
- Gerenciar preempção e comutação de tarefas
- Gerar rastreamento de execução e estatísticas

**Detalhes do Algoritmo**:
- **Rate-Monotonic**: Prioridade baseada em período (menor = maior)
- **EDF**: Prioridade baseada em prazo absoluto (anterior = maior)
- Desempate: Tarefas que chegam primeiro no arquivo de entrada vencem empates

**Rastreamento**:
- Rastreamento de execução (1 unidade por linha)
- Instâncias com prazo perdido
- Instâncias concluídas com sucesso
- Instâncias eliminadas (incompletas)

**Dependências**: scheduler.h, task.h, parser.h

---

### task.c
**Propósito**: Gerenciamento de estado de tarefa e funções utilitárias

**Funções**:
- `initialize_tasks()`: Cria instâncias de Task a partir de definições TaskDef
- `reset_task_instance()`: Reinicializa tarefa para próxima chegada
- `has_arrived()`: Verifica se instância de tarefa chegou
- `is_ready()`: Verifica se tarefa está pronta para executar (chegou, não completa, não passou prazo)
- `is_past_deadline()`: Verifica se tarefa passou seu prazo

**Responsabilidades**:
- Converter definições estáticas de tarefas em instâncias de tempo de execução dinâmicas
- Gerenciar transições de estado de tarefa
- Rastrear tempo de rajada restante para instância atual
- Calcular prazos absolutos com base em prazos relativos
- Consultar status de disponibilidade de tarefa

**Gerenciamento de Estado**:
- Rastreia informações por instância (remaining_burst, absolute_deadline, next_arrival)
- Gerencia contador de instâncias para estatísticas
- Mantém status de conclusão

**Dependências**: task.h

---

## Arquivos de Cabeçalho (include/)

### parser.h
**Estruturas**:
- `Schedule`: Contém tempo_total, array de tarefas e contagem de tarefas

**Funções** (definidas em parser.c):
- `parse_input()`: Faz parsing de arquivo de entrada
- `free_schedule()`: Libera memória alocada
- `print_error()`: Imprime mensagens de erro

---

### scheduler.h
**Estruturas**:
- `TaskStats`: Rastreia contagens de prazos_perdidos, completo e eliminado por tarefa
- `SchedulerState`: Mantém estado do escalonador durante simulação

**Funções** (definidas em scheduler.c):
- `execute_rate_monotonic()`: Executa algoritmo rate-monotonic
- `execute_edf()`: Executa algoritmo EDF

---

### task.h
**Estruturas**:
- `TaskDef`: Definição estática de tarefa (nome, período, prazo, rajada, id)
- `Task`: Instância de tarefa em tempo de execução com estado

**Funções** (definidas em task.c):
- `initialize_tasks()`: Cria tarefas de tempo de execução
- `reset_task_instance()`: Reseta para próxima chegada
- `has_arrived()`: Verifica status de chegada
- `is_ready()`: Verifica status de disponibilidade
- `is_past_deadline()`: Verifica status de prazo perdido

---

## Compilação

O Makefile automaticamente:
1. Compila cada arquivo .c para arquivo .o
2. Vincula todos os arquivos de objeto em um único executável `scheduler`
3. Inclui flags de compilador apropriadas (-Wall -Wextra -g -std=c99)

**Comando de Compilação**:
```bash
make
```

**Comando de Limpeza**:
```bash
make clean
```

---

## Fluxo de Dados

```
main.c
  ↓
parser.c (lê arquivo) → estrutura Schedule
  ↓
scheduler.c (chama algoritmo apropriado)
  ├→ execute_rate_monotonic()
  └→ execute_edf()
  ↓
task.c (gerencia estado)
  ↓
Arquivo de saída
```

---

## Gerenciamento de Memória

Toda memória alocada dinamicamente é adequadamente liberada:
- `parse_input()` aloca Schedule e array TaskDef
- `initialize_tasks()` aloca array Task
- `free_schedule()` libera todos os dados analisados
- Limpeza do escalonador antes de saída do programa

---

## Testes

Arquivos de teste em `tests/`:
- `voo.txt`: Exemplo da especificação (100 unidades, 2 tarefas)
- `test1.txt`: Caso de teste adicional (50 unidades, 2 tarefas)
- `run_tests.sh`: Executor de testes automatizado

---

## Compatibilidade do Sistema Operacional

- Desenvolvido para sistemas baseados em Linux
- Usa biblioteca C padrão (C99)
- Sem chamadas de sistema específicas de plataforma
- Deve funcionar em qualquer sistema compatível com POSIX com GCC
