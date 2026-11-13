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

Símbolo λ: `"&"` na transição (também aceita `"λ"` e `"ε"`).

- `alfabeto`, `estados` e `estado-inicial` são **obrigatórios**; `estados-finais` e `transicoes` são opcionais.
- Campos extras (ex.: `"nome"`, `"tipo"`) são **ignorados**, no autômato e na raiz.

## Fluxo de leitura

1. `ler_arquivo_inteiro` — limite de **10 MiB**; BOM UTF-8 no início é ignorado.
2. Parser recursivo (caseiro): objetos, arrays, strings com escapes simples (`\"`, `\\`, `\/`, `\n`, `\t`, `\r`). `\uXXXX` não é suportado: escreva o caractere direto (ex.: `"λ"`).
3. Chave duplicada no mesmo objeto → erro.
4. `remover_duplicatas`: símbolo, estado, estado final ou transição repetidos são mantidos uma vez só.
5. `automato_validar` em cada bloco carregado.

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
