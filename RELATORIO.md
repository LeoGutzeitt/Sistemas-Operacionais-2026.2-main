# Relatório — Implementação 5: Algoritmo do Banqueiro

> Relatório-base. A compilação e o caso de erro sem `customer.txt` passaram no autocheck informado pelo usuário. A execução do exemplo principal foi interrompida pelo sandbox antes de produzir uma avaliação do algoritmo; este resultado ainda precisa ser repetido em ambiente funcional.

## Identificação

- Disciplina: Sistemas Operacionais
- Aluno: Leonardo Gutzeit Borgmann Matos
- Login: `lgbm`
- Atividade: Implementação 5

## Objetivo

Implementar o algoritmo do Banqueiro com o número de tipos de recursos definido pelos argumentos, clientes descritos em `customer.txt`, comandos em `commands.txt` e resultados em `result.txt`.

## Organização e estruturas

O código principal está em `banker.c`, na raiz, e é compilado pelo `Makefile` para `./banker`. A Implementação 4 foi mantida em `unidade2/lgbm/` e o rascunho inicial desta atividade foi preservado separadamente em `unidade2/implementacao5-base-anterior/`. O estado do Banqueiro contém o total e a disponibilidade por recurso e as matrizes `maximum`, `allocation` e `need` por cliente e recurso. A quantidade de clientes é determinada pelas linhas de `customer.txt`.

## Implementação

O programa valida que cada argumento seja uma quantidade positiva representável como inteiro. Em seguida, lê cada linha de `customer.txt` como vetor CSV de máximos e inicializa a alocação em zero e a necessidade igual ao máximo. Os comandos são tokenizados e validados quanto ao nome, cliente e número de valores.

Para `RQ`, o programa verifica primeiro se o pedido excede a necessidade máxima do cliente, depois se há recursos disponíveis. Se passar, aplica a alocação provisoriamente e simula o algoritmo de segurança usando cópias de `available` como vetor `work`. Se não houver sequência segura, a alteração é revertida. `RL` é rejeitado como erro se tentar liberar mais do que o cliente possui. `*` imprime as matrizes e o vetor disponível no formato exigido.

As respostas são escritas diretamente em `result.txt`. Se um erro de formato ou uma liberação inválida for encontrado durante o processamento, o programa informa o erro e encerra; o arquivo pode conter as respostas dos comandos processados anteriormente.

## Testes realizados

Na versão enviada ao autocheck, a compilação foi concluída e o caso `sem_customer_txt` passou com código de saída 1. No caso `exemplo_do_professor`, o sandbox encerrou o processo com SIGSYS (exit 159) por uma syscall bloqueada por seccomp. A hipótese de que a renomeação do arquivo de saída provocou o bloqueio motivou a gravação direta em `result.txt`. Esta alteração posterior ainda não foi compilada nem revalidada. O autocheck informou 100% dos testes públicos concluídos, mas o critério de implementação registra a interrupção. A transcrição recebida está em `evidencias.log`.

Na sessão local, `gcc` e `mingw32-make` ainda falham ao iniciar devido a erro de realocação de DLL do Windows, e o WSL retorna acesso negado.

Quando o toolchain estiver disponível, executar o exemplo completo do enunciado e comparar a saída, além de testar argumentos inválidos, arquivos ausentes, CSV malformado, vetor com número incorreto de recursos, comando desconhecido, cliente inválido, liberação excessiva e pedidos acima do máximo, indisponíveis ou inseguros.

## Dificuldades e limitações

A principal dificuldade desta etapa é que o sandbox do autocheck bloqueou a execução do cenário principal. A compilação foi confirmada pelo autocheck e um caso de tratamento de erro passou, mas o exemplo do professor e os demais cenários ainda precisam de execução sem essa restrição. O PDF atual é uma base e deverá ser atualizado com os resultados finais.

## Conclusão

A implementação cobre a leitura das entradas, os comandos previstos e a validação de segurança. A compilação e um caso de erro passaram no autocheck. A validação do exemplo principal permanece pendente porque o processo foi terminado pelo sandbox, não por um resultado do programa. A tentativa de evitar a chamada de renomeação do arquivo também requer novo teste.
