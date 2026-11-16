# Teste de palavras

**Arquivo:** `src/teste.c`  
**Função:** `testar_palavra`  
**Menu:** **6** (terminal), **7** (arquivo)

## Pré-requisitos

- Autômato válido: **AFD, AFN ou AFN-λ** (não precisa determinizar antes).
- Caractere fora do `alfabeto` → **REJEITA** no menu (`símbolo 'x' fora do alfabeto`); o teste continua. Em `testar_palavra`, `aceita = false` e `return true`.

## Algoritmo

Simulação por conjunto de estados:

1. Ativos = fecho-λ do estado inicial.
2. Para cada caractere: ativos = fecho-λ de todos os destinos `(ativo, símbolo)`. Se ficar vazio → `aceita = false`, fim.
3. Após a palavra: `aceita =` algum estado ativo é final.

Para um AFD, o conjunto sempre tem um único estado, então é a simulação usual.

**ε:** string vazia → aceita se o fecho-λ do inicial contém um final (num AFD: se o inicial é final).

## Menu — opção 6

- Escolhe AF1, AF2 ou Resultado.
- Mostra alfabeto e tipo.
- `ε`, `<vazio>`, `<epsilon>`, `<e>` → palavra vazia.
- **Enter em linha vazia** encerra o modo teste (diferente de ε).

## Menu — opção 7

- Enter → `data/palavras.txt`.
- Uma palavra por linha; linha vazia = ε.
- Palavra com símbolo fora do alfabeto → `REJEITA` e segue para a próxima linha.
