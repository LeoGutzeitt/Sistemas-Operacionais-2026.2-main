# Detalhes de Implementação

## Arquitetura

A implementação do escalonador é dividida em quatro componentes principais:

### 1. Parser (parser.c/h)
- Lê e valida arquivos de entrada
- Verifica restrições: C ≤ D ≤ P
- Armazena definições de tarefas
- Trata várias condições de erro

### 2. Gerenciador de Tarefas (task.c/h)
- Gerencia o estado da tarefa durante a simulação
- Rastreia instâncias de tarefas, prazos e chegadas
- Fornece funções auxiliares para consultar o status da tarefa

### 3. Escalonador (scheduler.c/h)
- Implementa ambos os algoritmos RM e EDF
- Trata preempção e detecção de prazo
- Gera rastreamentos de execução e estatísticas

### 4. Principal (main.c)
- Processamento de argumentos de linha de comando
- Coordena a execução geral

## Detalhes do Algoritmo

### Escalonamento Rate-Monotonic
Atribuição de prioridade: `prioridade ∝ 1/período`
- Período menor → prioridade maior
- Prioridades estáticas atribuídas na inicialização
- Preemptivo: tarefas de maior prioridade interrompem as de menor prioridade

Implementação:
```c
find_highest_priority_task_rate() {
    // Encontra tarefa pronta com menor período
    // Desempate pela ordem da tarefa no arquivo de entrada
}
```

### Escalonamento Earliest-Deadline-First
Atribuição de prioridade: Baseada no prazo absoluto
- Prazo anterior → prioridade maior
- Prioridades dinâmicas atualizadas conforme tarefas chegam
- Preemptivo: tarefa com prazo mais próximo interrompe outras

Implementação:
```c
find_highest_priority_task_edf() {
    // Encontra tarefa pronta com prazo absoluto anterior
    // Desempate pela ordem da tarefa no arquivo de entrada
}
```

## Modelo de Execução

### Avanço de Tempo
- O tempo avança em incrementos discretos de 1 unidade
- A cada unidade de tempo:
  1. Verificar chegadas de novas tarefas
  2. Detectar prazos perdidos
  3. Selecionar tarefa pronta com maior prioridade
  4. Executar tarefa selecionada por 1 unidade (ou ficar ocioso)

### Transições de Estado
```
Pronto → Em Execução → Concluído/Preemptado
  ↓                    ↓
Não chegou      Pronto para próxima instância
```

### Tratamento de Atraso de Prazo
Quando o tempo atual atinge o prazo absoluto de uma tarefa:
- A rajada restante é descartada
- A tarefa é contabilizada como prazo perdido
- A tarefa aguarda a chegada de sua próxima instância

## Gerenciamento de Instância de Tarefa

Quando uma instância de tarefa é concluída ou perde o prazo:
```c
reset_task_instance(task, current_time) {
    instance++;
    remaining_burst = original_burst;
    absolute_deadline = current_time + relative_deadline;
    next_arrival = current_time + period;
}
```

## Rastreamento de Estatísticas

Três categorias rastreadas por tarefa:

1. **Prazos Perdidos**: Instâncias que perderam seu prazo
2. **Execução Completa**: Instâncias que terminaram antes do prazo
3. **Eliminadas**: Instâncias ainda em execução quando a simulação termina

## Casos Extremos Tratados

1. **Chegadas simultâneas**: Múltiplas tarefas chegando no tempo 0
2. **Preempção de tarefa**: Tarefa de maior prioridade chega durante execução
3. **Corrida de prazo**: Tarefa é concluída exatamente no prazo (conta como sucesso)
4. **Períodos ociosos**: Quando nenhuma tarefa está pronta para executar
5. **Desempate**: Ordenação consistente pela posição no arquivo de entrada

## Complexidade de Tempo

Para cada unidade de tempo:
- Verificação de chegada: O(n)
- Verificação de prazo: O(n)
- Seleção de prioridade: O(n)
- Total: O(tempo_total × n)

Onde n = número de tarefas
