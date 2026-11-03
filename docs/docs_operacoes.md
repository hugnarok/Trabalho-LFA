## Changelog

02/11/2026 ~ Arthur

- Atualiza makefile para compilar src/operacoes.c
- Transfere operações do `src/funcoes_stubs.c` para `src/operacoes.c`
- Cria `/docs` para armazenar explicações de codigo
- Atualiza lista de integrantes no README.md
- Cria 2 testes: `data/afd_parcial.json` e `data/afd_alfabetos_diferentes.json`

## Documentação das Operações

### Conceitos necessários

**AFD parcial:** AFD em que algum par (estado, símbolo) não tem transição. Quando a transição falta, a palavra é rejeitada.

**Estado poço:** estado não-final, com auto-laço em todos os símbolos, para onde mandamos todas as combinações (estado, símbolo) sem transição. Ele completa o AFD sem mudar a linguagem aceita.

- No produto (união, interseção e subtração), o poço é **virtual**: existe só como uma posição extra na tabela `delta`.
- No complemento, o poço é **criado de verdade**: entra na lista de estados, nas transições e nos finais.

**Tabela delta:** tabela de consulta no formato `delta[estado][posição do símbolo] = posição do próximo estado`. Serve para responder “se estou neste estado e leio este símbolo, para onde vou?”.

**Produto:** autômato formado por pares `(q, p)`, onde `q` é estado de AF1 e `p` é estado de AF2. Só entram os pares alcançáveis a partir do inicial.

### Funções auxiliares

|Função|Explicação|
|-------|-------|
|`alfabeto_contem`|Verifica se um símbolo já está em um alfabeto, para não repetir.|
|`montar_alfabeto_uniao`|Junta os alfabetos de AF1 e AF2 em um novo alfabeto, sem repetir símbolos.|
|`montar_delta`|Monta a tabela `delta[estado][posição do símbolo] = posição do destino`. Começa tudo apontando para o poço e depois sobrescreve com as transições reais.|
|`nome_estado_poco`|Gera um nome único para o poço que será criado, evitando conflito com estados reais.|
|`validar_afd`|Valida o autômato e verifica se ele é um AFD.|
|`criar_estado`|Adiciona um estado ao resultado e o marca como final se necessário.|
|`montar_nome_par`|Monta o nome `(q,p)` do estado do produto e verifica se cabe em `MAX_NOME`.|

### Núcleo: `produto`

As três operações binárias usam a mesma função. O tipo da operação só muda a regra de estado final.

1. Valida os dois autômatos com `automato_validar`.
2. Verifica se os dois são AFD com `automato_eh_deterministico`. Se não forem, retorna erro pedindo para determinizar antes.
3. Monta o alfabeto união.
4. Monta `delta1` e `delta2`, já com o poço virtual.
5. Cria o estado inicial `(inicial1, inicial2)`.
6. Usa uma fila para percorrer os pares alcançáveis.

Para cada par `(q,p)` e cada símbolo `a`:

```c
proximo_q = delta1[q][a];
proximo_p = delta2[p][a];
```

- Se o par `(proximo_q, proximo_p)` ainda não existe, cria o estado, verifica se é final e enfileira.
- Depois cria a transição `(q,p) --a--> (proximo_q,proximo_p)`.

O `mapa[q][p]` guarda o índice do par no resultado. Ele evita criar o mesmo estado duas vezes. Não confundir:

- `delta1` e `delta2`: para onde cada AFD vai.
- `mapa`: quais pares já viraram estado no resultado.

### Regra de estado final

|Operação|Um par `(q,p)` é final se...|
|---|---|
|União|`q` é final em AF1 **ou** `p` é final em AF2|
|Interseção|`q` é final em AF1 **e** `p` é final em AF2|
|Subtração|`q` é final em AF1 **e** `p` **não** é final em AF2|

No produto, o poço nunca é final, porque representa um lado que já rejeitou a palavra.

### Operações binárias

|Função|Linguagem|Ideia|
|---|---|---|
|`uniao`|`L(AF1) ∪ L(AF2)`|Aceita se pelo menos um dos lados aceitar.|
|`intersecao`|`L(AF1) ∩ L(AF2)`|Aceita só se os dois lados aceitarem.|
|`subtracao`|`L(AF1) − L(AF2)`|Aceita se AF1 aceitar e AF2 rejeitar.|

A ordem importa na subtração: `AF1 − AF2` é diferente de `AF2 − AF1`.

Na união, o poço é importante quando os alfabetos são diferentes: um lado pode não ter transição, mas o outro ainda pode aceitar a palavra. O poço representa “este lado já rejeitou” sem interromper a simulação.

Na subtração, se AF2 cai no poço, significa que AF2 rejeitou. Aí o resultado depende apenas de AF1.

### Complemento

A função `complemento` não usa produto. Ela faz:

1. Valida o autômato e verifica se é AFD.
2. Conta quantas transições faltam.
3. Se faltar alguma, cria o estado `__poco` e liga as transições ausentes até ele.
4. Cria os auto-laços do poço.
5. Inverte os finais: todo estado que não era final passa a ser final, e vice-versa.

O poço entra como final, pois era não-final antes da inversão.

A palavra vazia também funciona: se o inicial não era final, ele passa a ser final no complemento.

### Erros e limites

|Situação|Comportamento|
|---|---|
|Autômato inválido|Usa a mensagem de `automato_validar`.|
|Entrada não é AFD|Pede para determinizar antes (opção 4).|
|Alfabeto da união passa de 32 símbolos|Retorna erro, sem estourar o array.|
|Resultado passa de 128 estados|Retorna erro, sem estourar o array.|
|Resultado passa de 1024 transições|Retorna erro, sem estourar o array.|
|Nome de estado passa de 63 caracteres|Retorna erro, sem truncar.|
|Saída é o mesmo objeto da entrada|Retorna erro.|

### Testes

|Arquivo|O que testa|
|---|---|
|`data/dois_afds.json`|União, interseção, subtração e complemento de AF1.|
|`data/afd_parcial.json`|Complemento com criação de poço.|
|`data/afds_alfabetos_diferentes.json`|União com alfabetos diferentes e poço virtual.|

Resultados esperados com `dois_afds.json`:

|Operação|Finais|
|---|---|
|União|`(q0,p1)`, `(q1,p0)`, `(q1,p1)`|
|Interseção|`(q1,p1)`|
|Subtração|`(q1,p0)`|
|Complemento de AF1|`q0`|

### Integração

- `src/operacoes.c` está no `SRC` do `Makefile`.
- Os stubs de união, interseção, subtração e complemento foram removidos de `src/funcoes_stubs.c`.
- O menu chama as funções nas opções 8 a 11 e guarda o resultado em `AF Resultado`.
- Hoje o menu usa sempre `AF1` nas operações unárias. Para aplicar complemento no `AF Resultado`, seria preciso ajustar o `main.c`.
