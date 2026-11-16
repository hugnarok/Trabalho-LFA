# Trabalho LFA — Autômatos Finitos

Trabalho da disciplina **Linguagens Formais e Autômatos (LFA)**: um programa em **C (C11)** para carregar, manipular, testar e exportar autômatos finitos a partir de arquivos JSON.

## Visão geral

O programa mantém até três autômatos em memória:

- **AF1**: primeiro autômato carregado (obrigatório na maioria dos fluxos)
- **AF2**: segundo autômato (usado em operações binárias)
- **AF Resultado**: saída da última conversão/operação realizada

A interação é feita por um menu no terminal, com suporte a conversões entre tipos de autômatos, minimização de AFD, teste de palavras e operações entre linguagens.

## Funcionalidades

- **Carregamento de JSON**: leitura de `automato1`, `automato2` (opcional) e `resultado` (opcional) de arquivos em `data/` ou caminho personalizado.
- **Conversões**:
  - AFN com múltiplos destinos → AFN-λ
  - AFN-λ → AFN
  - AFN → AFD
- **Minimização** de AFD.
- **Teste de palavras**:
  - pelo terminal;
  - por arquivo texto (uma palavra por linha).
- **Operações entre linguagens** (sobre AFDs, usando AF1 e AF2):
  - união;
  - interseção;
  - subtração (`L(AF1) - L(AF2)`);
  - complemento.
- **Exportação** para JSON (arquivo completo ou apenas um autômato).
- **Visualização** dos autômatos atualmente em memória.

## Requisitos

- Compilador C com suporte a **C11** (ex.: `gcc`)
- **GNU Make**
- Sistema compatível com o ambiente de compilação do `Makefile` (Unix/WSL/MinGW)

## Compilação e execução

Para compilar e executar:

```bash
make
./lfa
```

Também é possível compilar e executar em um passo só:

```bash
make run
```

No **Windows**, o binário é gerado como `lfa.exe`: no `cmd`/PowerShell use `lfa.exe`; no Git Bash/WSL, `./lfa` funciona normalmente.

Para remover o binário gerado:

```bash
make clean
```

## Como usar

Ao iniciar, o programa exibe o estado de **AF1**, **AF2** e **AF Resultado** e apresenta o menu:

| Opção | Ação |
| ---: | --- |
| 1 | Carregar JSON (em AF1, AF2, Resultado, carga completa ou cópia entre slots) |
| 2 | Converter AFN múltiplos destinos → AFN-λ |
| 3 | Converter AFN-λ → AFN |
| 4 | Converter AFN → AFD |
| 5 | Minimizar AFD |
| 6 | Testar palavras (terminal) |
| 7 | Testar palavras (arquivo) |
| 8 | União (AF1 ∪ AF2) |
| 9 | Interseção (AF1 ∩ AF2) |
| 10 | Subtração (AF1 − AF2) |
| 11 | Complemento |
| 12 | Exportar JSON |
| 13 | Mostrar autômatos em memória |
| 0 | Sair |

### Dicas de uso

- **Opção 1**: permite escolher arquivos `.json` da pasta `data/`, informar outro caminho, carregar a partir de autômatos já em memória ou copiar entre slots.
- **Opções 2–5 e 11**: são operações unárias — você escolhe a entrada (AF1, AF2 ou Resultado) e a saída é gravada em **AF Resultado** (com opção de copiá-la para AF1/AF2).
- **Opções 6–7**: permitem escolher qual autômato será testado.
  - No terminal, digite `ε`, `<vazio>`, `<epsilon>` ou `<e>` para representar a palavra vazia; uma linha vazia encerra o teste.
  - Por arquivo, o padrão sugerido é `data/palavras.txt` (uma palavra por linha).
  - Símbolos fora do alfabeto do autômato são tratados como rejeição.
- **Opções 8–10**: exigem **AF1** e **AF2** carregados (tipicamente AFDs, conforme a operação).
- **Opção 12**: exporta o conjunto completo (`automato1`, `automato2` e `resultado`, quando existirem) ou apenas um autômato específico. A exportação completa exige **AF1** carregado.

## Formato do JSON

O arquivo de entrada pode conter um ou dois autômatos (e, opcionalmente, um resultado exportado anteriormente):

```json
{
  "automato1": {
    "alfabeto": ["a", "b"],
    "estados": ["q0", "q1"],
    "estados-finais": ["q1"],
    "estado-inicial": "q0",
    "transicoes": [
      ["q0", "q0", "a"],
      ["q0", "q1", "b"],
      ["q1", "q0", "a"],
      ["q1", "q1", "b"]
    ]
  }
}
```

O arquivo também pode conter `automato2` e `resultado`, no mesmo formato de `automato1` (ambos opcionais).

### Conceitos

- **`alfabeto`**: símbolos aceitos pela linguagem.
- **`estados`**: conjunto de estados.
- **`estados-finais`**: subconjunto de `estados`.
- **`estado-inicial`**: estado inicial do autômato.
- **`transicoes`**: lista de triplas no formato `[origem, destino, simbolo]`.
- **`"&"`**: representa transição lambda/épsilon (λ).
- **AFN**: pode haver múltiplas transições para o mesmo par `(origem, símbolo)`.
- **AFD**: não possui transições lambda e deve ser determinístico para as operações que exigem AFD.

Exemplos disponíveis na pasta [`data/`](data/).

## Arquivos de exemplo (`data/`)

| Arquivo | Descrição |
| --- | --- |
| `afd_simples.json` | AFD simples (ex.: aceita palavras que terminam com `b`) |
| `afn_lambda.json` | AFN com transições lambda (`&`) |
| `afn_nao_deterministico.json` | AFN não determinístico |
| `afd_parcial.json` | AFD com transições parciais |
| `dois_afds.json` | Dois AFDs para operações binárias |
| `afds_alfabetos_diferentes.json` | Caso com alfabetos diferentes |
| `palavras.txt` | Lista de palavras para teste por arquivo |
| `af_erro.json`, `test_*.json` | Casos para validação/erros |

## Estrutura do projeto

```text
.
├── data/                 # Arquivos JSON de exemplo e entradas de teste
├── docs/                 # Documentação técnica do projeto
├── lib/                  # Headers (.h)
│   ├── automato.h        # Estrutura do autômato e validações
│   ├── funcoes.h         # Contrato das operações/conversões
│   └── json_io.h         # Leitura e exportação de JSON
├── src/                  # Implementação (.c)
│   ├── automato.c
│   ├── conversao.c
│   ├── json_io.c
│   ├── main.c            # Menu e controle dos autômatos em memória
│   ├── minimizacao.c
│   ├── operacoes.c
│   └── teste.c
├── INSTRUCOES.md         # Enunciado e orientações do trabalho
├── Makefile
└── README.md
```

## Documentação

- [`docs/INDICE.md`](docs/INDICE.md): índice da documentação técnica e mapa menu → código.
- [`docs/`](docs/): capítulos sobre base do autômato, JSON, conversões, minimização, teste de palavras, operações e menu.
- [`INSTRUCOES.md`](INSTRUCOES.md): enunciado, divisão do grupo e checklist do trabalho.

## Equipe

| Papel | Nome |
| --- | --- |
| 1 | Hugo Henrique Marques |
| 2 | Ahmed Amer Hachem |
| 3 | Deivy Rossi Teixeira de Melo |
| 4 | Arthur Batista Lage e Silva |
