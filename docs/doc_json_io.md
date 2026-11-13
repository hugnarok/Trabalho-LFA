# JSON: carregar e exportar

**Arquivos:** `lib/json_io.h`, `src/json_io.c`  
**Menu:** opção **1** (carregar), **12** (exportar).

## Formato aceito (raiz do arquivo)

```json
{
  "automato1": { ... },
  "automato2": { ... },
  "resultado": { ... }
}
```

- `automato1` é **obrigatório** na carga completa.
- `automato2` e `resultado` são opcionais.

Dentro de cada autômato:

| Chave | Alternativa | Tipo |
|---|---|---|
| `alfabeto` | — | `["a", "b"]` (um caractere por string) |
| `estados` | — | array de strings |
| `estados-finais` | `estados_finais` | array de strings |
| `estado-inicial` | `estado_inicial` | **string** |
| `transicoes` | `transições` | `[[origem, destino, simbolo], ...]` |

Símbolo λ: `"&"` na transição.

## Fluxo de leitura

1. `ler_arquivo_inteiro` — limite de **10 MiB**.
2. Parser recursivo (caseiro): objetos, arrays, strings com escape.
3. Chave duplicada no mesmo objeto → erro.
4. `automato_validar` em cada bloco carregado.

## Funções públicas

| Função | Uso |
|---|---|
| `carregar_json` | Preenche AF1; AF2 e `resultado` se existirem no arquivo. |
| `exportar_json` | Exporta slots carregados (AF1 obrigatório se houver). |
| `exportar_um_af` | Um bloco com nome fixo (`automato1`, `resultado`, …). |

Parâmetros `af_resultado` / `resultado_presente` em `carregar_json` indicam se o bloco `resultado` foi lido.

## Integração com `main.c`

- Lista dinâmica **`data/*.json`** na opção 1.
- Submenu: carregar em AF1 / AF2 / Resultado / carga completa / copiar entre slots.
- Depois da lista de arquivos: números para copiar da **memória** (Resultado, AF1, AF2).
- **0** — digitar caminho manualmente.

## Exportação

Escrita com escape em nomes (`\`, `"`, quebras de linha). Opção 12: arquivo completo ou um slot apenas.

## Compatibilidade

Formatos alternativos do quadro (`automata`, `estado_final` como array, etc.) **não** são aceitos pelo parser atual — adaptar JSON ou estender `json_io.c`.
