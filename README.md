# Trabalho LFA — Autômatos Finitos

**Disciplina:** Linguagens Formais e Autômatos  
**Linguagem:** C (`-std=c11`)  
**Entrega:** 19/11

Programa em C que lê autômatos finitos em JSON e oferece conversões, teste de palavras e operações entre linguagens.

## Compilar e rodar

```bash
make
./lfa
```

No menu, use a opção **1** e digite o **número** do autômato da lista (os caminhos já estão salvos). Use `0` só se quiser informar outro caminho.

## Estrutura

```
lib/                        # headers (.h)
  automato.h                # struct Automato + validação
  json_io.h                 # carregar / exportar JSON
  funcoes.h                 # contrato das 10 funções
src/                        # implementação (.c)
  automato.c
  json_io.c
  operacoes.c               # união, interseção, subtração e complemento
  funcoes_stubs.c           # stubs (cada um substitui a sua parte)
  main.c                    # menu + AF1 / AF2 / AF Resultado
data/                       # JSONs e palavras de teste
docs/
  docs_operacoes.md           # explicação das operações de união, subtração, complemento e interseção
INSTRUCOES.md               # enunciado e divisão do grupo
```

## Divisão (4 integrantes)

| Papel | Funções | Arquivo sugerido |
|---|---|---|
| 1 | JSON, menu, exportar | já na base (`json_io`, `main`) |
| 2 | Conversões #1–#3 | `conversao.c` (trocar stubs) |
| 3 | Minimizar + testar #4–#5 | `minimizacao.c`, `teste.c` |
| 4 | União, interseção, subtração, complemento | `operacoes.c` |

Assinaturas em `lib/funcoes.h`. Ninguém inventa outro formato de autômato.

## Integrantes

| Papel | Nome |
|---|---|
| 1 | Hugo Henrique Marques |
| 2 | Ahmed Amer Hachem |
| 3 | Deivy Rossi Teixeira de Melo |
| 4 | Arthur Batista Lage e Silva |
