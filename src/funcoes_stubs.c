#include "funcoes.h"

#include <stdio.h>
#include <string.h>

/* Stubs: substituir pela implementação real de cada integrante. */

static bool nao_implementado(const char *nome, char *erro, int erro_tam) {
    snprintf(erro, erro_tam, "%s ainda não implementada", nome);
    return false;
}

bool afn_multi_para_afn_lambda(const Automato *entrada, Automato *saida,
                               char *erro, int erro_tam) {
    (void)entrada;
    (void)saida;
    return nao_implementado("AFN multi → AFN-λ", erro, erro_tam);
}

bool afn_lambda_para_afn(const Automato *entrada, Automato *saida, char *erro,
                         int erro_tam) {
    (void)entrada;
    (void)saida;
    return nao_implementado("AFN-λ → AFN", erro, erro_tam);
}

bool afn_para_afd(const Automato *entrada, Automato *saida, char *erro,
                  int erro_tam) {
    (void)entrada;
    (void)saida;
    return nao_implementado("AFN → AFD", erro, erro_tam);
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
