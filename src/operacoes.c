#include "funcoes.h"

#include <stdio.h>
#include <string.h>

/* ============================================================
 * Auxiliares comuns às operações
 * ============================================================ */

static bool alfabeto_contem(const char *alfabeto, int n, char simbolo) {
    for (int i = 0; i < n; i++) {
        if (alfabeto[i] == simbolo) {
            return true;
        }
    }
    return false;
}

/* Alfabeto do resultado = união dos alfabetos de AF1 e AF2. */
static bool montar_alfabeto_uniao(const Automato *af1, const Automato *af2,
                                  char *alfabeto, int *n_alfabeto, char *erro,
                                  int erro_tam) {
    const Automato *af[2] = {af1, af2};
    int n = 0;

    for (int k = 0; k < 2; k++) {
        for (int i = 0; i < af[k]->n_alfabeto; i++) {
            char simbolo = af[k]->alfabeto[i];
            if (alfabeto_contem(alfabeto, n, simbolo)) {
                continue;
            }
            if (n >= MAX_ALFABETO) {
                snprintf(erro, erro_tam,
                         "alfabeto do resultado excede o limite de %d símbolos",
                         MAX_ALFABETO);
                return false;
            }
            alfabeto[n++] = simbolo;
        }
    }

    *n_alfabeto = n;
    return true;
}

/*
 * Tabela de transição de um AFD sobre o alfabeto do resultado.
 * O índice af->n_estados representa o estado poço: quando o AFD não tem
 * transição para (q, a), a tabela aponta para esse poço, que fica em si mesmo.
 */
static void montar_delta(const Automato *af, const char *alfabeto,
                         int n_alfabeto,
                         int delta[MAX_ESTADOS + 1][MAX_ALFABETO]) {
    int poco = af->n_estados;

    for (int q = 0; q <= poco; q++) {
        for (int a = 0; a < n_alfabeto; a++) {
            delta[q][a] = poco;
        }
    }

    for (int q = 0; q < af->n_estados; q++) {
        for (int a = 0; a < n_alfabeto; a++) {
            if (!alfabeto_contem(af->alfabeto, af->n_alfabeto, alfabeto[a])) {
                continue; /* símbolo não pertence a este AFD */
            }
            for (int t = 0; t < af->n_transicoes; t++) {
                const Transicao *tr = &af->transicoes[t];
                if (tr->simbolo == alfabeto[a] &&
                    strcmp(tr->origem, af->estados[q]) == 0) {
                    int destino = automato_indice_estado(af, tr->destino);
                    if (destino >= 0) {
                        delta[q][a] = destino;
                    }
                    break;
                }
            }
        }
    }
}

/* Nome do estado poço que não colida com um estado real do autômato. */
static void nome_estado_poco(const Automato *af, const char *prefixo,
                             char *destino) {
    snprintf(destino, MAX_NOME, "%s", prefixo);
    while (automato_indice_estado(af, destino) >= 0) {
        size_t n = strlen(destino);
        if (n + 1 >= MAX_NOME) {
            break;
        }
        destino[n] = '_';
        destino[n + 1] = '\0';
    }
}

static bool validar_afd(const Automato *af, const char *nome, char *erro,
                        int erro_tam) {
    if (!automato_validar(af, erro, erro_tam)) {
        return false;
    }
    if (!automato_eh_deterministico(af)) {
        snprintf(erro, erro_tam,
                 "%s não é AFD (tem & ou transições repetidas); "
                 "determinize antes (opção 4)",
                 nome);
        return false;
    }
    return true;
}

/* ============================================================
 * Produto de AFDs (base para união, interseção e subtração)
 * ============================================================ */

typedef enum {
    PRODUTO_UNIAO,
    PRODUTO_INTERSECAO,
    PRODUTO_SUBTRACAO
} TipoProduto;

static const char *const NOMES_PRODUTO[] = {
    "união",
    "interseção",
    "subtração"
};

static bool eh_final_produto(TipoProduto tipo, bool final1, bool final2) {
    if (tipo == PRODUTO_UNIAO) {
        return final1 || final2;
    }
    if (tipo == PRODUTO_INTERSECAO) {
        return final1 && final2;
    }
    return final1 && !final2;
}

/* Adiciona um estado ao resultado, marcando-o como final se necessário. */
static bool criar_estado(Automato *saida, const char *nome, bool final,
                         const char *operacao, char *erro, int erro_tam) {
    if (saida->n_estados >= MAX_ESTADOS) {
        snprintf(erro, erro_tam, "%s excede o limite de %d estados", operacao,
                 MAX_ESTADOS);
        return false;
    }

    strcpy(saida->estados[saida->n_estados], nome);
    if (final) {
        strcpy(saida->estados_finais[saida->n_finais], nome);
        saida->n_finais++;
    }
    saida->n_estados++;
    return true;
}

static bool montar_nome_par(const char *nome1, const char *nome2, char *destino,
                            const char *operacao, char *erro, int erro_tam) {
    int tam = snprintf(destino, MAX_NOME, "(%s,%s)", nome1, nome2);
    if (tam < 0 || tam >= MAX_NOME) {
        snprintf(erro, erro_tam,
                 "nome de estado do resultado de %s excede %d caracteres",
                 operacao, MAX_NOME - 1);
        return false;
    }
    return true;
}

/*
 * Constrói o autômato produto alcançável de AF1 × AF2.
 * A regra de estado final depende do tipo da operação.
 */
static bool produto(const Automato *af1, const Automato *af2, TipoProduto tipo,
                    Automato *saida, char *erro, int erro_tam) {
    const char *operacao = NOMES_PRODUTO[tipo];

    if (!af1 || !af2 || !saida || !erro || erro_tam <= 0) {
        return false;
    }
    if (saida == af1 || saida == af2) {
        snprintf(erro, erro_tam,
                 "o autômato de saída deve ser diferente das entradas");
        return false;
    }

    if (!validar_afd(af1, "AF1", erro, erro_tam)) {
        return false;
    }
    if (!validar_afd(af2, "AF2", erro, erro_tam)) {
        return false;
    }

    char alfabeto[MAX_ALFABETO];
    int n_alfabeto = 0;
    if (!montar_alfabeto_uniao(af1, af2, alfabeto, &n_alfabeto, erro,
                               erro_tam)) {
        return false;
    }

    int delta1[MAX_ESTADOS + 1][MAX_ALFABETO];
    int delta2[MAX_ESTADOS + 1][MAX_ALFABETO];
    montar_delta(af1, alfabeto, n_alfabeto, delta1);
    montar_delta(af2, alfabeto, n_alfabeto, delta2);

    int n1 = af1->n_estados;
    int n2 = af2->n_estados;

    /* mapa[q][p] = índice do estado (q,p) no resultado, ou -1 se inédito. */
    int mapa[MAX_ESTADOS + 1][MAX_ESTADOS + 1];
    for (int q = 0; q <= n1; q++) {
        for (int p = 0; p <= n2; p++) {
            mapa[q][p] = -1;
        }
    }

    char poco1[MAX_NOME];
    char poco2[MAX_NOME];
    nome_estado_poco(af1, "__poco1", poco1);
    nome_estado_poco(af2, "__poco2", poco2);

    int inicial1 = automato_indice_estado(af1, af1->estado_inicial);
    int inicial2 = automato_indice_estado(af2, af2->estado_inicial);

    automato_init(saida);
    memcpy(saida->alfabeto, alfabeto, (size_t)n_alfabeto);
    saida->n_alfabeto = n_alfabeto;

    char nome[MAX_NOME];
    if (!montar_nome_par(af1->estados[inicial1], af2->estados[inicial2], nome,
                         operacao, erro, erro_tam)) {
        return false;
    }

    bool inicial_final =
        eh_final_produto(tipo, automato_eh_final(af1, af1->estados[inicial1]),
                         automato_eh_final(af2, af2->estados[inicial2]));
    if (!criar_estado(saida, nome, inicial_final, operacao, erro, erro_tam)) {
        return false;
    }
    strcpy(saida->estado_inicial, nome);
    mapa[inicial1][inicial2] = 0;

    int fila1[MAX_ESTADOS];
    int fila2[MAX_ESTADOS];
    int inicio_fila = 0;
    int fim_fila = 0;
    fila1[fim_fila] = inicial1;
    fila2[fim_fila] = inicial2;
    fim_fila++;

    while (inicio_fila < fim_fila) {
        int q = fila1[inicio_fila];
        int p = fila2[inicio_fila];
        int origem = mapa[q][p];
        inicio_fila++;

        for (int a = 0; a < n_alfabeto; a++) {
            int proximo_q = delta1[q][a];
            int proximo_p = delta2[p][a];
            int destino = mapa[proximo_q][proximo_p];

            if (destino < 0) {
                const char *nome_q =
                    (proximo_q == n1) ? poco1 : af1->estados[proximo_q];
                const char *nome_p =
                    (proximo_p == n2) ? poco2 : af2->estados[proximo_p];

                if (!montar_nome_par(nome_q, nome_p, nome, operacao, erro,
                                     erro_tam)) {
                    return false;
                }

                bool final1 =
                    proximo_q < n1 &&
                    automato_eh_final(af1, af1->estados[proximo_q]);
                bool final2 =
                    proximo_p < n2 &&
                    automato_eh_final(af2, af2->estados[proximo_p]);

                destino = saida->n_estados;
                if (!criar_estado(saida, nome,
                                  eh_final_produto(tipo, final1, final2),
                                  operacao, erro, erro_tam)) {
                    return false;
                }
                mapa[proximo_q][proximo_p] = destino;
                fila1[fim_fila] = proximo_q;
                fila2[fim_fila] = proximo_p;
                fim_fila++;
            }

            if (saida->n_transicoes >= MAX_TRANSICOES) {
                snprintf(erro, erro_tam,
                         "%s excede o limite de %d transições", operacao,
                         MAX_TRANSICOES);
                return false;
            }

            Transicao *tr = &saida->transicoes[saida->n_transicoes++];
            strcpy(tr->origem, saida->estados[origem]);
            strcpy(tr->destino, saida->estados[destino]);
            tr->simbolo = alfabeto[a];
        }
    }

    return true;
}

/* ============================================================
 * Operações entre duas linguagens
 * ============================================================ */

bool uniao(const Automato *af1, const Automato *af2, Automato *saida,
           char *erro, int erro_tam) {
    return produto(af1, af2, PRODUTO_UNIAO, saida, erro, erro_tam);
}

bool intersecao(const Automato *af1, const Automato *af2, Automato *saida,
                char *erro, int erro_tam) {
    return produto(af1, af2, PRODUTO_INTERSECAO, saida, erro, erro_tam);
}

bool subtracao(const Automato *af1, const Automato *af2, Automato *saida,
               char *erro, int erro_tam) {
    return produto(af1, af2, PRODUTO_SUBTRACAO, saida, erro, erro_tam);
}

/* ============================================================
 * Complemento de uma linguagem
 * ============================================================ */

/*
 * Completa o AFD com um estado poço (se necessário) e inverte os finais.
 * O complemento só é correto se todas as transições existirem.
 */
bool complemento(const Automato *af, Automato *saida, char *erro, int erro_tam) {
    if (!af || !saida || !erro || erro_tam <= 0) {
        return false;
    }
    if (saida == af) {
        snprintf(erro, erro_tam,
                 "o autômato de saída deve ser diferente da entrada");
        return false;
    }

    if (!validar_afd(af, "AF", erro, erro_tam)) {
        return false;
    }

    int delta[MAX_ESTADOS + 1][MAX_ALFABETO];
    montar_delta(af, af->alfabeto, af->n_alfabeto, delta);
    int indice_poco = af->n_estados;

    int faltantes = 0;
    for (int q = 0; q < af->n_estados; q++) {
        for (int a = 0; a < af->n_alfabeto; a++) {
            if (delta[q][a] == indice_poco) {
                faltantes++;
            }
        }
    }

    bool incompleto = faltantes > 0;
    if (incompleto) {
        if (af->n_estados >= MAX_ESTADOS) {
            snprintf(erro, erro_tam,
                     "complemento excede o limite de %d estados", MAX_ESTADOS);
            return false;
        }
        if (af->n_transicoes + faltantes + af->n_alfabeto > MAX_TRANSICOES) {
            snprintf(erro, erro_tam,
                     "complemento excede o limite de %d transições",
                     MAX_TRANSICOES);
            return false;
        }
    }

    *saida = *af;

    if (incompleto) {
        char poco[MAX_NOME];
        nome_estado_poco(af, "__poco", poco);

        if (!criar_estado(saida, poco, false, "complemento", erro, erro_tam)) {
            return false;
        }

        /* Transições ausentes dos estados originais passam a ir para o poço. */
        for (int q = 0; q < af->n_estados; q++) {
            for (int a = 0; a < af->n_alfabeto; a++) {
                if (delta[q][a] != indice_poco) {
                    continue;
                }
                Transicao *tr = &saida->transicoes[saida->n_transicoes++];
                strcpy(tr->origem, af->estados[q]);
                strcpy(tr->destino, poco);
                tr->simbolo = af->alfabeto[a];
            }
        }

        /* O poço tem auto-laço em todos os símbolos. */
        for (int a = 0; a < saida->n_alfabeto; a++) {
            Transicao *tr = &saida->transicoes[saida->n_transicoes++];
            strcpy(tr->origem, poco);
            strcpy(tr->destino, poco);
            tr->simbolo = saida->alfabeto[a];
        }
    }

    saida->n_finais = 0;
    for (int i = 0; i < saida->n_estados; i++) {
        if (!automato_eh_final(af, saida->estados[i])) {
            strcpy(saida->estados_finais[saida->n_finais],
                   saida->estados[i]);
            saida->n_finais++;
        }
    }

    return true;
}
