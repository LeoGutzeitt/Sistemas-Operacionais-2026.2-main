# Relatório da Implementação 5

## Identificação

- Disciplina: Sistemas Operacionais
- Aluno: Leonardo Gutzeit Borgmann Matos
- Login: `lgbm`
- Atividade: Algoritmo do Banqueiro

## 1. Objetivo

Implementei o algoritmo do Banqueiro em C11 para recursos definidos na linha de comando e clientes descritos em `customer.txt`. O programa lê os comandos `RQ`, `RL` e `*` de `commands.txt`, valida pedidos e liberações e grava as respostas em `result.txt`. A quantidade de clientes e de recursos não é fixa.

## 2. Checklist de requisitos

| Requisito do enunciado | Status | Como testar / evidência |
|---|---|---|
| Código principal em C na raiz | OK | `banker.c` na raiz |
| `make` gera `banker` na raiz com C11 e avisos | OK, segundo autocheck informado | `make clean && make`; confirmação registrada em `evidencias.log` |
| Totais de recursos recebidos por argumento | OK, segundo confirmação de testes | `./banker 10 5 7` |
| Clientes lidos de `customer.txt`, uma linha por cliente | OK, segundo confirmação de testes | Exemplo do professor; dados não são versionados |
| Comandos `RQ`, `RL` e `*` lidos de `commands.txt` | OK, segundo confirmação de testes | Exemplo do professor |
| Pedidos respeitam máximo, disponibilidade e estado seguro | OK, segundo confirmação de testes | Exemplo do professor; saída esperada está no enunciado |
| `result.txt` usa as palavras, símbolos e números do formato definido | OK, segundo confirmação de testes | Comparar a saída do exemplo do professor |
| Erros de entrada encerram com mensagem e código diferente de zero | OK, segundo confirmação de testes | O caso `sem_customer_txt` passou com código 1 na primeira rodada |
| `customer.txt`, `commands.txt` e `result.txt` não são versionados | OK | Regras em `.gitignore` |
| `make clean` remove apenas executável e objetos do Banqueiro | OK | Conferir a receita `clean` no Makefile |

O usuário confirmou que a rodada posterior do autocheck passou após a mudança para gravação direta em `result.txt`. A transcrição dessa rodada ainda não foi anexada; os status de execução acima refletem essa confirmação e devem receber a saída detalhada quando disponível.

## 3. Como reproduzir

Na raiz do repositório, com `customer.txt` e `commands.txt` preparados conforme o enunciado:

```sh
make clean
make
./banker 10 5 7
```

O programa lê os dois arquivos no diretório corrente e cria `result.txt`. Para conferir o exemplo, compare as palavras, os símbolos e os números do arquivo com a saída de referência do enunciado. Espaçamento e alinhamento podem variar.

Para conferir a limpeza dos artefatos de compilação:

```sh
make clean
```

O caso de erro com arquivo de clientes ausente deve terminar com código 1, conforme a primeira execução do autocheck registrada em `evidencias.log`.

## 4. Arquitetura resumida

### Arquivos e responsabilidades

1. `banker.c` → valida argumentos e arquivos, mantém o estado e executa `RQ`, `RL` e `*`.
2. `Makefile` → compila o programa na raiz com C11 e define a limpeza dos artefatos do Banqueiro.
3. `README.md` → descreve compilação, execução, arquivos e ambiente.
4. `evidencias.log` → registra tentativas, resultados observados e a origem de cada confirmação.
5. `RELATORIO.md` → texto-fonte deste relatório.
6. `lgbm.pdf` → versão em PDF deste relatório, preparada para revisão.

### Decisões técnicas

1. Vetores e matrizes são alocados dinamicamente → a quantidade de clientes e recursos vem das entradas → o programa não depende de dimensões fixas.
2. `RQ` verifica o máximo antes da disponibilidade → o enunciado exige essa precedência → pedidos que violam os dois limites recebem a mensagem de máximo.
3. O estado é aplicado provisoriamente e revertido quando inseguro → isso implementa a verificação de segurança do Banqueiro → pedidos inseguros não alteram a alocação.
4. A saída é gravada diretamente em `result.txt` → a primeira execução do autocheck terminou com SIGSYS no sandbox e a syscall bloqueada não foi identificada → evita a chamada `rename()`; em caso de erro posterior, o arquivo pode conter saída parcial.

## 5. Estratégias e diário de desenvolvimento

### 5.1 Estratégias

| Estratégia | Nome curto | Contexto | Motivo da troca |
|---|---|---|---|
| S1 | Saída temporária | A saída era escrita em arquivo temporário e publicada por renomeação ao final | O exemplo do autocheck foi encerrado por SIGSYS; a syscall específica não foi informada |
| S2 | Gravação direta | Escrever as respostas diretamente em `result.txt` | Estratégia atual; o usuário informou que a rodada posterior passou |

### 5.2 Diário de tentativas

| # | Estrat. | Quando | O que tentei | Resultado | Hipótese/causa se falhou | Evidência |
|---:|:---:|---|---|---|---|---|
| 1 | S1 | 2026-11-16, ordem registrada no log | Compilar localmente com MinGW e testar o Makefile | Não iniciou | Erro de realocação de DLL do Windows | `evidencias.log`, tentativas locais |
| 2 | S1 | 2026-11-16, primeira rodada remota | Rodar o autocheck, incluindo compilação e ausência de `customer.txt` | Compilação passou; `sem_customer_txt` passou com código 1 | — | `evidencias.log`, primeira transcrição do autocheck |
| 3 | S1 | 2026-11-16, após a etapa anterior | Executar `exemplo_do_professor` | Interrompido com SIGSYS, exit 159 | O sandbox bloqueou uma syscall; o autocheck não identificou qual | `evidencias.log`, primeira transcrição do autocheck |
| 4 | S2 | 2026-11-16, após a falha SIGSYS | Remover a renomeação e gravar diretamente em `result.txt` | O usuário informou que a rodada seguinte passou | A hipótese de que `rename()` acionava a syscall bloqueada não foi confirmada diretamente | `evidencias.log`, ajuste e confirmação do usuário |

O horário exato não foi incluído nas transcrições recebidas. A ordem acima segue a sequência documentada em `evidencias.log`.

## 6. Evidências

`evidencias.log` contém as falhas locais de inicialização do MinGW, a primeira transcrição do autocheck, a interrupção por SIGSYS, a mudança para gravação direta e a confirmação posterior do usuário. A transcrição detalhada da última rodada ainda precisa ser anexada.

Prints solicitados pelo guia:

- Erro principal: **pendente de anexar**. A linha correspondente no `evidencias.log` descreve o SIGSYS da primeira rodada.
- Validação final: **pendente de anexar**. O usuário confirmou a aprovação posterior, mas não foi fornecida captura do autocheck.

## 7. Uso de IA

**Onde usei IA:** usei assistência de IA para estruturar o programa em C, revisar o tratamento dos comandos e erros, diagnosticar a interrupção do sandbox e redigir o README e o relatório.

**Prompts principais:** criar a base do algoritmo do Banqueiro; implementar a leitura de clientes e comandos; investigar a falha SIGSYS; organizar o relatório pelo guia fornecido.

**O que validei manualmente e como:** conferi a estrutura do código e do Makefile, comparei as mensagens implementadas com o enunciado e conferi visualmente o PDF renderizado. A compilação e a aprovação dos testes finais foram informadas pelo autocheck/usuário; a transcrição final ainda precisa ser anexada às evidências.

## 8. Reflexão final

Com mais tempo, eu ampliaria os testes de formato para cobrir arquivos vazios, linhas longas e números nos limites do tipo inteiro. Também repetiria cada cenário em Linux para confirmar o comportamento fora do sandbox do autocheck. A mudança para gravação direta resolveu a interrupção reportada, mas pode deixar saída parcial se houver erro depois de comandos válidos. Eu registraria cada execução com a transcrição completa desde o início. Para a próxima atividade, prepararia os casos de erro e os casos limite antes de começar a implementação. Isso ajudaria a separar falhas do programa de limitações do ambiente mais cedo.

## 9. Checklist final de entrega

| Item | Confirmado? |
|---|---|
| Compilação a partir de `make clean` e `make` | Sim, segundo o autocheck informado |
| Testes executados e registrados em `evidencias.log` | Parcial: primeira transcrição registrada; confirmação final sem transcrição detalhada |
| Prints obrigatórios de erro e validação final | Não anexados ainda |
| Caso limite e caso inválido testados | Sim, segundo confirmação do usuário; `customer.txt` ausente consta na primeira transcrição |
| Seção de uso de IA preenchida | Sim |
| Relatório revisado e sem campos genéricos | Revisar dados pessoais/ reflexão e anexar evidências finais |
| Repositório associado à atividade | Confirmar no Greep/Classroom |

## 10. Se eu tivesse mais 2 horas

1. Repetiria o exemplo completo do professor e guardaria a transcrição integral do autocheck.
2. Executaria casos separados para comando desconhecido, cliente inexistente e vetor com dimensão incorreta.
3. Testaria liberação acima da alocação e pedido acima do máximo enquanto também faltam recursos.
4. Conferiria byte a byte as palavras, símbolos e números da saída de referência.
5. Testaria arquivos vazios, linhas excedendo o limite e valores maiores que `INT_MAX`.
6. Recompilaria após `make clean` em Linux e guardaria a saída sem warnings.
7. Capturaria os prints do erro principal e da validação final para anexar junto ao relatório.
