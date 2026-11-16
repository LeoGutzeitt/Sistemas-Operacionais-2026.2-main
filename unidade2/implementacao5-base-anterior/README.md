# Base anterior da Implementação 5 — Algoritmo do Banqueiro

Esta pasta preserva o esqueleto e a primeira versão de trabalho. A entrega atual e canônica foi movida para a raiz do repositório (`banker.c`, `Makefile`, `README.md`, `RELATORIO.md` e `evidencias.log`) para corresponder ao enunciado.

Implementação em C do algoritmo do Banqueiro com quantidades de recursos definidas pela linha de comando e entrada de clientes/comandos por arquivos.

## Arquivos

- `banker.c`: valida argumentos e arquivos, mantém as estruturas `maximum`, `allocation`, `need` e `available`, processa `RQ`, `RL` e `*`, e verifica estados seguros.
- `Makefile`: compila o executável `banker` neste diretório.
- `RELATORIO.md`: estrutura inicial para documentar implementação, testes, dificuldades e limitações.
- `evidencias.log`: registro inicial das ações realizadas nesta etapa.

Os arquivos `customer.txt`, `commands.txt` e `result.txt` são arquivos de execução e não devem ser versionados.

## Compilação

Com um compilador C disponível:

```sh
make
```

A compilação usa C11 e as opções `-Wall -Wextra -Wpedantic`.

## Execução

```sh
./banker 10 5 7
```

Os argumentos representam as instâncias disponíveis de cada tipo de recurso. No diretório de execução, `customer.txt` contém uma linha por cliente, com os máximos separados por vírgulas; `commands.txt` contém um comando por linha. A saída é gravada em `result.txt`. Consulte o enunciado da atividade para os formatos detalhados.

Pedidos acima do máximo são verificados antes da disponibilidade. Pedidos indisponíveis ou inseguros são negados. Uma liberação maior que a alocação atual e qualquer arquivo malformado encerram o programa com erro. `result.txt` só é atualizado se todo o arquivo de comandos for válido.

## Limpeza

```sh
make clean
```

Remove apenas o executável e arquivos objeto desta atividade.

## Sistema operacional

O código foi preparado em ambiente Windows para compilação C11 e usa recursos padrão da linguagem compatíveis com Linux, Unix e macOS. A compilação local ainda não pôde ser confirmada porque o MinGW instalado falha ao iniciar com erro de realocação de DLL do Windows; o WSL também não está acessível nesta sessão.
