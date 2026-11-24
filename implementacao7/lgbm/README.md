# Implementação 7: Gerenciador de Memória Virtual (Virtual Memory Manager)

- **Instituição:** CESAR School  
- **Disciplina:** Sistemas Operacionais (2026.2)  
- **Aluno:** Leonardo Gutzeit Borgmann Matos  
- **E-mail institucional / Login:** `lgbm@cesar.school` (`lgbm`)  
- **Identificador de Entrega:** `lgbm`  

---

## 1. Visão Geral

Este projeto implementa um **Gerenciador de Memória Virtual (Virtual Memory Manager)** baseado no projeto da página P-51 do livro *Operating System Concepts* (Silberschatz et al., 10ª edição), atendendo integralmente às modificações exigidas:

1. **Espaço de Endereçamento Lógico (16 bits):**
   - Endereços entre `0` e `65535`.
   - **Número da Página:** 8 bits superiores (`(address >> 8) & 0xFF`) → 256 páginas (`0` a `255`).
   - **Deslocamento (Offset):** 8 bits inferiores (`address & 0xFF`) → páginas e frames de 256 bytes.
2. **Memória Física com 128 Frames:**
   - Capacidade física total de `128 * 256 = 32.768` bytes.
   - Os frames são preenchidos sequencialmente de `0` até `127`. Quando a memória física fica cheia, o algoritmo de substituição configurado escolhe o frame vítima.
3. **Políticas de Substituição:**
   - **Tabela de Páginas (Memória Física):** `fifo` e `lru`.
   - **TLB (16 entradas):** `fifo` e `lru`.
   - Sempre que uma página é removida da memória física em um *page fault*, sua entrada na tabela de páginas e eventual entrada correspondente na TLB são invalidadas imediatamente.
4. **Busca Concorrente na TLB com Threads POSIX:**
   - A busca na TLB cria **1 thread por entrada (16 threads `pthread`)** para inspecionar as entradas em paralelo, simulando o comportamento associativo em hardware.
5. **Arquivo de Saída (`correct.txt`):**
   - Gravado no formato padrão do Silberschatz:
     ```text
     Virtual address: 16916 Physical address: 20 Value: 0
     ...
     Number of Translated Addresses = 1000
     Page Faults = ...
     Page Fault Rate = ...
     TLB Hits = ...
     TLB Hit Rate = ...
     ```

---

## 2. Estrutura de Arquivos

```text
implementacao7/lgbm/
├── Makefile       # Compilação (gera executável vm) e limpeza (make clean)
├── README.md      # Esta documentação técnica
├── vm.h           # Constantes, structs da TLB, Tabela de Páginas e Frames
├── vm.c           # Motor de tradução, busca paralela na TLB, FIFO/LRU e CLI
├── RELATORIO.md   # Texto-fonte do relatório técnico
└── evidencias.log # Registro de implementação, verificações e notas de ambiente
```

---

## 3. Compilação e Execução

No diretório `implementacao7/lgbm/`:

```bash
make
```

O comando compila o projeto em C11/POSIX (`-Wall -Wextra -Wpedantic -std=c11 -pthread -O2`) e gera o executável **`vm`**.

### Execução
```bash
./vm <arquivo_enderecos> <algoritmo_pagina> <algoritmo_tlb>
```

#### Exemplos:
```bash
# Substituição de página LRU e TLB FIFO (configuração do correct.txt de referência)
./vm address.txt lru fifo

# Substituição de página FIFO e TLB FIFO
./vm address.txt fifo fifo

# Substituição de página LRU e TLB LRU
./vm address.txt lru lru

# Substituição de página FIFO e TLB LRU
./vm address.txt fifo lru
```

### Limpeza
```bash
make clean
```

---

## 4. Tratamento de Erros

O programa valida rigorosamente todas as entradas **antes** de criar ou sobrescrever `correct.txt`:
1. **Número de argumentos:** Exige exatamente 3 argumentos (`argc == 4`). Qualquer quantidade diferente exibe mensagem explicativa e encerra com código `1`.
2. **Tipo de algoritmo:** Aceita estritamente `fifo` ou `lru` para substituição de página e da TLB; qualquer outro valor encerra com erro.
3. **Arquivo de endereços:** Verifica existência, permissão de leitura, arquivo vazio e formatação linha a linha (apenas inteiros decimais no intervalo `0` a `65535`, rejeitando letras, sinais negativos ou linhas malformadas).
4. **Backing Store:** Verifica abertura e tamanho mínimo (`65536` bytes) de `BACKING_STORE.bin`.

---

## 5. Cobertura das Rubricas

| Item da Rubrica | Peso | Suporte no Projeto |
| :--- | :---: | :--- |
| **README + Makefile** | 10% | `README.md` completo + `Makefile` com `make` (`vm`) e `make clean`. |
| **Relatório** | 20% | `RELATORIO.md` estruturado nas 10 seções do Guia de Relatórios. |
| **Tratamento de Erro** | 10% | Validação completa de `argc`, algoritmos, `BACKING_STORE.bin` e pré-validação integral de `address.txt`. |
| **Tradução dos endereços** | 5% | `(addr >> 8) & 0xFF`, `addr & 0xFF`, `(frame << 8) \| offset`, leitura de `signed char`. |
| **Sem TLB (256 frames)** | 10% | Alvo auxiliar `make vm-256-notlb` (mantendo `make` padrão em 128 frames). |
| **Sem TLB (128 frames, FIFO)** | 10% | Alvo auxiliar `make vm-128-notlb` + `./vm address.txt fifo fifo`. |
| **Sem TLB (128 frames, LRU)** | 15% | Alvo auxiliar `make vm-128-notlb` + `./vm address.txt lru fifo`. |
| **Com TLB sem thread (FIFO)** | 5% | Alvo auxiliar `make vm-128-nothreads` + `./vm address.txt lru fifo`. |
| **Com TLB sem thread (LRU)** | 5% | Alvo auxiliar `make vm-128-nothreads` + `./vm address.txt lru lru`. |
| **TLB com thread** | 10% | Comportamento padrão gerado por `make` (16 threads concorrentes por busca na TLB). |
