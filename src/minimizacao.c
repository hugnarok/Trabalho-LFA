#include "funcoes.h"

#include <stdio.h>
#include <string.h>

/*
 * Minimização de AFD. O resultado é sempre o AFD mínimo COMPLETO.
 *
 * Antes de particionar, o AFD é completado com um estado poço: toda transição
 * que falta passa a ir para o poço, e o poço vai para ele mesmo com qualquer
 * símbolo. Assim, "transição ausente" e "transição para um estado morto"
 * ficam equivalentes e o algoritmo consegue juntar esses estados.
 */

static int destino_da_transicao(const Automato *af, int estado, char simbolo) {
    for (int i = 0; i < af->n_transicoes; i++) {
        Transicao t = af->transicoes[i];

        if (strcmp(t.origem, af->estados[estado]) == 0 &&
            t.simbolo == simbolo) {
            return automato_indice_estado(af, t.destino);
        }
    }

    return -1;
}

bool minimizar_afd(const Automato *entrada, Automato *saida, char *erro, int erro_tam) {
    if (entrada == NULL || saida == NULL) {
        snprintf(erro, erro_tam, "parâmetro inválido em minimizar_afd");
        return false;
    }
    if (saida == entrada) {
        snprintf(erro, erro_tam,
                 "o autômato de saída deve ser diferente da entrada");
        return false;
    }
    if (!automato_validar(entrada, erro, erro_tam)) {
        return false;
    }

    if (!automato_eh_deterministico(entrada)) {
        snprintf(erro, erro_tam,
                 "o autômato precisa ser determinístico");
        return false;
    }

    automato_init(saida);

    /* 1. Tabela de transição completa. O índice n_estados é o poço. */
    int n_estados = entrada->n_estados;
    int poco = n_estados;
    int total = n_estados + 1; /* estados originais + poço */

    static int tabela[MAX_ESTADOS + 1][MAX_ALFABETO];

    for (int estado = 0; estado < n_estados; estado++) {
        for (int simbolo = 0; simbolo < entrada->n_alfabeto; simbolo++) {
            int destino = destino_da_transicao(entrada, estado,
                                               entrada->alfabeto[simbolo]);
            if (destino < 0) {
                destino = poco;
            }
            tabela[estado][simbolo] = destino;
        }
    }
    for (int simbolo = 0; simbolo < entrada->n_alfabeto; simbolo++) {
        tabela[poco][simbolo] = poco;
    }

    bool eh_final[MAX_ESTADOS + 1];
    for (int estado = 0; estado < n_estados; estado++) {
        eh_final[estado] = automato_eh_final(entrada, entrada->estados[estado]);
    }
    eh_final[poco] = false;

    /* 2. Estados alcançáveis a partir do inicial (busca em largura). */
    bool alcancavel[MAX_ESTADOS + 1] = {false};
    int fila[MAX_ESTADOS + 1];
    int inicio_fila = 0;
    int fim_fila = 0;
    int indice_inicial =
        automato_indice_estado(entrada, entrada->estado_inicial);

    alcancavel[indice_inicial] = true;
    fila[fim_fila++] = indice_inicial;

    while (inicio_fila < fim_fila) {
        int estado_atual = fila[inicio_fila++];

        for (int simbolo = 0; simbolo < entrada->n_alfabeto; simbolo++) {
            int destino = tabela[estado_atual][simbolo];
            if (!alcancavel[destino]) {
                alcancavel[destino] = true;
                fila[fim_fila++] = destino;
            }
        }
    }

    /* 3. Partição inicial: finais × não finais. */
    int classe[MAX_ESTADOS + 1];
    int n_classes = 0;
    int classe_nao_final = -1;
    int classe_final = -1;

    for (int estado = 0; estado < total; estado++) {
        if (!alcancavel[estado]) {
            classe[estado] = -1;
            continue;
        }

        if (eh_final[estado]) {
            if (classe_final < 0) {
                classe_final = n_classes++;
            }
            classe[estado] = classe_final;
        } else {
            if (classe_nao_final < 0) {
                classe_nao_final = n_classes++;
            }
            classe[estado] = classe_nao_final;
        }
    }

    /*
     * 4. Refinamento: dois estados da mesma classe continuam juntos só se,
     * para todo símbolo, os destinos caem na mesma classe. Repete até o
     * número de classes parar de aumentar.
     */
    bool mudou;
    do {
        int nova_classe[MAX_ESTADOS + 1];
        int novas_classes = 0;

        for (int estado = 0; estado < total; estado++) {
            if (!alcancavel[estado]) {
                nova_classe[estado] = -1;
                continue;
            }

            int classe_encontrada = -1;

            for (int outro = 0; outro < estado; outro++) {
                if (!alcancavel[outro] || classe[estado] != classe[outro]) {
                    continue;
                }

                bool mesma_assinatura = true;
                for (int simbolo = 0; simbolo < entrada->n_alfabeto;
                     simbolo++) {
                    int destino1 = tabela[estado][simbolo];
                    int destino2 = tabela[outro][simbolo];

                    if (classe[destino1] != classe[destino2]) {
                        mesma_assinatura = false;
                        break;
                    }
                }

                if (mesma_assinatura) {
                    classe_encontrada = nova_classe[outro];
                    break;
                }
            }

            if (classe_encontrada < 0) {
                classe_encontrada = novas_classes++;
            }
            nova_classe[estado] = classe_encontrada;
        }

        mudou = (novas_classes != n_classes);
        for (int estado = 0; estado < total; estado++) {
            classe[estado] = nova_classe[estado];
        }
        n_classes = novas_classes;
    } while (mudou);

    if (n_classes > MAX_ESTADOS) {
        snprintf(erro, erro_tam,
                 "minimização excede o limite de %d estados", MAX_ESTADOS);
        return false;
    }

    /* 5. Monta o AFD mínimo: um estado por classe (M0, M1, ...). */
    saida->n_alfabeto = entrada->n_alfabeto;
    memcpy(saida->alfabeto, entrada->alfabeto,
           sizeof(saida->alfabeto));

    for (int c = 0; c < n_classes; c++) {
        snprintf(saida->estados[saida->n_estados], MAX_NOME, "M%d", c);
        saida->n_estados++;
    }

    int classe_inicial = classe[indice_inicial];
    snprintf(saida->estado_inicial, MAX_NOME, "M%d", classe_inicial);

    for (int c = 0; c < n_classes; c++) {
        /* Representante: primeiro estado alcançável que está na classe c. */
        int representante = -1;
        for (int estado = 0; estado < total; estado++) {
            if (alcancavel[estado] && classe[estado] == c) {
                representante = estado;
                break;
            }
        }

        if (eh_final[representante]) {
            snprintf(saida->estados_finais[saida->n_finais], MAX_NOME,
                     "M%d", c);
            saida->n_finais++;
        }

        for (int simbolo = 0; simbolo < entrada->n_alfabeto; simbolo++) {
            if (saida->n_transicoes >= MAX_TRANSICOES) {
                snprintf(erro, erro_tam,
                         "minimização excede o limite de %d transições",
                         MAX_TRANSICOES);
                return false;
            }

            int destino = tabela[representante][simbolo];

            Transicao nova;
            snprintf(nova.origem, MAX_NOME, "M%d", c);
            snprintf(nova.destino, MAX_NOME, "M%d", classe[destino]);
            nova.simbolo = entrada->alfabeto[simbolo];
            saida->transicoes[saida->n_transicoes++] = nova;
        }
    }

    if (!automato_validar(saida, erro, erro_tam)) {
        return false;
    }

    return true;
}
