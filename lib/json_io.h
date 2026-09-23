#ifndef JSON_IO_H
#define JSON_IO_H

#include "automato.h"

#include <stdbool.h>

/* Carrega automato1 (obrigatório) e automato2 (opcional).
 * Retorna true em sucesso. Se não houver automato2, af2_presente = false. */
bool carregar_json(const char *caminho, Automato *af1, Automato *af2,
                   bool *af2_presente, char *erro, int erro_tam);

bool exportar_json(const Automato *af1, const Automato *af2, bool tem_af2,
                   const Automato *resultado, bool tem_resultado,
                   const char *caminho, char *erro, int erro_tam);

bool exportar_um_af(const Automato *af, const char *nome_campo,
                    const char *caminho, char *erro, int erro_tam);

#endif
