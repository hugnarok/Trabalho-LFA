# Menu e fluxo do programa

**Arquivo:** `src/main.c`  
**Binário:** `lfa` (`make`).

## Memória global

| Slot | Variável | Flag |
|---|---|---|
| AF1 | `af1` | `tem_af1` |
| AF2 | `af2` | `tem_af2` |
| Resultado | `af_resultado` | `tem_resultado` |

## Menu principal

Opções **1–13**, **0** sair. Status no rodapé: tipo (AFD/AFN/AFN-λ), |estados|, |Σ|.

## Opção 1 — Carregar

**Destino:** AF1 / AF2 / Resultado / carga completa / copiar entre slots.

**Origem:**

- Arquivos em `data/*.json` (listagem dinâmica).
- **0** — caminho digitado.
- Números após a lista — copiar da memória (Resultado, AF1, AF2).

Se o JSON tiver só um autômato, o menu pode carregar esse bloco no destino escolhido.

## Opções 2–11

- **Unárias (2–5, 11):** `escolher_automato_unario()` → função de `funcoes.h` → `af_resultado`.
- **Binárias (8–10):** exige AF1 **e** AF2; resultado em `af_resultado`.

Após sucesso: opção de copiar Resultado para AF1, AF2, ambos ou nenhum.

Erros que citam AFD sugerem opção **4**.

## Opções 6–7

Teste com o autômato escolhido (mesmo subselector unário).

## Opção 12 — Exportar

Completo ou apenas Resultado / AF1 / AF2.

## Opção 13 — Mostrar

`automato_imprimir` para cada slot carregado.

## Funções locais úteis

| Função | Papel |
|---|---|
| `escolher_fonte_carregamento` | Arquivo em `data/` ou memória |
| `copiar_da_memoria` | Duplica struct com validação |
| `aplicar_unario` / `aplicar_binario` | Chamam `funcoes.h` e tratam erros |
| `resumo_automato` | Texto do status no menu |

## Demonstração rápida

```text
make && ./lfa
1 → escolher data/dois_afds.json (carga completa ou AF1+AF2)
8 → união
12 → exportar
```

Compilar no Linux na máquina de entrega; binário Mac não é portável.
