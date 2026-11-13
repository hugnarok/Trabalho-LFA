# Documentação do Trabalho LFA — Autômatos Finitos

Documentação interna do código para estudo e entrega. Cada capítulo segue o mesmo estilo de `docs_operacoes.md`: conceitos, algoritmo, funções e ligação com o menu.

## Equipe e responsáveis

| Integrante | Arquivos / funções |
|---|---|
| Hugo Henrique Marques | `main.c`, `json_io.c`, `automato.c`, menu e carga |
| Ahmed Amer Hachem | `conversao.c` — opções 2, 3, 4 |
| Deivy Rossi Teixeira de Melo | `minimizacao.c`, `teste.c` — opções 5, 6, 7 |
| Arthur Batista Lage e Silva | `operacoes.c` — opções 8–11 (`docs_operacoes.md`) |

## Capítulos

| Documento | Conteúdo |
|---|---|
| [doc_base.md](doc_base.md) | Struct `Automato`, validação, AFD/AFN |
| [doc_json_io.md](doc_json_io.md) | Leitura e exportação JSON |
| [doc_conversao.md](doc_conversao.md) | AFN multi→λ, λ→AFN, AFN→AFD |
| [doc_minimizacao.md](doc_minimizacao.md) | Minimização de AFD |
| [doc_teste.md](doc_teste.md) | Teste de palavras |
| [docs_operacoes.md](docs_operacoes.md) | União, interseção, subtração, complemento |
| [doc_menu.md](doc_menu.md) | Menu, memória AF1/AF2/Resultado, fluxos |

## Mapa menu → código

| Menu | Função (`funcoes.h`) | Arquivo |
|---:|---|---|
| 1 | `carregar_json` (e fluxo em `main`) | `json_io.c`, `main.c` |
| 2 | `afn_multi_para_afn_lambda` | `conversao.c` |
| 3 | `afn_lambda_para_afn` | `conversao.c` |
| 4 | `afn_para_afd` | `conversao.c` |
| 5 | `minimizar_afd` | `minimizacao.c` |
| 6–7 | `testar_palavra` | `teste.c` |
| 8 | `uniao` | `operacoes.c` |
| 9 | `intersecao` | `operacoes.c` |
| 10 | `subtracao` | `operacoes.c` |
| 11 | `complemento` | `operacoes.c` |
| 12 | `exportar_json` / `exportar_um_af` | `json_io.c` |
| 13 | `automato_imprimir` | `automato.c` |

Todas as opções 2–11 gravam o resultado em **AF Resultado** (e o menu pode copiar para AF1/AF2). Nas opções unárias (2–5 e 11) e no teste (6–7), o usuário escolhe **AF1**, **AF2** ou **Resultado** como entrada.

## Fluxo típico para estudo / demonstração

1. **Carregar** JSON em `data/` (opção 1).
2. Se for AFN: **2 → 3 → 4** (ou pular etapas conforme o autômato).
3. **6 ou 7** para testar palavras (exige AFD).
4. **8–11** se precisar de duas linguagens (AF1 e AF2 AFD).
5. **12** para exportar; **13** para inspecionar.

## Gerar PDF para estudo (fora do repositório)

Com [Pandoc](https://pandoc.org) instalado:

```bash
cd docs
pandoc INDICE.md doc_base.md doc_json_io.md doc_conversao.md doc_minimizacao.md \
  doc_teste.md docs_operacoes.md doc_menu.md \
  -o ~/Desktop/LFA-manual-estudo.pdf \
  --metadata title="Trabalho LFA — Manual do Código"
```

Para Overleaf: copie o conteúdo dos `.md` ou gere LaTeX com `pandoc -t latex ...`.
