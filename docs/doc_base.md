# Base: tipo `Automato` e `automato.c`

**Arquivos:** `lib/automato.h`, `src/automato.c`  
**Usado por:** todo o projeto (JSON, conversões, operações, teste, menu).

## Estrutura de dados

O autômato é um único tipo compartilhado (`lib/funcoes.h` inclui `automato.h`).

| Campo | Significado |
|---|---|
| `alfabeto[]`, `n_alfabeto` | Símbolos da linguagem (sem `&`) |
| `estados[][]`, `n_estados` | Nomes dos estados |
| `estados_finais[][]`, `n_finais` | Subconjunto de finais |
| `estado_inicial[]` | Um único estado inicial |
| `transicoes[]`, `n_transicoes` | Triplas origem, destino, símbolo |
| `SIMBOLO_LAMBDA` (`'&'`) | Apenas nas transições, nunca no alfabeto |

**Limites:** `MAX_ESTADOS` 256, `MAX_TRANSICOES` 4096, `MAX_ALFABETO` 32, `MAX_NOME` 64.

## Funções públicas

| Função | O que faz |
|---|---|
| `automato_init` | Zera a struct. |
| `automato_indice_estado` | Nome → índice em `estados[]`, ou -1. |
| `automato_eh_final` | Verifica se o nome está em `estados_finais`. |
| `automato_tem_lambda` | Existe transição com `&`? |
| `automato_eh_deterministico` | Sem `&` e sem duas transições com o mesmo (origem, símbolo). |
| `automato_validar` | Checagem estrutural antes dos algoritmos. |
| `automato_imprimir` | Saída legível (opção 13). |

## `automato_validar` — regras importantes

- Estado inicial e finais pertencem a `estados`.
- Alfabeto sem `&`, sem símbolos repetidos.
- Estados sem nome vazio; sem duplicar nome de estado ou de final.
- Toda transição: origem/destino conhecidos; símbolo no alfabeto ou `&`.
- Transição idêntica duplicada → erro.
- Nomes de estado sem `"`, `\`, quebra de linha.

## Classificação (resumo no menu)

- **AFN-λ** — `automato_tem_lambda`.
- **AFD** — determinístico e sem λ.
- **AFN** — não determinístico, sem λ.

## Onde entra no fluxo

```
JSON → carregar_json → automato_validar
                              ↓
         conversões / operações / teste (cada uma valida de novo)
                              ↓
                    automato_imprimir / exportar_json
```

## Memória no programa

`main.c` mantém três cópias: **AF1**, **AF2** (opcional), **AF Resultado**. As funções escrevem em buffer de saída separado da entrada (`saida != entrada`).
