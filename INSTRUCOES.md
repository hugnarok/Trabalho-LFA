# Trabalho LFA — Autômatos Finitos

**Disciplina:** Linguagens Formais e Autômatos  
**Entrega:** 19/11  
**Equipe:** 4 integrantes

O programa lê um arquivo JSON com **1 ou 2 autômatos finitos** e executa conversões, testes de palavras e operações entre linguagens. Os autômatos ficam em memória (`AF1`, `AF2` opcional e `AF Resultado`) e podem ser exportados.

---

## 1. Objetivo

Implementar um programa que:

1. Leia um JSON no formato da seção 2.
2. Carregue `automato1` (obrigatório) e `automato2` (opcional) em memória.
3. Ofereça um menu para as 10 funções listadas pelo professor.
4. Grave o resultado das operações em `AF Resultado`.
5. Exporte os autômatos (JSON no mesmo formato; se quiserem, também um desenho/DOT).

---

## 2. Formato do JSON de entrada

O professor passou o formato abaixo. `automato2` é **opcional**.

```json
{
  "automato1": {
    "alfabeto": ["a", "b"],
    "estados": ["q0", "q1", "q2"],
    "estados_finais": ["q2"],
    "estado_inicial": "q0",
    "transicoes": [
      ["q0", "q0", "a"],
      ["q0", "q0", "b"],
      ["q0", "q1", "a"],
      ["q1", "q2", "b"],
      ["q2", "q2", "&"]
    ]
  },
  "automato2": {
    "alfabeto": ["a", "b"],
    "estados": ["p0", "p1"],
    "estados_finais": ["p1"],
    "estado_inicial": "p0",
    "transicoes": [
      ["p0", "p1", "a"],
      ["p1", "p1", "b"]
    ]
  }
}
```

### Convenções combinadas pelo grupo

| Campo | Significado |
|---|---|
| `alfabeto` | Símbolos da linguagem (sem `&`) |
| `estados` | Nomes dos estados |
| `estados_finais` | Subconjunto de `estados` |
| `estado_inicial` | Um único estado |
| `transicoes` | Lista de triplas `[origem, destino, simbolo]` |
| `&` | Transição lambda / épsilon (`λ`) |

**AFN:** o mesmo par `(origem, simbolo)` pode ter vários destinos.  
**AFD:** cada par `(origem, simbolo do alfabeto)` tem no máximo um destino, e não há `&`.

> Se o professor usou chaves com espaço (`"estados finais"`, `"estado inicial"`), o leitor deve aceitar as duas formas.

---

## 3. Modelo em memória

O programa mantém até três autômatos:

```
AF1         ← automato1 do JSON (sempre)
AF2         ← automato2 do JSON (se existir)
AF Resultado ← saída da última operação/conversão
```

Operações que pedem **AFD-2** (união, interseção, subtração) usam `AF1` e `AF2`.  
Operações que pedem **AFD-1** (complemento, minimizar, teste, conversões) usam um autômato escolhido no menu (em geral `AF1` ou `AF Resultado`).

Fluxo típico sugerido no quadro (união via AFN-λ):

```
        λ
Novo I ──► (início de AF1)
        λ
        └──► (início de AF2)

O AF Resultado fica com os estados/finais dos dois, mais o novo inicial.
```

---

## 4. Funções obrigatórias

### Conversões

| # | Função | Entrada | Saída |
|---|---|---|---|
| 1 | AFN múltiplo-estado → AFN-λ | AFN cuja transição pode ir a vários destinos de uma vez | AFN-λ equivalente (estados intermediários + transições `&` se necessário) |
| 2 | AFN-λ → AFN | AFN com `&` | AFN sem `&` (fecho-λ + transições reais) |
| 3 | AFN → AFD | AFN sem `&` | AFD (construção de subconjuntos) |
| 4 | Minimizar AFD | AFD | AFD mínimo (particionamento / Hopcroft ou tabela de pares) |

### Teste e I/O

| # | Função | Detalhe |
|---|---|---|
| 5 | Testar palavras | **Por terminal:** usuário digita a palavra. **Por arquivo texto:** uma palavra por linha. Responder ACEITA / REJEITA para cada uma. |
| 10 | Exportar AFs | Gravar `AF1`, `AF2` e/ou `AF Resultado` em JSON (mesmo formato). |

### Operações entre linguagens (sobre AFD)

| # | Função | Aridade | Ideia |
|---|---|---|---|
| 6 | União | 2 AFDs | Autômato produto; final se **algum** dos dois for final. Alternativa do quadro: novo inicial + `&` para os dois inícios (vira AFN-λ; depois determinizar). |
| 7 | Interseção | 2 AFDs | Autômato produto; final se **os dois** forem finais. |
| 8 | Subtração `L(AF1) − L(AF2)` | 2 AFDs | Produto; final se o de AF1 for final **e** o de AF2 **não**. |
| 9 | Complemento | 1 AFD | Completar o AFD (estado poço se faltar transição) e inverter finais. |

---

## 5. Divisão entre os 4 integrantes

A divisão equilibra teoria e dependência. **O Integrante 1 começa primeiro**: sem estrutura comum, ninguém consegue plugar a sua função.

### Integrante 1 — Base, menu, I/O e exportação

**Funções:** leitura do JSON, memória (`AF1` / `AF2` / `AF Resultado`), menu, **#10 Exportar**.

Entregar:

- Tipo/struct do autômato (estados, alfabeto, inicial, finais, transições).
- `carregar_json(caminho) → AF1, AF2?`
- `exportar_json(af, caminho)`
- Menu no terminal, por exemplo:

```
1) Carregar JSON
2) Converter AFN múltiplo-estado → AFN-λ
3) Converter AFN-λ → AFN
4) Converter AFN → AFD
5) Minimizar AFD
6) Testar palavras (terminal)
7) Testar palavras (arquivo)
8) União
9) Interseção
10) Subtração
11) Complemento
12) Exportar
0) Sair
```

- Validação básica: estado inicial existe, finais ⊆ estados, transições só usam estados conhecidos.

**Contrato para o resto do grupo:** as outras funções recebem e devolvem o **mesmo tipo** de autômato. Combinar a assinatura no primeiro dia, por exemplo:

```
AFN_multi_para_AFN_lambda(af) → af
AFN_lambda_para_AFN(af) → af
AFN_para_AFD(af) → af
minimizar_AFD(af) → af
testar_palavra(af, palavra) → bool
uniao(af1, af2) → af
intersecao(af1, af2) → af
subtracao(af1, af2) → af
complemento(af) → af
```

### Integrante 2 — Conversões (núcleo teórico)

**Funções #1, #2 e #3.**

1. **AFN múltiplo-estado → AFN-λ**  
   Se uma transição “pula” para vários destinos de uma vez, quebrar em AFN-λ equivalente (estados auxiliares + `&`). Se o JSON já vier só com triplas `[origem, destino, simbolo]`, tratar o conjunto de destinos do mesmo `(origem, simbolo)` e documentar a transformação.

2. **AFN-λ → AFN**  
   Para cada estado `q`, calcular o **fecho-λ(q)**.  
   Nova transição com símbolo `a`: de `q` para todos os estados alcançáveis por `fecho-λ → a → fecho-λ`.  
   `q` é final se algum estado do fecho-λ for final. Remover `&`.

3. **AFN → AFD**  
   Construção de subconjuntos: cada estado do AFD é um conjunto de estados do AFN. Estado inicial = `{q0}`. Final se o conjunto contém algum final. Descartar estados inalcançáveis.

Arquivos sugeridos: `conversao_lambda.py`, `determinizacao.py` (ou o equivalente na linguagem escolhida).

### Integrante 3 — Minimização e teste de palavras

**Funções #4 e #5.**

4. **Minimizar AFD**  
   - Remover estados inalcançáveis.  
   - Particionar finais × não-finais e refinar até estabilizar (algoritmo de Hopcroft ou tabela de distinguibilidade).  
   - Juntar estados equivalentes.

5. **Testar palavras**  
   - Terminal: lê uma string e simula o AFD (ou AFN, se o grupo quiser aceitar os dois).  
   - Arquivo: uma palavra por linha; imprimir `palavra → ACEITA|REJEITA`.  
   - Palavra vazia (`ε`): aceita só se o inicial for final (AFD) ou se o fecho-λ do inicial contiver final (AFN-λ).

Arquivos sugeridos: `minimizacao.py`, `teste_palavras.py`.

### Integrante 4 — Operações entre linguagens

**Funções #6, #7, #8 e #9.**

Todas partem de **AFD**. Se o usuário passar AFN, o menu deve avisar para determinizar antes (Integrante 2) — ou o Integrante 4 chama a determinização.

Construção produto `AF1 × AF2`:

- Estados: pares `(q_i, p_j)`
- Inicial: `(inicial1, inicial2)`
- Transição: `δ((q,p), a) = (δ1(q,a), δ2(p,a))`
- **União:** final se `q ∈ F1` **ou** `p ∈ F2`
- **Interseção:** final se `q ∈ F1` **e** `p ∈ F2`
- **Subtração:** final se `q ∈ F1` **e** `p ∉ F2`

**Complemento (1 AFD):**

1. Completar: se faltar `δ(q, a)`, criar estado poço `sink` com `δ(sink, a) = sink` para todo `a`.
2. Novos finais = estados que **não** eram finais.

Arquivo sugerido: `operacoes.py`.

---

## 6. Ordem de implementação (para não travar o grupo)

```
Semana 1
  Integrante 1: struct + JSON + menu vazio (as opções chamam "TODO")
  Integrante 2: fecho-λ e AFN → AFD (dá para testar sem o resto)
  Integrante 3: simulador de AFD (teste no terminal)
  Integrante 4: produto + interseção (a mais direta das operações)

Semana 2
  Integrante 2: AFN-λ → AFN e AFN multi → AFN-λ
  Integrante 3: minimização + teste por arquivo
  Integrante 4: união, subtração, complemento
  Integrante 1: exportação + encaixe de todas as funções no menu

Semana 3 (até 19/11)
  Juntar, criar JSONs de exemplo, testar casos-limite, README de uso
```

---

## 7. Linguagem

O professor listou **bônus** se o trabalho for nessas linguagens:

| Linguagem | Pontos extras |
|---|---|
| COBOL | 3 |
| Assembly | 3 |
| C puro | 2 |
| Pascal | 2 |
| Fortran | 2 |

Sugestão prática: **Python** ou **C** para entregar no prazo com as 10 funções corretas. Bônus só vale a pena se alguém do grupo já tiver fluência — não troquem a linguagem no meio do caminho.

O grupo precisa **escolher uma linguagem no primeiro encontro** e manter o mesmo tipo de dados.

---

## 8. Casos de teste mínimos (todo mundo usa os mesmos)

Criar a pasta `exemplos/` com pelo menos:

1. `afd_simples.json` — AFD que aceita palavras que terminam com `b`.
2. `afn_lambda.json` — AFN-λ com pelo menos uma transição `&`.
3. `afn_nao_deterministico.json` — dois destinos para o mesmo `(estado, simbolo)`.
4. `dois_afds.json` — `automato1` e `automato2` para união / interseção / subtração.
5. `palavras.txt` — linhas com palavras aceitas e rejeitadas.

Cada integrante testa a própria função nesses arquivos antes de integrar.

---

## 9. Regras de integração

- Ninguém inventa um formato paralelo de autômato. Se precisar de campo extra (ex.: tipo `AFD|AFN|AFN-L`), conversar com o Integrante 1.
- Funções **não** leem JSON direto: recebem o objeto já carregado.
- Funções **não** imprimem o AF formatado no meio do algoritmo: devolvem o AF e o menu/exportação cuida da saída.
- Commits pequenos por função (`feat: determinizacao AFN -> AFD`), não um dump no último dia.
- Se uma operação receber autômato incompleto ou do tipo errado, devolver erro claro, sem crash.

---

## 10. Checklist de entrega (19/11)

- [ ] Lê JSON com 1 ou 2 autômatos
- [ ] Conversão AFN múltiplo-estado → AFN-λ
- [ ] Conversão AFN-λ → AFN
- [ ] Conversão AFN → AFD
- [ ] Minimização de AFD
- [ ] Teste de palavras no terminal e por arquivo
- [ ] União, interseção e subtração (2 AFDs)
- [ ] Complemento (1 AFD)
- [ ] Exportação dos AFs
- [ ] Exemplos + instrução de como rodar
- [ ] Nomes dos 4 integrantes no README

---

## 11. Quem fica com o quê (preencher os nomes)

| Papel | Nome | Funções |
|---|---|---|
| Integrante 1 | ________________ | Base, JSON, menu, exportar (#10) |
| Integrante 2 | ________________ | Conversões #1 #2 #3 |
| Integrante 3 | ________________ | Minimizar #4 + teste #5 |
| Integrante 4 | ________________ | União #6, interseção #7, subtração #8, complemento #9 |
