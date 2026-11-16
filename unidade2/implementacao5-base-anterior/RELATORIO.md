# Relatório — Implementação 5: Algoritmo do Banqueiro

> Rascunho de relatório. Atualizar a seção de testes com resultados reais assim que a compilação puder ser executada.

## Identificação

- Disciplina: Sistemas Operacionais
- Aluno: Leonardo Gutzeit Borgmann Matos
- Login: `lgbm`
- Atividade: Implementação 5

## Objetivo

Descrever o objetivo da implementação do algoritmo do Banqueiro e as adaptações exigidas para ler os recursos pela linha de comando e os clientes e comandos por arquivos.

## Estruturas e organização

Documentar os arquivos do programa, as estruturas de dados escolhidas e a organização das matrizes `maximum`, `allocation` e `need`, além do vetor `available`.

## Implementação

O programa armazena os totais de recursos, os máximos por cliente, a alocação corrente, a necessidade restante e os recursos disponíveis. A leitura de `customer.txt` constrói a matriz de máximos e inicializa alocação com zero e necessidade com o máximo. O interpretador de `commands.txt` processa `RQ`, `RL` e `*`. Antes de conceder uma requisição, confere o máximo, depois a disponibilidade e por fim simula o algoritmo de segurança do Banqueiro; se o estado resultante for inseguro, desfaz a alocação provisória. Liberações acima da alocação atual são erro. O resultado é escrito temporariamente e renomeado para `result.txt` apenas após o processamento válido de todos os comandos.

## Testes realizados

O build e os testes ainda não foram executados: `gcc` e `mingw32-make` não iniciam neste Windows devido a erro de realocação de DLL, e o WSL retorna acesso negado. A tentativa está registrada em `evidencias.log`. Após corrigir o ambiente, executar o exemplo do enunciado, comparar `result.txt` palavra por palavra e testar erros de argumentos, arquivos ausentes/malformados, cliente inválido, comando desconhecido, liberação excessiva, pedido acima do máximo, indisponível e inseguro.

## Dificuldades encontradas

Registrar dificuldades reais durante o desenvolvimento e as soluções adotadas.

## Limitações e itens não implementados

Ainda falta compilar e executar os testes em Linux/Unix/macOS ou em um ambiente Windows com toolchain funcional. Até essa validação, não se afirma que a implementação está aprovada. O relatório PDF final também depende da coleta desses resultados.

## Conclusão

Resumir o resultado alcançado após concluir a implementação e os testes.
