#include "funcoes.h"

#include <stdio.h>
#include <string.h>

static bool simbolo_no_alfabeto(const Automato *af, char simbolo) {
    for (int i = 0; i < af->n_alfabeto; i++) {
        if (af->alfabeto[i] == simbolo) {
            return true;
        }
    }
    return false;
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

    if (!automato_eh_deterministico(af)) {
        snprintf(erro, erro_tam,
                 "o autômato precisa ser determinístico (sem & e sem "
                 "transições repetidas)");
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

    int estado_atual = automato_indice_estado(af, af->estado_inicial);
    if (estado_atual < 0) {
        snprintf(erro, erro_tam, "estado inicial inválido");
        return false;
    }

    for (int i = 0; palavra[i] != '\0'; i++) {
        char simbolo = palavra[i];
        int proximo = -1;

        for (int t = 0; t < af->n_transicoes; t++) {
            const Transicao *transicao = &af->transicoes[t];

            if (strcmp(transicao->origem, af->estados[estado_atual]) == 0 &&
                transicao->simbolo == simbolo) {
                proximo = automato_indice_estado(af, transicao->destino);
                break;
            }
        }

        if (proximo < 0) {
            *aceita = false;
            return true;
        }

        estado_atual = proximo;
    }

    *aceita = automato_eh_final(af, af->estados[estado_atual]);
    return true;
}
