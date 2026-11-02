#include "funcoes.h"
#include <stdio.h>
#include <string.h>

bool testar_palavra(const Automato *af, const char *palavra, bool *aceita, char *erro, int erro_tam) {
    if (!automato_validar(af, erro, erro_tam)) {
        return false;
    }

    if (!automato_eh_deterministico(af)) {
        snprintf(erro, erro_tam, "o autômato precisa ser determinístico");
        return false;
    }

    *aceita = false;

    int estado_atual = automato_indice_estado(af, af->estado_inicial);

    if (estado_atual < 0) {
        snprintf(erro, erro_tam, "estado inicial inválido");
        return false;
    }

    for (int i = 0; palavra[i] != '\0'; i++) {
        char simbolo = palavra[i];
        int proximo = -1;

        for(int t = 0; t < af->n_transicoes; t++){
            Transicao transicao = af->transicoes[t];
            
            if(strcmp(transicao.origem, af->estados[estado_atual])==0){

                if(transicao.simbolo == simbolo) {
                   proximo = automato_indice_estado(af, transicao.destino);
                   break;
                }

            }



        }

        if(proximo < 0){
            *aceita=false;
            return true;
        }

        estado_atual=proximo;

    }

    
    *aceita=automato_eh_final(af, af->estados[estado_atual]);
    return true;
}