#include "automato.h"

#include <stdio.h>
#include <string.h>

void automato_init(Automato *af) {
    memset(af, 0, sizeof(Automato));
}

int automato_indice_estado(const Automato *af, const char *nome) {
    for (int i = 0; i < af->n_estados; i++) {
        if (strcmp(af->estados[i], nome) == 0) {
            return i;
        }
    }
    return -1;
}

bool automato_eh_final(const Automato *af, const char *nome) {
    for (int i = 0; i < af->n_finais; i++) {
        if (strcmp(af->estados_finais[i], nome) == 0) {
            return true;
        }
    }
    return false;
}

bool automato_tem_lambda(const Automato *af) {
    for (int i = 0; i < af->n_transicoes; i++) {
        if (af->transicoes[i].simbolo == SIMBOLO_LAMBDA) {
            return true;
        }
    }
    return false;
}

bool automato_eh_deterministico(const Automato *af) {
    if (automato_tem_lambda(af)) {
        return false;
    }

    for (int i = 0; i < af->n_transicoes; i++) {
        for (int j = i + 1; j < af->n_transicoes; j++) {
            if (af->transicoes[i].simbolo == af->transicoes[j].simbolo &&
                strcmp(af->transicoes[i].origem, af->transicoes[j].origem) == 0) {
                return false;
            }
        }
    }
    return true;
}

bool automato_validar(const Automato *af, char *erro, int erro_tam) {
    if (af->n_estados <= 0) {
        snprintf(erro, erro_tam, "autômato sem estados");
        return false;
    }

    if (automato_indice_estado(af, af->estado_inicial) < 0) {
        snprintf(erro, erro_tam, "estado inicial '%s' não está em estados",
                 af->estado_inicial);
        return false;
    }

    for (int i = 0; i < af->n_finais; i++) {
        if (automato_indice_estado(af, af->estados_finais[i]) < 0) {
            snprintf(erro, erro_tam, "estado final '%s' não está em estados",
                     af->estados_finais[i]);
            return false;
        }
    }

    for (int i = 0; i < af->n_transicoes; i++) {
        const Transicao *t = &af->transicoes[i];
        if (automato_indice_estado(af, t->origem) < 0) {
            snprintf(erro, erro_tam, "transição com origem desconhecida '%s'",
                     t->origem);
            return false;
        }
        if (automato_indice_estado(af, t->destino) < 0) {
            snprintf(erro, erro_tam, "transição com destino desconhecido '%s'",
                     t->destino);
            return false;
        }
        if (t->simbolo != SIMBOLO_LAMBDA) {
            bool no_alfabeto = false;
            for (int a = 0; a < af->n_alfabeto; a++) {
                if (af->alfabeto[a] == t->simbolo) {
                    no_alfabeto = true;
                    break;
                }
            }
            if (!no_alfabeto) {
                snprintf(erro, erro_tam,
                         "símbolo '%c' da transição não está no alfabeto",
                         t->simbolo);
                return false;
            }
        }
    }

    return true;
}

void automato_imprimir(const Automato *af) {
    printf("  Alfabeto: {");
    for (int i = 0; i < af->n_alfabeto; i++) {
        printf("%c%s", af->alfabeto[i], i + 1 < af->n_alfabeto ? ", " : "");
    }
    printf("}\n");

    printf("  Estados: {");
    for (int i = 0; i < af->n_estados; i++) {
        printf("%s%s", af->estados[i], i + 1 < af->n_estados ? ", " : "");
    }
    printf("}\n");

    printf("  Inicial: %s\n", af->estado_inicial);

    printf("  Finais: {");
    for (int i = 0; i < af->n_finais; i++) {
        printf("%s%s", af->estados_finais[i], i + 1 < af->n_finais ? ", " : "");
    }
    printf("}\n");

    printf("  Transições (%d):\n", af->n_transicoes);
    for (int i = 0; i < af->n_transicoes; i++) {
        const Transicao *t = &af->transicoes[i];
        printf("    %s --%c--> %s\n", t->origem, t->simbolo, t->destino);
    }
}
