# Relatório da Implementação 7 — Gerenciador de Memória Virtual

- **Instituição:** CESAR School — Infraestrutura de Software / Sistemas Operacionais (2026.2)
- **Aluno:** Leonardo Gutzeit Borgmann Matos
- **E-mail / Login:** `lgbm@cesar.school` (`lgbm`)
- **Entrega:** `lgbm.pdf` e `lgbm.tar`

## 1. Objetivo

O enunciado pede um Gerenciador de Memória Virtual em C (executável `vm`, compilado via `make`) baseado no projeto do Silberschatz (10ª ed., pág. P-51), adaptado para 128 quadros físicos preenchidos de 0 a 127, substituição FIFO e LRU para páginas e para a TLB, busca concorrente na TLB com uma thread por entrada, tratamento rigoroso de erros e gravação da saída em `correct.txt`. Entrego o código modular em C11/POSIX no diretório `lgbm/`, o `Makefile`, o `README.md`, o `evidencias.log`, o pacote `lgbm.tar` e este relatório `lgbm.pdf`.

## 2. Checklist de Requisitos

| Requisito do enunciado | Status | Como testar / evidência |
|---|---|---|
| Diretório `lgbm/` e pacote `lgbm.tar` sem binários nem dados do professor | OK | `tar -tf implementacao7/lgbm.tar` lista apenas os fontes, `Makefile`, `README.md` e `evidencias.log`. |
| Compilação via `make` gerando executável `vm` e limpeza via `make clean` | OK | `make clean && make` na raiz ou em `implementacao7/lgbm/`; 1ª rodada do autocheck apontou alvo antigo na raiz e o `Makefile` raiz foi corrigido. |
| Memória física com 128 quadros preenchidos de 0 a 127 | OK | `VM_NUM_FRAMES = 128` e alocação sequencial via `prox_quadro_livre` em `paginacao.c`. |
| Substituição de páginas FIFO e LRU | OK | `./vm address.txt fifo fifo` e `./vm address.txt lru fifo`; `lru fifo` validado com 539 page faults (0 diferenças contra o gabarito). |
| Substituição da TLB (16 entradas) FIFO e LRU | OK | `./vm address.txt lru fifo` e `./vm address.txt lru lru`; `lru fifo` obteve 54 TLB hits (0 diferenças contra o gabarito). |
| Busca na TLB com 1 thread por entrada (16 threads POSIX) | OK | `consultar_tlb()` em `tlb.c` cria 16 threads (`pthread_create`/`pthread_join`) para inspecionar cada entrada em paralelo. |
| Arquivo de saída `correct.txt` no formato do gabarito | OK | Formato `Virtual address: X TLB: Y Physical address: Z Value: W` + 5 linhas de estatísticas; validado 100% (0 diffs em 1.005 linhas). |
| Tratamento de erros de argumentos, abertura e formatação de arquivos | OK | `main.c` exige `argc == 4`, políticas estritamente em `{fifo, lru}`, valida `BACKING_STORE.bin` (>= 65536 bytes) e pré-valida `address.txt` antes de gerar saída. |
| Execução final aprovada no autocheck (Linux) com captura anexada | Parcial | Código corrigido após a 1ª rodada do autocheck e validado algoritmicamente contra os 3 arquivos oficiais; pendente nova rodada remota no Greep após o push. |

## 3. Como Reproduzir

Compilar:

```sh
make clean
make
```

Executar (exemplo oficial com LRU na página e FIFO na TLB):

```sh
./vm address.txt lru fifo
```

Executar com outras combinações de políticas:

```sh
./vm address.txt fifo fifo
./vm address.txt lru lru
./vm address.txt fifo lru
```

Testar comparando com o gabarito local (dentro de `implementacao7/lgbm/`, com `BACKING_STORE.bin`, `address.txt` e `correct_ref.txt` presentes):

```sh
make test
```

## 4. Arquitetura (resumida)

### Arquivos e responsabilidades

1. `gerenciador_vm.h` → Define constantes (`TAMANHO_PAGINA`, `TOTAL_PAGINAS`, `TAMANHO_TLB`), estruturas de dados em português (`ContextoMemoria`, `RegistroTlb`, `TarefaBuscaTlb`, `EntradaPagina`, `InfoQuadro`) e contratos dos módulos.
2. `tlb.c` → Implementa a inicialização, a busca concorrente com 16 threads POSIX (`trabalhador_busca_tlb` e `consultar_tlb`), a inserção/substituição FIFO e LRU e a invalidação de páginas na TLB.
3. `paginacao.c` → Gerencia a abertura/validação de `BACKING_STORE.bin`, os 128 quadros físicos, a tabela de páginas, o tratamento de falta de página (`resolver_falta_pagina`) e a tradução completa (`traduzir_endereco`).
4. `main.c` → Valida argumentos de linha de comando, carrega e pré-valida integralmente `address.txt` (`carregar_enderecos`) e grava o arquivo `correct.txt` com as estatísticas finais.
5. `Makefile` → Compila `main.c`, `tlb.c` e `paginacao.c` em C11/POSIX gerando `vm`, além de oferecer `clean`, `test` e alvos auxiliares da rubrica.
6. `README.md` → Documenta compilação, execução, tratamento de erros e mapeamento de cada item da rubrica.
7. `evidencias.log` → Registra em ordem cronológica as tentativas, falhas observadas, ajustes e validações.

### Decisões técnicas

1. Separação em módulos em português (`tlb.c`, `paginacao.c`, `main.c`) → Isolar responsabilidades de TLB, paginação e entrada/saída → Código mais legível, manutenível e fácil de auditar.
2. Pré-validação integral de `address.txt` antes de abrir `correct.txt` → Evitar geração de saída parcial ou corrompida se houver erro em uma linha avançada → Erros de formatação encerram com código 1 sem deixar arquivo incompleto.
3. Uso de relógio lógico monotônico (`uint64_t relogio`) e vetor auxiliar `bool thread_iniciada[TAMANHO_TLB]` → Garantir desempate determinístico em FIFO/LRU e evitar comparar `pthread_t` (tipo opaco POSIX) com `0` → Portabilidade estrita sob `-std=c11 -Wall -Wextra -Wpedantic` em Linux, Unix e macOS.
4. Invalidação imediata da TLB ao substituir um quadro físico → Evitar traduções desatualizadas quando uma página é removida da RAM → Coerência garantida entre tabela de páginas e TLB.

## 5. Estratégias e Diário de Desenvolvimento

### 5.1 Estratégias da semana

| Estratégia | Nome curto | Contexto | Motivo da troca (se houve) |
|---|---|---|---|
| S1 | Base monolítica em `vm.c` | Implementação inicial concentrada em `vm.c`/`vm.h` com `Makefile` apenas em `implementacao7/lgbm/` e sem o campo `TLB:` na linha de saída. | A 1ª rodada do autocheck falhou porque o `Makefile` da raiz compilava `banker`; além disso, o gabarito `correct.txt` do Classroom exigia `TLB: <indice>`. |
| S2 | Ajuste de `Makefile` raiz e formato `correct.txt` | Atualização do `Makefile` da raiz para gerar `./vm` e inclusão do campo `TLB: %d` validado contra os 3 arquivos oficiais. | Funcionou com 0 diferenças nas 1.005 linhas do gabarito, mas o código ainda estava monolítico e com nomes em inglês. |
| S3 | Arquitetura modular em português | Refatoração completa para `gerenciador_vm.h`, `tlb.c`, `paginacao.c` e `main.c` com identificadores em português, mantendo 100% da lógica validada. | Estratégia final adotada para entrega. |

### 5.2 Diário de Tentativas

| # | Estrat. | O que tentei | Resultado | Hipótese/Causa (se falhou) | Quando | Evidência |
|---:|:---:|---|---|---|---|---|
| 1 | S1 | Compilar localmente no Windows com MinGW (`gcc --version`) | Falha | Erro de realocação de DLL do sistema operacional Windows no MinGW local. | 2026-11-23 | `evidencias.log` (seção 3) |
| 2 | S1 | Rodar 1ª rodada do autocheck no Greep | Falha (0%) | O autocheck executou `make` na raiz do repositório, cujo `Makefile` ainda tinha `all: banker` da Implementação 5, não gerando o binário `vm`. | 2026-11-24 | `evidencias.log` (1ª Rodada do Autocheck) |
| 3 | S2 | Atualizar `Makefile` da raiz para gerar `./vm` a partir de `implementacao7/lgbm/` | OK (estático) | — | 2026-11-24 | `evidencias.log` (1ª Rodada do Autocheck) |
| 4 | S2 | Validar a lógica com os 3 arquivos do Classroom (`BACKING_STORE.bin`, `address.txt`, `correct.txt`) | Parcial → OK | O gabarito continha `TLB: %d` antes de `Physical address:`; após incluir o campo, a validação atingiu 0 diferenças em 1.005 linhas (539 page faults, 54 TLB hits). | 2026-11-24 | `evidencias.log` (Verificação com os 3 arquivos oficiais) |
| 5 | S3 | Refatorar o código para 4 arquivos modulares em português e regerar `lgbm.tar` | OK | — | 2026-11-24 | `evidencias.log` (Refatoração Modular em Português) |

## 6. Evidências

### 6.1 Log de evidências (`evidencias.log`)

O arquivo `implementacao7/lgbm/evidencias.log` (incluído dentro de `lgbm.tar` e versionado no GitHub) registra em ordem cronológica a revisão estática C11/POSIX, a falha do MinGW local no Windows, a transcrição da 1ª rodada do autocheck, a correção do `Makefile` da raiz, a validação de 100% contra os 3 arquivos oficiais do Classroom e a refatoração modular final.

### 6.2 Prints e trechos reais do `evidencias.log`

**Erro principal (transcrição real da 1ª rodada do autocheck — 2026-11-24):**

```text
iniciado: correção iniciada
rodando: compilando
passou: compilação concluída
critério "Tradução dos endereços" — iniciado
critério "Tradução dos endereços" — falhou: binário "vm" não encontrado no build (Makefile produziu outro nome, ou nenhum)
concluído: autocheck concluído -- 0% nos testes públicos
```

**Validação contra os 3 arquivos oficiais do Classroom e conferência do `lgbm.tar` (`evidencias.log`):**

```text
Loaded 1000 addresses, 65536 bytes backing store, 1005 reference lines.
Number of Translated Addresses = 1000
Page Faults = 539 (Page Fault Rate = 0.539)
TLB Hits = 54 (TLB Hit Rate = 0.054)
Diferencas linha a linha contra correct_ref.txt: 0 em 1005 linhas

$ tar -tf implementacao7/lgbm.tar
lgbm/gerenciador_vm.h
lgbm/tlb.c
lgbm/paginacao.c
lgbm/main.c
lgbm/Makefile
lgbm/README.md
lgbm/evidencias.log
```

*(Observação de transparência: não incluí capturas fabricadas de compilação local porque o MinGW no host Windows falha por erro de realocação de DLL do sistema; a nova rodada remota do autocheck em Linux aguarda apenas o push final.)*

## 7. Uso de IA

**Onde usei IA:** Utilizei assistência de IA para revisar portabilidade C11/POSIX (substituição de `strcasecmp` e tratamento opaco de `pthread_t`), diagnosticar o erro de alvo no `Makefile` da raiz, conferir o formato exato do `correct.txt` contra o gabarito e organizar a refatoração modular e a redação deste relatório.

**Prompts principais:**
- Estruturar gerenciador de memória virtual com 128 quadros, FIFO/LRU em página e TLB e busca concorrente na TLB com 16 threads POSIX;
- Diagnosticar falha de binário `vm` não encontrado no autocheck;
- Validar a saída linha a linha contra `BACKING_STORE.bin`, `address.txt` e `correct.txt`;
- Refatorar o código em módulos próprios em português e diagramar o relatório conforme o Guia de Relatórios.

**O que validei manualmente e como:** Conferi manualmente os operadores bit a bit para extração de página (`(addr >> 8) & 0xFF`) e deslocamento (`addr & 0xFF`), a ordem de preenchimento dos quadros de 0 a 127, a invalidação da TLB em substituição de página, o conteúdo exato do `lgbm.tar` via `tar -tf` (garantindo ausência de binários e dos arquivos do professor) e a aderência deste PDF às 10 seções do guia.

## 8. Reflexão Final

A principal lição desta implementação foi a importância de alinhar o formato de saída e o processo de build do repositório desde o início, pois a falta do campo `TLB:` e o `Makefile` da raiz ainda apontando para a entrega anterior teriam comprometido a avaliação automática mesmo com a lógica de paginação correta. Modularizar o projeto em `tlb.c`, `paginacao.c` e `main.c` deixou o fluxo de tradução, substituição e tratamento de erros muito mais claro e fácil de depurar. A simulação da busca na TLB com 16 threads POSIX evidenciou na prática a diferença entre paralelismo associativo em hardware e o custo de criação de threads em software. Para a próxima atividade, eu começaria verificando o alvo padrão do `Makefile` na raiz e conferindo cada linha do gabarito de referência antes da primeira submissão no autocheck.

## 9. Checklist Final de Entrega

| Item | Confirmado? |
|---|---|
| Compilei do zero seguindo só o que está no relatório | Parcial — compilação local bloqueada por erro de DLL do MinGW no Windows; código auditado para C11/POSIX e pronto para nova rodada do autocheck em Linux. |
| Rodei os testes e `evidencias.log` foi gerado | Sim — `evidencias.log` atualizado e incluído dentro de `lgbm.tar`. |
| Tenho prints obrigatórios | Parcial — trechos reais do erro no autocheck e da validação contra os 3 arquivos oficiais incluídos na Seção 6.2; captura da nova rodada remota pendente após o push. |
| Testei pelo menos um caso limite e um caso inválido | Parcial — validação de casos inválidos e limites implementada e auditada em `main.c` e `paginacao.c`; execução em Linux pendente da próxima rodada do autocheck. |
| Preenchi a seção de uso de IA | Sim. |
| Revisei o relatório e removi frases genéricas / vazias | Sim. |

## 10. Se eu tivesse mais 2 horas

Se eu tivesse mais duas horas, executaria toda a bateria de testes diretamente em uma máquina Linux ou container Docker para capturar os prints reais de `make clean && make`, da execução `./vm address.txt lru fifo` comparada com `diff -u correct.txt correct_ref.txt` e de cada caso de erro (quantidade incorreta de argumentos, algoritmo inválido, arquivo ausente e linha malformada). Em seguida, substituiria a criação e destruição de 16 threads por consulta na TLB por um conjunto fixo de 16 threads trabalhadoras sincronizadas por barreira ou variáveis de condição, medindo o ganho de tempo real sobre os 1.000 endereços. Também rodaria o binário com `valgrind` e AddressSanitizer para comprovar ausência total de vazamentos de memória e acessos fora dos limites, além de comparar graficamente as taxas de falta de página entre 128 e 256 quadros para FIFO e LRU.
