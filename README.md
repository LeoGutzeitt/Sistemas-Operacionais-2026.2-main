# Implementação 5 — Algoritmo do Banqueiro

Esta atividade implementa o algoritmo do Banqueiro com números variáveis de clientes e tipos de recursos. A Implementação 4 permanece em `unidade2/lgbm/`; o rascunho anterior desta atividade foi preservado em `unidade2/implementacao5-base-anterior/`. A entrega atual da Implementação 5 usa os arquivos da raiz do repositório.

## Arquivos

- `banker.c`: valida os argumentos e arquivos, mantém `maximum`, `allocation`, `need` e `available`, processa `RQ`, `RL` e `*`, e verifica estados seguros.
- `Makefile`: compila `banker` na raiz e limpa somente o executável e os objetos desta atividade.
- `evidencias.log`: registra comandos executados, resultados e limitações de validação.
- `RELATORIO.md`: texto-fonte do relatório organizado conforme o Guia de Relatórios; `lgbm.pdf` é a versão diagramada. As evidências finais detalhadas e os prints ainda podem ser anexados.

`customer.txt`, `commands.txt` e `result.txt` são arquivos locais de execução e estão ignorados pelo Git.

## Compilação

Na raiz do repositório:

```sh
make
```

O comando usa C11 e `-Wall -Wextra -Wpedantic`. O executável resultante é `./banker` em Linux, Unix e macOS.

Para limpar os arquivos compilados desta atividade:

```sh
make clean
```

## Entrada e execução

Passe um argumento inteiro positivo para cada tipo de recurso. Por exemplo:

```sh
./banker 10 5 7
```

No diretório de execução, `customer.txt` contém uma linha por cliente, com o máximo de cada tipo de recurso separado por vírgula. `commands.txt` contém um comando por linha: `RQ <cliente> <vetor>`, `RL <cliente> <vetor>` ou `*`. O programa grava a resposta em `result.txt`.

Pedidos são verificados nesta ordem: máximo individual, disponibilidade e segurança do estado. Pedidos indisponíveis ou inseguros são negados com as mensagens definidas no enunciado. Comandos ou arquivos malformados, cliente inexistente e liberações acima do que está alocado encerram a execução com uma mensagem de erro. O programa grava as respostas diretamente em `result.txt`.

## Teste de referência

Use os dados de cinco clientes e os comandos apresentados no enunciado, execute `./banker 10 5 7` e compare as palavras, símbolos e números de `result.txt` com a saída esperada. Os dados de exemplo não são versionados neste repositório.

## Ambiente

O desenvolvimento foi feito em Windows. O código usa C11 padrão e destina-se a Linux, Unix e macOS. Após a alteração para gravar diretamente em `result.txt`, o usuário informou que a nova rodada do autocheck passou. O registro detalhado dessa rodada deve ser anexado a `evidencias.log` quando disponível. Localmente, o GCC/MinGW falha ao iniciar por erro de realocação de DLL e WSL não está acessível. Consulte `evidencias.log` para os detalhes.
