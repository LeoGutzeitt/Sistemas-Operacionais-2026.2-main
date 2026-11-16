# Relatório — Implementação 5: Algoritmo do Banqueiro

> Rascunho técnico. Completar a seção de testes e exportar para `lgbm.pdf` após executar a compilação e os casos de validação em um ambiente funcional.

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

As respostas são escritas em `result.txt.tmp`. O arquivo final só é substituído depois de todo `commands.txt` ser processado com sucesso, evitando publicar saída parcial quando há erro de formato ou liberação inválida.

## Testes realizados

Ainda não foi possível compilar ou executar os testes nesta sessão. `gcc` e `mingw32-make` falham ao iniciar devido a um erro de realocação de DLL do Windows; a consulta ao WSL retorna acesso negado. Esses eventos foram registrados em `evidencias.log`.

Quando o toolchain estiver disponível, executar o exemplo completo do enunciado e comparar a saída, além de testar argumentos inválidos, arquivos ausentes, CSV malformado, vetor com número incorreto de recursos, comando desconhecido, cliente inválido, liberação excessiva e pedidos acima do máximo, indisponíveis ou inseguros.

## Dificuldades e limitações

A principal dificuldade desta etapa foi a indisponibilidade do compilador no ambiente Windows. A lógica foi implementada, mas não há confirmação de compilação ou comportamento em execução. Também falta registrar evidências dos casos positivos e negativos e gerar o PDF final.

## Conclusão

A implementação cobre a leitura das entradas, os comandos previstos, a validação de segurança e a geração transacional do arquivo de saída. A entrega permanece pendente de compilação, execução dos testes e revisão do resultado em Linux, Unix ou macOS.
