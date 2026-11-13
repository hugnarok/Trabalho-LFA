# Minimização de AFD

**Arquivo:** `src/minimizacao.c`  
**Função:** `minimizar_afd`  
**Menu:** **5**

## Pré-requisitos

- `automato_validar` ok.
- `automato_eh_deterministico` (AFD, sem `&`).
- `saida != entrada`.

## Algoritmo

O resultado é sempre o **AFD mínimo completo**: todo estado tem transição para todo símbolo. Quando a linguagem exige, aparece um estado de erro (poço) não final que leva todo símbolo para ele mesmo.

### 1. Completar o AFD

Monta a tabela `delta[q][a]`. Transição ausente vai para um **poço implícito** (índice `n`), com `delta[poço][a] = poço`. Assim, transições ausentes e estados mortos explícitos ficam equivalentes, e o particionamento consegue juntá-los.

### 2. Inalcançáveis

BFS a partir do inicial sobre a tabela completa; estados não alcançáveis são ignorados (`classe[q] = -1`). O poço só entra no resultado se for alcançável.

### 3. Partição inicial

- Classe dos **finais** alcançáveis.
- Classe dos **não finais** alcançáveis (inclui o poço).

### 4. Refinamento

Repetir até o número de classes parar de crescer: estados na mesma classe permanecem juntos só se, para **cada** símbolo, os destinos caem na mesma classe.

### 5. AFD mínimo

Um estado por classe (`M0`, `M1`, …); transições a partir do representante; final se o representante é final.

## Uso

- Opção **5** → **AF Resultado**.
- Mesma linguagem antes e depois; número mínimo de estados para um AFD completo.

## Limites

`MAX_ESTADOS` (o poço conta como estado) e `MAX_TRANSICOES` na montagem; validação na saída.
