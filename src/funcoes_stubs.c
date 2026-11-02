#include "funcoes.h"

#include <stdio.h>
#include <string.h>

/* Stubs: substituir pela implementação real de cada integrante. */

static bool nao_implementado(const char *nome, char *erro, int erro_tam) {
    snprintf(erro, erro_tam, "%s ainda não implementada", nome);
    return false;
}

bool uniao(const Automato *af1, const Automato *af2, Automato *saida,
           char *erro, int erro_tam) {
    (void)af1;
    (void)af2;
    (void)saida;
    return nao_implementado("União", erro, erro_tam);
}

bool intersecao(const Automato *af1, const Automato *af2, Automato *saida,
                char *erro, int erro_tam) {
    (void)af1;
    (void)af2;
    (void)saida;
    return nao_implementado("Interseção", erro, erro_tam);
}

bool subtracao(const Automato *af1, const Automato *af2, Automato *saida,
               char *erro, int erro_tam) {
    (void)af1;
    (void)af2;
    (void)saida;
    return nao_implementado("Subtração", erro, erro_tam);
}

bool complemento(const Automato *af, Automato *saida, char *erro, int erro_tam) {
    (void)af;
    (void)saida;
    return nao_implementado("Complemento", erro, erro_tam);
}
