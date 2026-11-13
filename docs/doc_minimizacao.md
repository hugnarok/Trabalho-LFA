# Minimização de AFD

**Arquivo:** `src/minimizacao.c`  
**Função:** `minimizar_afd`  
**Menu:** **5**

## Pré-requisitos

- `automato_validar` ok.
- `automato_eh_deterministico` (AFD, sem `&`).
- `saida != entrada`.

## Algoritmo

### 1. Inalcançáveis

BFS a partir do inicial; estados não alcançáveis são ignorados (`classe[i] = -1`).

### 2. Partição inicial

- Classe dos **finais** alcançáveis.
- Classe dos **não finais** alcançáveis.

### 3. Refinamento

Repetir até estabilizar: estados na mesma classe permanecem juntos só se, para **cada** símbolo do alfabeto, os destinos caem na mesma classe (transição ausente conta como destino inválido).

### 4. AFD mínimo

Um estado por classe; transições a partir do representante; final se a classe contém algum final original.

## Auxiliar

`destino_da_transicao` — primeira transição (origem, símbolo); `-1` se omitida (AFD parcial).

## Uso

- Opção **5** → **AF Resultado**.
- Mesma linguagem antes e depois; menos estados quando há equivalentes.

## Limites

`MAX_TRANSICOES` na montagem; validação na saída.
