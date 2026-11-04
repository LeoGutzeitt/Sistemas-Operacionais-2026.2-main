# Unidade 2 - Sistemas Operacionais (2026.2)
## Implementação 4: Sincronização do Metrô (METROREC)

- **Instituição:** CESAR School
- **Aluno:** Leonardo Gutzeit Borgmann Matos
- **E-mail institucional / Login:** `lgbm@cesar.school` (`lgbm`)
- **Identificador de Entrega:** `lgbm`

---

## 1. Visão Geral da Entrega

Esta pasta contém todos os artefatos da **Implementação 4** desenvolvidos rigorosamente conforme as especificações do professor:

1. **Diretório de Implementação (`lgbm/`):**
   - Nomeado estritamente com as iniciais do e-mail em minúsculas (`lgbm`).
   - Contém o código-fonte C, headers, suite de testes e Makefile.
2. **Pacote Compactado para o Classroom (`lgbm.tar`):**
   - Compactação do diretório `lgbm/` sem arquivos binários ou objetos compilados.
   - Disponível em `unidade2/lgbm.tar` e na raiz do repositório.
3. **Relatório em PDF para o Classroom (`lgbm.pdf`):**
   - Documento técnico com detalhes de implementação, modelagem, prevenção de deadlocks, tabela de resultados dos testes e dificuldades encontradas.
   - Disponível em `unidade2/lgbm.pdf` e na raiz do repositório.
4. **Relatório em Markdown (`RELATORIO_LGBM.md`):**
   - Código-fonte do relatório para visualização direta no GitHub.

---

## 2. Estrutura de Arquivos

```text
unidade2/
├── README.md               # Este guia geral da Unidade 2
├── RELATORIO_LGBM.md       # Relatório técnico completo em Markdown
├── lgbm.pdf                # Relatório técnico em PDF pronto para submissão
├── lgbm.tar                # Pacote compactado pronto para submissão
├── generate_pdf.py         # Script Python para regenerar o lgbm.pdf
├── package_tar.py          # Script Python para empacotar o lgbm.tar
└── lgbm/                   # Diretório de código-fonte oficial
    ├── Makefile            # Automação de compilação, testes e limpeza
    ├── README.md           # Documentação técnica específica do código C
    ├── sync_car.h          # Definição da struct station e protótipos
    ├── sync_car.c          # Implementação das 4 funções com pthreads
    └── main.c              # Suíte com 7 testes automatizados e modo CLI
```

---

## 3. Instruções de Compilação e Execução

Acesse o diretório `lgbm/`:

```bash
cd unidade2/lgbm
```

### Compilar
```bash
make
```

### Rodar a Bateria Completa de Testes Automatizados
```bash
make test
```
*Todos os 7 testes foram aprovados com 100% de sucesso.*

### Rodar Cenário Customizado
```bash
./sync_car <numero_passageiros> <capacidade_carro>
# Exemplo:
./sync_car 20 6
```

### Limpar Arquivos Compilados
```bash
make clean
```

### Gerar Novo Pacote .tar
```bash
make tar
# ou na pasta unidade2:
python package_tar.py
```

### Regenerar o PDF
```bash
# na pasta unidade2:
python generate_pdf.py
```

---

## 4. Checklist de Submissão no Google Classroom

- [x] Arquivo compactado nomeado exatamente **`lgbm.tar`** (contendo `lgbm/`).
- [x] Relatório nomeado exatamente **`lgbm.pdf`**.
- [x] Repositório GitHub atualizado com o código, `README.md` e `lgbm.pdf`.
- [x] Código em conformidade total com as restrições:
  - Apenas 1 mutex (`pthread_mutex_t`) por `struct station`.
  - Zero semáforos ou outras primitivas.
  - Embarque simultâneo suportado (Requisito 5).
  - Sem espera ocupada (*busy-wait*).
  - Makefile funcional gerando executável `sync_car`.
