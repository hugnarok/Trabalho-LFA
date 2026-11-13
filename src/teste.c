#include "funcoes.h"

#include <stdio.h>
#include <string.h>

/*
 * Teste de palavras para AFD, AFN e AFN-λ.
 *
 * Em vez de um único estado atual, guardamos o CONJUNTO de estados ativos:
 *   1. Começa com o fecho-λ do estado inicial.
 *   2. Para cada símbolo, os novos ativos são os destinos das transições
 *      dos estados ativos com esse símbolo (mais o fecho-λ deles).
 *   3. A palavra é aceita se, no fim, algum estado ativo for final.
 * Num AFD o conjunto tem sempre um estado só, então é a simulação de sempre.
 */

static bool simbolo_no_alfabeto(const Automato *af, char simbolo) {
    for (int i = 0; i < af->n_alfabeto; i++) {
        if (af->alfabeto[i] == simbolo) {
            return true;
        }
    }
    return false;
}

/* Acrescenta aos ativos todo estado alcançável só por transições '&'. */
static void aplicar_fecho_lambda(const Automato *af, bool ativos[]) {
    bool mudou = true;
    while (mudou) {
        mudou = false;
        for (int t = 0; t < af->n_transicoes; t++) {
            Transicao transicao = af->transicoes[t];
            if (transicao.simbolo != SIMBOLO_LAMBDA) {
                continue;
            }

            int origem = automato_indice_estado(af, transicao.origem);
            int destino = automato_indice_estado(af, transicao.destino);
            if (ativos[origem] && !ativos[destino]) {
                ativos[destino] = true;
                mudou = true;
            }
        }
    }
}

bool testar_palavra(const Automato *af, const char *palavra, bool *aceita,
                    char *erro, int erro_tam) {
    if (af == NULL || palavra == NULL || aceita == NULL) {
        snprintf(erro, erro_tam, "parâmetro inválido em testar_palavra");
        return false;
    }

    if (!automato_validar(af, erro, erro_tam)) {
        return false;
    }

    *aceita = false;

    for (int i = 0; palavra[i] != '\0'; i++) {
        if (!simbolo_no_alfabeto(af, palavra[i])) {
            snprintf(erro, erro_tam,
                     "símbolo '%c' não pertence ao alfabeto do autômato",
                     palavra[i]);
            return false;
        }
    }

    bool ativos[MAX_ESTADOS] = {false};
    int inicial = automato_indice_estado(af, af->estado_inicial);
    ativos[inicial] = true;
    aplicar_fecho_lambda(af, ativos);

    for (int i = 0; palavra[i] != '\0'; i++) {
        bool proximos[MAX_ESTADOS] = {false};

        for (int t = 0; t < af->n_transicoes; t++) {
            Transicao transicao = af->transicoes[t];
            if (transicao.simbolo != palavra[i]) {
                continue;
            }

            int origem = automato_indice_estado(af, transicao.origem);
            if (ativos[origem]) {
                int destino = automato_indice_estado(af, transicao.destino);
                proximos[destino] = true;
            }
        }

        aplicar_fecho_lambda(af, proximos);
        memcpy(ativos, proximos, sizeof(ativos));
    }

    for (int estado = 0; estado < af->n_estados; estado++) {
        if (ativos[estado] && automato_eh_final(af, af->estados[estado])) {
            *aceita = true;
            break;
        }
    }
    return true;
}
