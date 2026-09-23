#ifndef AUTOMATO_H
#define AUTOMATO_H

#include <stdbool.h>

#define MAX_NOME 64
#define MAX_ESTADOS 128
#define MAX_ALFABETO 32
#define MAX_TRANSICOES 1024
#define SIMBOLO_LAMBDA '&'

typedef struct {
    char origem[MAX_NOME];
    char destino[MAX_NOME];
    char simbolo; /* '&' = λ */
} Transicao;

typedef struct {
    char alfabeto[MAX_ALFABETO];
    int n_alfabeto;

    char estados[MAX_ESTADOS][MAX_NOME];
    int n_estados;

    char estados_finais[MAX_ESTADOS][MAX_NOME];
    int n_finais;

    char estado_inicial[MAX_NOME];

    Transicao transicoes[MAX_TRANSICOES];
    int n_transicoes;
} Automato;

void automato_init(Automato *af);
bool automato_validar(const Automato *af, char *erro, int erro_tam);
void automato_imprimir(const Automato *af);
int automato_indice_estado(const Automato *af, const char *nome);
bool automato_eh_final(const Automato *af, const char *nome);
bool automato_tem_lambda(const Automato *af);
bool automato_eh_deterministico(const Automato *af);

#endif
