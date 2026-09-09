# Guia Rápido de Início

## Compilação
```bash
make
```

## Executando Testes
```bash
# Escalonamento Rate-Monotonic
./scheduler rate tests/voo.txt
cat rate_mla.out

# Escalonamento Earliest-Deadline-First
./scheduler edf tests/voo.txt
cat edf_mla.out
```

## Entendendo a Saída

O arquivo de saída contém três seções:

1. **RASTREAMENTO DE EXECUÇÃO**: Mostra execução de tarefas e períodos ociosos
   - Cada linha representa 1 unidade de tempo
   - `F` = Tarefa concluída (última unidade de execução)
   - `H` = Tarefa ainda tem trabalho restante

2. **PRAZOS PERDIDOS**: Quantas instâncias perderam seu prazo
   - Tarefa foi preemptada ou não conseguiu ser concluída no tempo
   - Mostra diferença entre algoritmos

3. **EXECUÇÃO COMPLETA**: Instâncias concluídas com sucesso

4. **ELIMINADAS**: Instâncias ainda em execução quando a simulação terminou

## Exemplo de Comparação

Com o arquivo `voo.txt` fornecido:
- **Rate-Monotonic**: NAV perde 1 prazo (tratamento de atraso de prazo)
- **EDF**: NAV é concluída com sucesso (melhor conhecimento de prazo)

Isto demonstra por que EDF pode ser melhor para sistemas críticos em prazos.

## Arquivos Inclusos

- `src/main.c` - Interface de linha de comando
- `src/parser.c` - Análise de arquivo de entrada
- `src/scheduler.c` - Implementações de algoritmo
- `src/task.c` - Gerenciamento de tarefas
- `include/*.h` - Arquivos de cabeçalho
- `tests/voo.txt` - Arquivo de teste principal
- `tests/test1.txt` - Arquivo de teste secundário
- `tests/run_tests.sh` - Script de teste automatizado
- `README.md` - Documentação completa
- `IMPLEMENTATION.md` - Detalhes técnicos

## Próximos Passos

1. Revise o README.md para documentação completa
2. Examine IMPLEMENTATION.md para detalhes do algoritmo
3. Execute `bash tests/run_tests.sh` para testar todos os cenários
4. Modifique arquivos de teste para experimentar com diferentes conjuntos de tarefas
