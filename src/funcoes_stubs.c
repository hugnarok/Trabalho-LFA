#include "funcoes.h"

#include <stdio.h>

/* Stubs: substituir pela implementação real de cada integrante. */

static bool nao_implementado(const char *nome, char *erro, int erro_tam) {
    snprintf(erro, erro_tam, "%s ainda não implementada", nome);
    return false;
}

bool minimizar_afd(const Automato *entrada, Automato *saida, char *erro,
                   int erro_tam) {
    (void)entrada;
    (void)saida;
    return nao_implementado("Minimizar AFD", erro, erro_tam);
}

bool testar_palavra(const Automato *af, const char *palavra, bool *aceita,
                    char *erro, int erro_tam) {
    (void)af;
    (void)palavra;
    (void)aceita;
    return nao_implementado("Testar palavra", erro, erro_tam);
}
