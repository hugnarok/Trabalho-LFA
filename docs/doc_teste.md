# Teste de palavras

**Arquivo:** `src/teste.c`  
**Função:** `testar_palavra`  
**Menu:** **6** (terminal), **7** (arquivo)

## Pré-requisitos

- Autômato válido e **determinístico**.
- Cada caractere da palavra pertence ao `alfabeto`; senão **erro** (não aceita/rejeita silenciosamente).

## Algoritmo

1. Estado atual = índice do inicial.
2. Para cada caractere: busca transição única `(estado, símbolo)`; se não houver → `aceita = false`, fim.
3. Após a palavra: `aceita =` estado atual é final.

**ε:** string vazia → aceita iff o inicial é final.

## Menu — opção 6

- Escolhe AF1, AF2 ou Resultado.
- Mostra alfabeto e tipo.
- `ε`, `<vazio>`, `<epsilon>`, `<e>` → palavra vazia.
- **Enter em linha vazia** encerra o modo teste (diferente de ε).

## Menu — opção 7

- Enter → `data/palavras.txt`.
- Uma palavra por linha; linha vazia = ε.

## Limitações

- Não simula AFN nem transições `&`.
- Converta para AFD (opções 3–4) antes de testar.
