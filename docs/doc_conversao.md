# Conversões (AFN multi, AFN-λ, AFD)

**Arquivo:** `src/conversao.c`  
**Contrato:** `lib/funcoes.h`  
**Menu:** **2**, **3**, **4** (entrada: AF1, AF2 ou Resultado).

Resultado sempre gravado em **AF Resultado** pelo menu.

## Funções auxiliares (internas)

| Função | Papel |
|---|---|
| `adicionar_estado` / `adicionar_final` | Montagem do autômato destino |
| `adicionar_transicao` | Evita duplicar a mesma transição |
| `gerar_nome_auxiliar` | Estado auxiliar `origem_símbolo` |
| `calcular_fecho_lambda` | Fecho-λ a partir de um estado |
| `nomear_subconjunto` | Nome `{q0,q1}` ou `S0` se muito longo |

---

## `afn_multi_para_afn_lambda` — Menu 2

**Ideia:** Vários destinos para o mesmo (origem, símbolo) viram um auxiliar + transições `&`.

1. Copia alfabeto, estados, inicial, finais, transições λ existentes.
2. Agrupa destinos por (estado, símbolo).
3. Um destino → aresta direta; vários → `origem --s→ aux` e `aux --&→ dest_i`.

---

## `afn_lambda_para_afn` — Menu 3

**Ideia:** Eliminar `&` com fecho-λ.

1. Para cada estado `i`, calcula `fecho[i][]`.
2. Novo final: `i` final se algum estado no fecho de `i` era final.
3. Para cada `i` e símbolo `a`, destinos = união dos alcançados por `fecho[i] --a--> r --fecho--> ...`.
4. Saída sem transições `&`.

---

## `afn_para_afd` — Menu 4

**Ideia:** Construção de subconjuntos.

- **Pré-requisito:** entrada **sem** `&` (use opção 3 antes).
- Fila de subconjuntos de estados do AFN.
- Inicial `{q0}`; final se intersecta finais do AFN.
- Para cada subconjunto `S` e `a`: destino = união de `δ(q,a)` para `q ∈ S`.
- Limite: `MAX_ESTADOS` subconjuntos.

---

## Ordem sugerida

```
AFN com ramificações → (2) se necessário
AFN com &            → (3)
AFN sem &            → (4)
AFD                  → (5), (6)(7), (8–11)
```

## Erros típicos

- Validação falhou, `saida == entrada`, limites de estados/transições.
- AFN→AFD com `&` presente.
