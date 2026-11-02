#include "funcoes.h"

#include <stdio.h>
#include <string.h>


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
    if (!automato_validar(entrada, erro, erro_tam)) {
        return false;
    }

    if (!automato_eh_deterministico(entrada)) {
        snprintf(erro, erro_tam,
                 "o autômato precisa ser determinístico");
        return false;
    }

    automato_init(saida);

   
    bool alcançavel[MAX_ESTADOS] = {false};
    int fila[MAX_ESTADOS];
    int inicio_fila = 0;
    int fim_fila = 0;
    int indice_inicial =
        automato_indice_estado(entrada, entrada->estado_inicial);

    alcançavel[indice_inicial] = true;
    fila[fim_fila++] = indice_inicial;

    while (inicio_fila < fim_fila) {
        int estado_atual = fila[inicio_fila++];

        for (int i = 0; i < entrada->n_transicoes; i++) {
            Transicao t = entrada->transicoes[i];

            if (strcmp(t.origem, entrada->estados[estado_atual]) != 0) {
                continue;
            }

            int destino = automato_indice_estado(entrada, t.destino);
            if (!alcançavel[destino]) {
                alcançavel[destino] = true;
                fila[fim_fila++] = destino;
            }
        }
    }

   
    int classe[MAX_ESTADOS];
    int n_classes = 0;
    int classe_nao_final = -1;
    int classe_final = -1;

    for (int i = 0; i < entrada->n_estados; i++) {
        if (!alcançavel[i]) {
            classe[i] = -1;
            continue;
        }

        if (automato_eh_final(entrada, entrada->estados[i])) {
            if (classe_final < 0) {
                classe_final = n_classes++;
            }
            classe[i] = classe_final;
        } else {
            if (classe_nao_final < 0) {
                classe_nao_final = n_classes++;
            }
            classe[i] = classe_nao_final;
        }
    }

    
    bool mudou;
    do {
        int nova_classe[MAX_ESTADOS];
        int novas_classes = 0;
        mudou = false;

        for (int estado = 0; estado < entrada->n_estados; estado++) {
            if (!alcançavel[estado]) {
                nova_classe[estado] = -1;
                continue;
            }

            int classe_encontrada = -1;

            for (int outro = 0; outro < estado; outro++) {
                if (!alcançavel[outro] || classe[estado] != classe[outro]) {
                    continue;
                }

                bool mesma_assinatura = true;
                for (int simbolo = 0; simbolo < entrada->n_alfabeto;
                     simbolo++) {
                    int destino1 = destino_da_transicao(
                        entrada, estado, entrada->alfabeto[simbolo]);
                    int destino2 = destino_da_transicao(
                        entrada, outro, entrada->alfabeto[simbolo]);

                    int classe_destino1 = -1;
                    int classe_destino2 = -1;

                    if (destino1 >= 0) {
                        classe_destino1 = classe[destino1];
                    }

                    if (destino2 >= 0) {
                        classe_destino2 = classe[destino2];
                    }

                    if (classe_destino1 != classe_destino2) {
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

        for (int i = 0; i < entrada->n_estados; i++) {
            if (alcançavel[i] && classe[i] != nova_classe[i]) {
                mudou = true;
            }
            classe[i] = nova_classe[i];
        }
        n_classes = novas_classes;
    } while (mudou);

    
    saida->n_alfabeto = entrada->n_alfabeto;
    memcpy(saida->alfabeto, entrada->alfabeto,
           sizeof(saida->alfabeto));

    for (int c = 0; c < n_classes; c++) {
        snprintf(saida->estados[saida->n_estados], MAX_NOME, "M%d", c);
        saida->n_estados++;
    }

    int classe_inicial = classe[indice_inicial];
    snprintf(saida->estado_inicial, MAX_NOME, "M%d", classe_inicial);

    for (int estado = 0; estado < entrada->n_estados; estado++) {
        if (!alcançavel[estado] ||
            !automato_eh_final(entrada, entrada->estados[estado])) {
            continue;
        }

        int c = classe[estado];
        bool ja_e_final = automato_eh_final(saida, saida->estados[c]);
        if (!ja_e_final) {
            snprintf(saida->estados_finais[saida->n_finais], MAX_NOME,
                     "M%d", c);
            saida->n_finais++;
        }
    }

    for (int c = 0; c < n_classes; c++) {
        int representante = -1;
        for (int estado = 0; estado < entrada->n_estados; estado++) {
            if (alcançavel[estado] && classe[estado] == c) {
                representante = estado;
                break;
            }
        }

        for (int simbolo = 0; simbolo < entrada->n_alfabeto; simbolo++) {
            int destino = destino_da_transicao(
                entrada, representante, entrada->alfabeto[simbolo]);
            if (destino < 0) {
                continue;
            }

            Transicao nova;
            snprintf(nova.origem, MAX_NOME, "M%d", c);
            snprintf(nova.destino, MAX_NOME, "M%d", classe[destino]);
            nova.simbolo = entrada->alfabeto[simbolo];
            saida->transicoes[saida->n_transicoes++] = nova;
        }
    }

    return true;
}
