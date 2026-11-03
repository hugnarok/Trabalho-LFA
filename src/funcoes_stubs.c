#include "funcoes.h"

#include <stdio.h>

/* Stubs: substituir pela implementação real de cada integrante. */

static bool nao_implementado(const char *nome, char *erro, int erro_tam) {
    snprintf(erro, erro_tam, "%s ainda não implementada", nome);
    return false;
}
