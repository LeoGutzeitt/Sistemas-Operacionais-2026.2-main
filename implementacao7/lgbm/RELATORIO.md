# Relatório da Implementação 7: Gerenciador de Memória Virtual

## Identificação

- Disciplina: Sistemas Operacionais (2026.2)
- Aluno: Leonardo Gutzeit Borgmann Matos
- Login / Identificador: `lgbm` (`lgbm@cesar.school`)
- Atividade: Gerenciador de Memória Virtual (Virtual Memory Manager)

## 1. Objetivo

Implementei um Gerenciador de Memória Virtual em C11 com base no projeto clássico do Silberschatz (*Operating System Concepts*, 10ª edição), adaptado com requisitos de restrição física a 128 frames, suporte a múltiplas políticas de substituição de páginas e de TLB (`fifo` e `lru`), busca paralela na TLB utilizando 16 threads POSIX (`pthread`) concorrentes e geração do arquivo de saída `correct.txt`. O programa realiza a tradução de endereços virtuais de 16 bits para endereços físicos, lendo blocos de 256 bytes sob demanda do arquivo `BACKING_STORE.bin`.

## 2. Checklist de requisitos e Rubricas

| Requisito do enunciado / Rubrica | Peso | Status | Como testar / Evidência |
|---|:---:|:---:|---|
| Diretório `lgbm/` e pacote `lgbm.tar` | — | OK | `lgbm/` criado; compactação via Makefile / `package_tar.py` |
| `make` gera executável `vm` com C11/POSIX | 10% | OK | `make` e `make clean` no diretório `lgbm/` |
| README técnico completo | 10% | OK | `implementacao7/lgbm/README.md` |
| Relatório Técnico (`lgbm.pdf`) | 20% | OK | `RELATORIO.md` estruturado nas 10 seções do Guia |
| Tratamento de Erro | 10% | OK | Verificação de argumentos, existência e integridade de arquivos |
| Tradução dos endereços lógicos | 5% | OK | Máscara de página (`>> 8`), deslocamento (`& 0xFF`) e composição física |
| Fluxo sem TLB com 256 frames | 10% | OK | `make vm-256-notlb && ./vm address.txt fifo fifo` |
| Fluxo sem TLB com 128 frames substituindo com FIFO | 10% | OK | `make vm-128-notlb && ./vm address.txt fifo fifo` |
| Fluxo sem TLB com 128 frames substituindo com LRU | 15% | OK | `make vm-128-notlb && ./vm address.txt lru fifo` |
| Fluxo com TLB sem thread substituindo com FIFO | 5% | OK | `make vm-128-nothreads && ./vm address.txt lru fifo` |
| Fluxo com TLB sem thread substituindo com LRU | 5% | OK | `make vm-128-nothreads && ./vm address.txt lru lru` |
| Fluxo da TLB com thread (16 threads paralelas) | 10% | OK | `make && ./vm address.txt lru fifo` (modo padrão) |

## 3. Como reproduzir

No diretório `implementacao7/lgbm/`:

```bash
make clean
make
./vm address.txt lru fifo
```

O programa lerá os endereços lógicos linha a linha de `address.txt`, consultará a TLB em paralelo através de 16 threads, buscará na tabela de páginas ou no arquivo binário `BACKING_STORE.bin` e salvará o resultado traduzido em `correct.txt`.

Para limpar os artefatos compilados:
```bash
make clean
```

Para validar cenários específicos da rubrica:
```bash
# 128 frames, sem TLB, política FIFO
make vm-128-notlb && ./vm address.txt fifo fifo

# 128 frames, sem TLB, política LRU
make vm-128-notlb && ./vm address.txt lru fifo

# 128 frames, TLB com LRU e busca sequencial (sem threads)
make vm-128-nothreads && ./vm address.txt lru lru

# Retornar ao binário padrão oficial (128 frames, TLB com 16 threads)
make clean && make && ./vm address.txt lru fifo
```

## 4. Arquitetura resumida

### Arquivos e responsabilidades

1. `vm.h` → Definições de constantes, estruturas de dados da TLB, tabela de páginas e metadados de frames.
2. `vm.c` → Implementação do ciclo de tradução, busca concorrente com pthreads, substituição FIFO/LRU e interface CLI.
3. `Makefile` → Automação de compilação com `-Wall -Wextra -Wpedantic -std=c11 -pthread` e limpeza.
4. `README.md` → Documentação técnica de uso, parâmetros e mapeamento de rubricas.
5. `evidencias.log` → Registro de desenvolvimento, parâmetros aceitos e notas de validação.
6. `correct.txt` → Saída produzida contendo `Virtual address: %d Physical address: %d Value: %d`.

### Decisões técnicas

1. **Preenchimento Sequencial de Frames:** Conforme a especificação, os frames físicos são preenchidos ordenadamente de 0 a 127. Somente após a ocupação total dos 128 frames a substituição entra em ação.
2. **Busca Paralela na TLB:** Cada uma das 16 entradas da TLB é associada a uma thread dedicada (`pthread_t`), disparada concorrentemente para simular hardware associativo. A primeira thread a registrar correspondência sinaliza o hit.
3. **Invalidação Consistente:** Sempre que uma página vítima é selecionada para descarte da memória física, a função `tlb_invalidate_page()` varre a TLB e remove qualquer entrada referenciando a página descartada, impedindo leituras espúrias.
4. **Relógio Lógico Global:** A ordenação temporal de carga e acesso para FIFO e LRU utiliza um contador monotônico incremental de 64 bits (`uint64_t global_clock`), evitando problemas de precisão de relógios de tempo real.

## 5. Estratégias e diário de desenvolvimento

### 5.1 Estratégias

| Estratégia | Nome curto | Contexto | Motivo da troca / Adoção |
|---|---|---|---|
| S1 | TLB Sequencial Simples | Consulta linear tradicional de 0 a 15 em array | Atendia ao livro do Silberschatz, mas não atendia ao requisito explícito de simulação concorrente com threads. |
| S2 | TLB Multithread (16 threads) | Disparo de 1 thread por entrada com argumento dedicado | Adotada para atender plenamente a especificação e ao item de 10% da rubrica. |

### 5.2 Diário de tentativas

| # | Estrat. | Quando | O que tentei | Resultado | Hipótese/causa se falhou | Evidência |
|---:|:---:|---|---|---|---|---|
| 1 | S1 | 2026-11-23, início | Estruturação inicial das structs de memória e parsing | Sucesso conceitual | — | `vm.h` |
| 2 | S2 | 2026-11-23, implementação | Criação das 16 threads com `pthread_create` e `pthread_join` | Sucesso na arquitetura | — | `vm.c` (`tlb_lookup`) |
| 3 | S2 | 2026-11-23, empacotamento | Criação do tar com estrutura exigida `lgbm/` | Arquivo `lgbm.tar` gerado com sucesso | — | `package_tar.py` |

## 6. Evidências

O arquivo `evidencias.log` contém o registro da arquitetura desenvolvida, os detalhes dos algoritmos de substituição e os comandos para reprodução de cada item da rubrica.

Prints solicitados pelo guia:
- Erro principal: **pendente de anexar** (captura da execução de teste com arquivo inexistente ou argumento inválido).
- Validação final: **pendente de anexar** (captura da execução com `address.txt lru fifo` exibindo as estatísticas no terminal e o arquivo `correct.txt`).

## 7. Uso de IA

**Onde usei IA:** Auxílio no planejamento da arquitetura multithread para consulta concorrente na TLB, elaboração dos esquemas de invalidação de página na substituição e estruturação da documentação e Makefile.

**Prompts principais:** projetar gerenciador de memória virtual Silberschatz com 128 frames e busca concorrente na TLB; implementar políticas FIFO e LRU; criar Makefile e documentação conforme rubrica.

**O que validei manualmente e como:** conferência manual de bitwise operators para extração de página (`(addr >> 8) & 0xFF`) e offset (`addr & 0xFF`), cálculo de endereço físico, regras de substituição de frames e conferência do padrão de saída `correct.txt`.

## 8. Reflexão final

A transição de uma tabela de páginas com 256 frames (onde todas as páginas cabem na memória física) para um ambiente com 128 frames evidencia na prática a importância de políticas eficientes de substituição. A política LRU tende a apresentar taxas de page fault significativamente menores que FIFO sob padrões de localidade temporal. Adicionalmente, a implementação de concorrência com uma thread por entrada da TLB demonstra o custo de sincronização em software quando comparado à busca associativa direta em hardware, enriquecendo a compreensão dos compromissos entre paralelismo e sobrecarga computacional.

## 9. Checklist final de entrega

| Item | Confirmado? |
|---|---|
| Compilação a partir de `make clean` e `make` | Sim, Makefile configurado com flags rigorosas |
| Testes executados e registrados em `evidencias.log` | Sim |
| Prints obrigatórios de erro e validação final | Pendente de anexar após execução do autocheck |
| Caso limite e caso inválido testados | Sim, validação estrita de argumentos e linhas |
| Seção de uso de IA preenchida | Sim |
| Relatório revisado e sem campos genéricos | Sim |
| Arquivos compactados no padrão `lgbm.tar` | Sim, arquivo gerado e pronto |

## 10. Se eu tivesse mais 2 horas

Se eu tivesse mais duas horas, implementaria um thread pool fixo ou mecanismo de sincronização via barreiras com variáveis de condição para a TLB, evitando a criação e destruição repetida de 16 pthreads a cada acesso de memória e medindo o impacto no tempo total de execução. Também criaria um script de benchmark automatizado comparando as curvas de page faults e TLB hit rates para diferentes tamanhos de memória física (de 32 a 256 frames) e diferentes distribuições de acesso à memória, gerando gráficos comparativos de desempenho entre FIFO e LRU.
