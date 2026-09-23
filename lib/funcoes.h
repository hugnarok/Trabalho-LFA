#ifndef FUNCOES_H
#define FUNCOES_H

#include "automato.h"

#include <stdbool.h>

/* Contrato compartilhado pelo grupo.
 * Cada integrante implementa no respectivo .c e remove o stub. */

/* Integrante 2 — conversões */
bool afn_multi_para_afn_lambda(const Automato *entrada, Automato *saida,
                               char *erro, int erro_tam);
bool afn_lambda_para_afn(const Automato *entrada, Automato *saida, char *erro,
                         int erro_tam);
bool afn_para_afd(const Automato *entrada, Automato *saida, char *erro,
                  int erro_tam);

/* Integrante 3 — minimização e teste */
bool minimizar_afd(const Automato *entrada, Automato *saida, char *erro,
                   int erro_tam);
bool testar_palavra(const Automato *af, const char *palavra, bool *aceita,
                    char *erro, int erro_tam);

/* Integrante 4 — operações entre linguagens (AFDs) */
bool uniao(const Automato *af1, const Automato *af2, Automato *saida,
           char *erro, int erro_tam);
bool intersecao(const Automato *af1, const Automato *af2, Automato *saida,
                char *erro, int erro_tam);
bool subtracao(const Automato *af1, const Automato *af2, Automato *saida,
               char *erro, int erro_tam);
bool complemento(const Automato *af, Automato *saida, char *erro, int erro_tam);

#endif
