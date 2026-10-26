#include "funcoes.h"

#include <stdio.h>
#include <string.h>

/* =========================================================================
 * Funções auxiliares internas (manipulação segura de estados e transições)
 * ========================================================================= */

static bool adicionar_estado(Automato *af, const char *nome, char *erro,
                             int erro_tam) {
    if (automato_indice_estado(af, nome) >= 0) {
        return true; /* já existe */
    }
    if (af->n_estados >= MAX_ESTADOS) {
        snprintf(erro, erro_tam, "limite máximo de estados (%d) atingido",
                 MAX_ESTADOS);
        return false;
    }
    snprintf(af->estados[af->n_estados], MAX_NOME, "%s", nome);
    af->n_estados++;
    return true;
}

static bool adicionar_final(Automato *af, const char *nome, char *erro,
                            int erro_tam) {
    if (automato_eh_final(af, nome)) {
        return true; /* já está marcado como final */
    }
    if (af->n_finais >= MAX_ESTADOS) {
        snprintf(erro, erro_tam, "limite máximo de estados finais atingido");
        return false;
    }
    snprintf(af->estados_finais[af->n_finais], MAX_NOME, "%s", nome);
    af->n_finais++;
    return true;
}

static bool existe_transicao(const Automato *af, const char *origem,
                             const char *destino, char simbolo) {
    for (int i = 0; i < af->n_transicoes; i++) {
        if (af->transicoes[i].simbolo == simbolo &&
            strcmp(af->transicoes[i].origem, origem) == 0 &&
            strcmp(af->transicoes[i].destino, destino) == 0) {
            return true;
        }
    }
    return false;
}

static bool adicionar_transicao(Automato *af, const char *origem,
                                const char *destino, char simbolo, char *erro,
                                int erro_tam) {
    if (existe_transicao(af, origem, destino, simbolo)) {
        return true; /* evita transições duplicadas */
    }
    if (af->n_transicoes >= MAX_TRANSICOES) {
        snprintf(erro, erro_tam, "limite máximo de transições (%d) atingido",
                 MAX_TRANSICOES);
        return false;
    }
    Transicao *t = &af->transicoes[af->n_transicoes];
    snprintf(t->origem, MAX_NOME, "%s", origem);
    snprintf(t->destino, MAX_NOME, "%s", destino);
    t->simbolo = simbolo;
    af->n_transicoes++;
    return true;
}

/* Gera um nome único para estado auxiliar (ex: "q0_a", ou "q0_a_1" se já existir) */
static void gerar_nome_auxiliar(const Automato *af, const char *origem,
                                char simbolo, char *nome_out, int tam) {
    snprintf(nome_out, (size_t)tam, "%s_%c", origem, simbolo);
    if (automato_indice_estado(af, nome_out) < 0) {
        return;
    }

    int sufixo = 1;
    while (1) {
        snprintf(nome_out, (size_t)tam, "%s_%c_%d", origem, simbolo, sufixo);
        if (automato_indice_estado(af, nome_out) < 0) {
            return;
        }
        sufixo++;
    }
}

/* =========================================================================
 * 1) Conversão AFN múltiplo-estado → AFN-λ
 *
 * Ideia:
 *   Quando um mesmo par (origem, simbolo) possui múltiplos destinos (k >= 2),
 *   criamos um estado intermediário auxiliar (ex: "q0_a"):
 *     - origem --simbolo--> estado_aux
 *     - estado_aux --&--> destino_1
 *     - estado_aux --&--> destino_2 ...
 *   Se tiver apenas 1 destino, mantemos a transição original diretamente.
 * ========================================================================= */
bool afn_multi_para_afn_lambda(const Automato *entrada, Automato *saida,
                               char *erro, int erro_tam) {
    if (!entrada || !saida) {
        snprintf(erro, erro_tam, "ponteiro nulo na conversão AFN multi → AFN-λ");
        return false;
    }
    if (!automato_validar(entrada, erro, erro_tam)) {
        return false;
    }

    Automato res;
    automato_init(&res);

    /* Copia alfabeto, estados originais, estado inicial e estados finais */
    res.n_alfabeto = entrada->n_alfabeto;
    memcpy(res.alfabeto, entrada->alfabeto, (size_t)entrada->n_alfabeto);
    snprintf(res.estado_inicial, MAX_NOME, "%s", entrada->estado_inicial);

    for (int i = 0; i < entrada->n_estados; i++) {
        if (!adicionar_estado(&res, entrada->estados[i], erro, erro_tam)) {
            return false;
        }
    }
    for (int i = 0; i < entrada->n_finais; i++) {
        if (!adicionar_final(&res, entrada->estados_finais[i], erro, erro_tam)) {
            return false;
        }
    }

    /* Mantém transições lambda ('&') que já existam no autômato de entrada */
    for (int i = 0; i < entrada->n_transicoes; i++) {
        const Transicao *t = &entrada->transicoes[i];
        if (t->simbolo == SIMBOLO_LAMBDA) {
            if (!adicionar_transicao(&res, t->origem, t->destino,
                                     SIMBOLO_LAMBDA, erro, erro_tam)) {
                return false;
            }
        }
    }

    /* Para cada par (estado de origem, símbolo do alfabeto), agrupa os destinos */
    for (int i = 0; i < entrada->n_estados; i++) {
        const char *origem = entrada->estados[i];

        for (int a = 0; a < entrada->n_alfabeto; a++) {
            char simb = entrada->alfabeto[a];

            /* Coleta índices dos destinos distintos para (origem, simb) */
            int destinos[MAX_ESTADOS];
            int n_dest = 0;

            for (int t = 0; t < entrada->n_transicoes; t++) {
                if (entrada->transicoes[t].simbolo == simb &&
                    strcmp(entrada->transicoes[t].origem, origem) == 0) {
                    int idx_d = automato_indice_estado(
                        entrada, entrada->transicoes[t].destino);
                    if (idx_d >= 0) {
                        bool ja_tem = false;
                        for (int k = 0; k < n_dest; k++) {
                            if (destinos[k] == idx_d) {
                                ja_tem = true;
                                break;
                            }
                        }
                        if (!ja_tem) {
                            destinos[n_dest++] = idx_d;
                        }
                    }
                }
            }

            if (n_dest == 1) {
                /* Apenas 1 destino: mantém transição direta */
                const char *dest = entrada->estados[destinos[0]];
                if (!adicionar_transicao(&res, origem, dest, simb, erro,
                                         erro_tam)) {
                    return false;
                }
            } else if (n_dest > 1) {
                /* Múltiplos destinos: cria estado auxiliar + transições '&' */
                char nome_aux[MAX_NOME];
                gerar_nome_auxiliar(&res, origem, simb, nome_aux,
                                    sizeof(nome_aux));

                if (!adicionar_estado(&res, nome_aux, erro, erro_tam)) {
                    return false;
                }
                if (!adicionar_transicao(&res, origem, nome_aux, simb, erro,
                                         erro_tam)) {
                    return false;
                }
                for (int k = 0; k < n_dest; k++) {
                    const char *dest = entrada->estados[destinos[k]];
                    if (!adicionar_transicao(&res, nome_aux, dest,
                                             SIMBOLO_LAMBDA, erro, erro_tam)) {
                        return false;
                    }
                }
            }
        }
    }

    if (!automato_validar(&res, erro, erro_tam)) {
        return false;
    }

    *saida = res;
    return true;
}

/* =========================================================================
 * 2) Conversão AFN-λ → AFN
 *
 * Ideia:
 *   1. Calcula o fecho-λ(q) para cada estado q (estados alcançáveis só por '&').
 *   2. q passa a ser final se algum estado em fecho-λ(q) for final na entrada.
 *   3. Para cada estado q e símbolo 'a' do alfabeto:
 *      Nova transição q --a--> s para todo estado s alcançável por:
 *      fecho-λ(q) --a--> r --fecho-λ--> s.
 *   4. Remove todas as transições '&'.
 * ========================================================================= */

static void calcular_fecho_lambda(const Automato *af, int idx_inicial,
                                  bool fecho[MAX_ESTADOS]) {
    for (int i = 0; i < MAX_ESTADOS; i++) {
        fecho[i] = false;
    }
    fecho[idx_inicial] = true;

    /* Expande iterativamente pelos caminhos '&' até estabilizar */
    bool mudou = true;
    while (mudou) {
        mudou = false;
        for (int t = 0; t < af->n_transicoes; t++) {
            if (af->transicoes[t].simbolo == SIMBOLO_LAMBDA) {
                int u = automato_indice_estado(af, af->transicoes[t].origem);
                int v = automato_indice_estado(af, af->transicoes[t].destino);
                if (u >= 0 && v >= 0 && fecho[u] && !fecho[v]) {
                    fecho[v] = true;
                    mudou = true;
                }
            }
        }
    }
}

bool afn_lambda_para_afn(const Automato *entrada, Automato *saida, char *erro,
                         int erro_tam) {
    if (!entrada || !saida) {
        snprintf(erro, erro_tam, "ponteiro nulo na conversão AFN-λ → AFN");
        return false;
    }
    if (!automato_validar(entrada, erro, erro_tam)) {
        return false;
    }

    Automato res;
    automato_init(&res);

    /* Copia alfabeto, estados e estado inicial */
    res.n_alfabeto = entrada->n_alfabeto;
    memcpy(res.alfabeto, entrada->alfabeto, (size_t)entrada->n_alfabeto);
    snprintf(res.estado_inicial, MAX_NOME, "%s", entrada->estado_inicial);

    for (int i = 0; i < entrada->n_estados; i++) {
        if (!adicionar_estado(&res, entrada->estados[i], erro, erro_tam)) {
            return false;
        }
    }

    /* Passo 1: calcula o fecho-λ de todos os estados */
    bool fecho[MAX_ESTADOS][MAX_ESTADOS];
    for (int i = 0; i < entrada->n_estados; i++) {
        calcular_fecho_lambda(entrada, i, fecho[i]);
    }

    /* Passo 2: define os novos estados finais (se o fecho-λ contém algum final) */
    for (int i = 0; i < entrada->n_estados; i++) {
        for (int p = 0; p < entrada->n_estados; p++) {
            if (fecho[i][p] &&
                automato_eh_final(entrada, entrada->estados[p])) {
                if (!adicionar_final(&res, entrada->estados[i], erro,
                                     erro_tam)) {
                    return false;
                }
                break;
            }
        }
    }

    /* Passo 3: cria as transições reais usando fecho-λ(q) -> a -> fecho-λ(r) */
    for (int i = 0; i < entrada->n_estados; i++) {
        const char *origem = entrada->estados[i];

        for (int a = 0; a < entrada->n_alfabeto; a++) {
            char simb = entrada->alfabeto[a];

            /* Para cada estado p no fecho-λ(i) */
            for (int p = 0; p < entrada->n_estados; p++) {
                if (!fecho[i][p]) {
                    continue;
                }

                /* Procura transições p --simb--> r */
                for (int t = 0; t < entrada->n_transicoes; t++) {
                    if (entrada->transicoes[t].simbolo == simb &&
                        strcmp(entrada->transicoes[t].origem,
                               entrada->estados[p]) == 0) {
                        int r = automato_indice_estado(
                            entrada, entrada->transicoes[t].destino);
                        if (r < 0) {
                            continue;
                        }

                        /* Adiciona todo estado s pertencente ao fecho-λ(r) */
                        for (int s = 0; s < entrada->n_estados; s++) {
                            if (fecho[r][s]) {
                                if (!adicionar_transicao(
                                        &res, origem, entrada->estados[s], simb,
                                        erro, erro_tam)) {
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (!automato_validar(&res, erro, erro_tam)) {
        return false;
    }

    *saida = res;
    return true;
}

/* =========================================================================
 * 3) Conversão AFN → AFD (Construção de Subconjuntos)
 *
 * Ideia:
 *   - Cada estado do AFD representa um subconjunto não-vazio de estados do AFN.
 *   - Estado inicial do AFD = {q0}.
 *   - A partir dos subconjuntos descobertos (fila), calcula para cada símbolo 'a'
 *     o subconjunto destino (união dos destinos dos estados do subconjunto atual).
 *   - Um subconjunto é final no AFD se contém pelo menos um estado final do AFN.
 *   - Estados inalcançáveis já ficam de fora naturalmente.
 * ========================================================================= */

static bool subconjuntos_iguais(const bool a[MAX_ESTADOS],
                                const bool b[MAX_ESTADOS], int n_estados) {
    for (int i = 0; i < n_estados; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

static bool subconjunto_vazio(const bool conj[MAX_ESTADOS], int n_estados) {
    for (int i = 0; i < n_estados; i++) {
        if (conj[i]) {
            return false;
        }
    }
    return true;
}

static bool subconjunto_contem_final(const Automato *afn,
                                     const bool conj[MAX_ESTADOS]) {
    for (int i = 0; i < afn->n_estados; i++) {
        if (conj[i] && automato_eh_final(afn, afn->estados[i])) {
            return true;
        }
    }
    return false;
}

/* Formata o nome do subconjunto como "{q0,q1}" (ou fallback "S<id>" se muito longo) */
static void nomear_subconjunto(const Automato *afn, const Automato *afd_atual,
                               const bool conj[MAX_ESTADOS], int id_subconjunto,
                               char *nome_out, int tam) {
    int pos = 0;
    nome_out[pos++] = '{';
    nome_out[pos] = '\0';

    bool primeiro = true;
    for (int i = 0; i < afn->n_estados; i++) {
        if (!conj[i]) {
            continue;
        }
        const char *est = afn->estados[i];
        int len_est = (int)strlen(est);
        int extra = (primeiro ? 0 : 1) + len_est;

        /* Garante espaço para o nome + '}' + '\0' */
        if (pos + extra + 2 >= tam) {
            snprintf(nome_out, (size_t)tam, "S%d", id_subconjunto);
            pos = -1;
            break;
        }
        if (!primeiro) {
            nome_out[pos++] = ',';
        }
        memcpy(nome_out + pos, est, (size_t)len_est);
        pos += len_est;
        nome_out[pos] = '\0';
        primeiro = false;
    }

    if (pos >= 0) {
        nome_out[pos++] = '}';
        nome_out[pos] = '\0';
    }

    /* Caso raro de colisão de nome, garante unicidade */
    if (automato_indice_estado(afd_atual, nome_out) >= 0) {
        int sufixo = 1;
        while (1) {
            snprintf(nome_out, (size_t)tam, "S%d_%d", id_subconjunto, sufixo);
            if (automato_indice_estado(afd_atual, nome_out) < 0) {
                break;
            }
            sufixo++;
        }
    }
}

bool afn_para_afd(const Automato *entrada, Automato *saida, char *erro,
                  int erro_tam) {
    if (!entrada || !saida) {
        snprintf(erro, erro_tam, "ponteiro nulo na conversão AFN → AFD");
        return false;
    }
    if (!automato_validar(entrada, erro, erro_tam)) {
        return false;
    }
    if (automato_tem_lambda(entrada)) {
        snprintf(erro, erro_tam,
                 "o autômato possui transições '&'; converta AFN-λ → AFN antes");
        return false;
    }

    Automato res;
    automato_init(&res);

    res.n_alfabeto = entrada->n_alfabeto;
    memcpy(res.alfabeto, entrada->alfabeto, (size_t)entrada->n_alfabeto);

    /* Tabela de subconjuntos descobertos (funciona como fila de busca em largura) */
    bool subconjuntos[MAX_ESTADOS][MAX_ESTADOS];
    memset(subconjuntos, 0, sizeof(subconjuntos));
    int n_subconjuntos = 0;

    /* Subconjunto inicial = {estado_inicial} */
    int idx_ini = automato_indice_estado(entrada, entrada->estado_inicial);
    subconjuntos[0][idx_ini] = true;
    n_subconjuntos = 1;

    char nome_ini[MAX_NOME];
    nomear_subconjunto(entrada, &res, subconjuntos[0], 0, nome_ini,
                       sizeof(nome_ini));
    snprintf(res.estado_inicial, MAX_NOME, "%s", nome_ini);

    if (!adicionar_estado(&res, nome_ini, erro, erro_tam)) {
        return false;
    }
    if (subconjunto_contem_final(entrada, subconjuntos[0])) {
        if (!adicionar_final(&res, nome_ini, erro, erro_tam)) {
            return false;
        }
    }

    /* Processa cada subconjunto alcançável na fila */
    for (int s = 0; s < n_subconjuntos; s++) {
        for (int a = 0; a < entrada->n_alfabeto; a++) {
            char simb = entrada->alfabeto[a];

            /* Calcula o subconjunto destino para (subconjuntos[s], simb) */
            bool novo_conj[MAX_ESTADOS];
            memset(novo_conj, 0, sizeof(novo_conj));

            for (int u = 0; u < entrada->n_estados; u++) {
                if (!subconjuntos[s][u]) {
                    continue;
                }
                for (int t = 0; t < entrada->n_transicoes; t++) {
                    if (entrada->transicoes[t].simbolo == simb &&
                        strcmp(entrada->transicoes[t].origem,
                               entrada->estados[u]) == 0) {
                        int v = automato_indice_estado(
                            entrada, entrada->transicoes[t].destino);
                        if (v >= 0) {
                            novo_conj[v] = true;
                        }
                    }
                }
            }

            /* Se não há destino para este símbolo, não cria transição */
            if (subconjunto_vazio(novo_conj, entrada->n_estados)) {
                continue;
            }

            /* Verifica se o subconjunto destino já foi descoberto */
            int idx_dest = -1;
            for (int k = 0; k < n_subconjuntos; k++) {
                if (subconjuntos_iguais(subconjuntos[k], novo_conj,
                                        entrada->n_estados)) {
                    idx_dest = k;
                    break;
                }
            }

            /* Se é um novo subconjunto, adiciona na fila e no AFD */
            if (idx_dest < 0) {
                if (n_subconjuntos >= MAX_ESTADOS) {
                    snprintf(erro, erro_tam,
                             "limite máximo de estados (%d) atingido no AFD",
                             MAX_ESTADOS);
                    return false;
                }
                idx_dest = n_subconjuntos;
                memcpy(subconjuntos[idx_dest], novo_conj, sizeof(novo_conj));
                n_subconjuntos++;

                char nome_novo[MAX_NOME];
                nomear_subconjunto(entrada, &res, novo_conj, idx_dest,
                                   nome_novo, sizeof(nome_novo));
                if (!adicionar_estado(&res, nome_novo, erro, erro_tam)) {
                    return false;
                }
                if (subconjunto_contem_final(entrada, novo_conj)) {
                    if (!adicionar_final(&res, nome_novo, erro, erro_tam)) {
                        return false;
                    }
                }
            }

            /* Adiciona a transição determinística entre os subconjuntos */
            if (!adicionar_transicao(&res, res.estados[s],
                                     res.estados[idx_dest], simb, erro,
                                     erro_tam)) {
                return false;
            }
        }
    }

    if (!automato_validar(&res, erro, erro_tam)) {
        return false;
    }

    *saida = res;
    return true;
}
