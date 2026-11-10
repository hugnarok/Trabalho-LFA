#include "json_io.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void pular_espacos(const char **p) {
    while (**p && isspace((unsigned char)**p)) {
        (*p)++;
    }
}

static bool consumir(const char **p, char c) {
    pular_espacos(p);
    if (**p != c) {
        return false;
    }
    (*p)++;
    return true;
}

static bool ler_string(const char **p, char *out, int out_tam) {
    pular_espacos(p);
    if (**p != '"') {
        return false;
    }
    (*p)++;

    int i = 0;
    while (**p && **p != '"') {
        if (i + 1 >= out_tam) {
            return false;
        }
        out[i++] = **p;
        (*p)++;
    }
    if (**p != '"') {
        return false;
    }
    (*p)++;
    out[i] = '\0';
    return true;
}

static bool chave_igual(const char *chave, const char *a, const char *b) {
    return strcmp(chave, a) == 0 || (b != NULL && strcmp(chave, b) == 0);
}

static bool ler_array_simbolos(const char **p, Automato *af) {
    if (!consumir(p, '[')) {
        return false;
    }
    pular_espacos(p);
    if (**p == ']') {
        (*p)++;
        return true;
    }

    while (1) {
        char s[MAX_NOME];
        if (!ler_string(p, s, sizeof(s)) || s[0] == '\0' || s[1] != '\0') {
            return false;
        }
        if (af->n_alfabeto >= MAX_ALFABETO) {
            return false;
        }
        af->alfabeto[af->n_alfabeto++] = s[0];

        pular_espacos(p);
        if (**p == ',') {
            (*p)++;
            continue;
        }
        if (**p == ']') {
            (*p)++;
            return true;
        }
        return false;
    }
}

static bool ler_array_estados(const char **p, char dest[][MAX_NOME], int *n,
                              int max_n) {
    if (!consumir(p, '[')) {
        return false;
    }
    pular_espacos(p);
    if (**p == ']') {
        (*p)++;
        *n = 0;
        return true;
    }

    *n = 0;
    while (1) {
        if (*n >= max_n) {
            return false;
        }
        if (!ler_string(p, dest[*n], MAX_NOME)) {
            return false;
        }
        (*n)++;

        pular_espacos(p);
        if (**p == ',') {
            (*p)++;
            continue;
        }
        if (**p == ']') {
            (*p)++;
            return true;
        }
        return false;
    }
}

static bool ler_transicao(const char **p, Transicao *t) {
    char simb[MAX_NOME];
    if (!consumir(p, '[')) {
        return false;
    }
    if (!ler_string(p, t->origem, MAX_NOME)) {
        return false;
    }
    if (!consumir(p, ',')) {
        return false;
    }
    if (!ler_string(p, t->destino, MAX_NOME)) {
        return false;
    }
    if (!consumir(p, ',')) {
        return false;
    }
    if (!ler_string(p, simb, sizeof(simb)) || simb[0] == '\0' ||
        simb[1] != '\0') {
        return false;
    }
    t->simbolo = simb[0];
    return consumir(p, ']');
}

static bool ler_array_transicoes(const char **p, Automato *af) {
    if (!consumir(p, '[')) {
        return false;
    }
    pular_espacos(p);
    if (**p == ']') {
        (*p)++;
        return true;
    }

    while (1) {
        if (af->n_transicoes >= MAX_TRANSICOES) {
            return false;
        }
        if (!ler_transicao(p, &af->transicoes[af->n_transicoes])) {
            return false;
        }
        af->n_transicoes++;

        pular_espacos(p);
        if (**p == ',') {
            (*p)++;
            continue;
        }
        if (**p == ']') {
            (*p)++;
            return true;
        }
        return false;
    }
}

static bool ler_objeto_automato(const char **p, Automato *af) {
    automato_init(af);
    if (!consumir(p, '{')) {
        return false;
    }

    pular_espacos(p);
    if (**p == '}') {
        (*p)++;
        return true;
    }

    while (1) {
        char chave[MAX_NOME];
        if (!ler_string(p, chave, sizeof(chave))) {
            return false;
        }
        if (!consumir(p, ':')) {
            return false;
        }

        if (chave_igual(chave, "alfabeto", NULL)) {
            if (!ler_array_simbolos(p, af)) {
                return false;
            }
        } else if (chave_igual(chave, "estados", NULL)) {
            if (!ler_array_estados(p, af->estados, &af->n_estados,
                                   MAX_ESTADOS)) {
                return false;
            }
        } else if (chave_igual(chave, "estados-finais", "estados_finais")) {
            if (!ler_array_estados(p, af->estados_finais, &af->n_finais,
                                   MAX_ESTADOS)) {
                return false;
            }
        } else if (chave_igual(chave, "estado-inicial", "estado_inicial")) {
            if (!ler_string(p, af->estado_inicial, MAX_NOME)) {
                return false;
            }
        } else if (chave_igual(chave, "transicoes", "transições")) {
            if (!ler_array_transicoes(p, af)) {
                return false;
            }
        } else {
            return false;
        }

        pular_espacos(p);
        if (**p == ',') {
            (*p)++;
            continue;
        }
        if (**p == '}') {
            (*p)++;
            return true;
        }
        return false;
    }
}

static char *ler_arquivo_inteiro(const char *caminho, char *erro, int erro_tam) {
    FILE *f = fopen(caminho, "rb");
    if (!f) {
        snprintf(erro, erro_tam, "não foi possível abrir '%s'", caminho);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        snprintf(erro, erro_tam, "erro ao ler '%s'", caminho);
        return NULL;
    }
    long tam = ftell(f);
    if (tam < 0) {
        fclose(f);
        snprintf(erro, erro_tam, "erro ao ler '%s'", caminho);
        return NULL;
    }
    rewind(f);

    char *buf = malloc((size_t)tam + 1);
    if (!buf) {
        fclose(f);
        snprintf(erro, erro_tam, "sem memória para ler JSON");
        return NULL;
    }

    size_t lidos = fread(buf, 1, (size_t)tam, f);
    fclose(f);
    buf[lidos] = '\0';
    return buf;
}

bool carregar_json(const char *caminho, Automato *af1, Automato *af2,
                   bool *af2_presente, char *erro, int erro_tam) {
    *af2_presente = false;
    automato_init(af1);
    automato_init(af2);

    char *buf = ler_arquivo_inteiro(caminho, erro, erro_tam);
    if (!buf) {
        return false;
    }

    const char *p = buf;
    bool ok = false;
    bool tem_af1 = false;

    if (!consumir(&p, '{')) {
        snprintf(erro, erro_tam, "JSON inválido: esperado '{'");
        goto fim;
    }

    pular_espacos(&p);
    if (*p == '}') {
        snprintf(erro, erro_tam, "JSON sem automato1");
        goto fim;
    }

    while (1) {
        char chave[MAX_NOME];
        if (!ler_string(&p, chave, sizeof(chave))) {
            snprintf(erro, erro_tam, "JSON inválido: chave esperada");
            goto fim;
        }
        if (!consumir(&p, ':')) {
            snprintf(erro, erro_tam, "JSON inválido: esperado ':'");
            goto fim;
        }

        if (strcmp(chave, "automato1") == 0) {
            if (!ler_objeto_automato(&p, af1)) {
                snprintf(erro, erro_tam, "falha ao ler automato1");
                goto fim;
            }
            tem_af1 = true;
        } else if (strcmp(chave, "automato2") == 0) {
            if (!ler_objeto_automato(&p, af2)) {
                snprintf(erro, erro_tam, "falha ao ler automato2");
                goto fim;
            }
            *af2_presente = true;
        } else {
            snprintf(erro, erro_tam, "chave desconhecida '%s'", chave);
            goto fim;
        }

        pular_espacos(&p);
        if (*p == ',') {
            p++;
            continue;
        }
        if (*p == '}') {
            p++;
            break;
        }
        snprintf(erro, erro_tam, "JSON inválido após um autômato");
        goto fim;
    }

    if (!tem_af1) {
        snprintf(erro, erro_tam, "JSON sem automato1");
        goto fim;
    }

    if (!automato_validar(af1, erro, erro_tam)) {
        goto fim;
    }
    if (*af2_presente && !automato_validar(af2, erro, erro_tam)) {
        goto fim;
    }

    ok = true;

fim:
    free(buf);
    return ok;
}

static void escrever_af(FILE *f, const Automato *af, const char *nome,
                        int indent) {
    fprintf(f, "%*s\"%s\": {\n", indent, "", nome);

    fprintf(f, "%*s\"alfabeto\": [", indent + 2, "");
    for (int i = 0; i < af->n_alfabeto; i++) {
        fprintf(f, "\"%c\"%s", af->alfabeto[i],
                i + 1 < af->n_alfabeto ? ", " : "");
    }
    fprintf(f, "],\n");

    fprintf(f, "%*s\"estados\": [", indent + 2, "");
    for (int i = 0; i < af->n_estados; i++) {
        fprintf(f, "\"%s\"%s", af->estados[i],
                i + 1 < af->n_estados ? ", " : "");
    }
    fprintf(f, "],\n");

    fprintf(f, "%*s\"estados-finais\": [", indent + 2, "");
    for (int i = 0; i < af->n_finais; i++) {
        fprintf(f, "\"%s\"%s", af->estados_finais[i],
                i + 1 < af->n_finais ? ", " : "");
    }
    fprintf(f, "],\n");

    fprintf(f, "%*s\"estado-inicial\": \"%s\",\n", indent + 2, "",
            af->estado_inicial);

    fprintf(f, "%*s\"transicoes\": [\n", indent + 2, "");
    for (int i = 0; i < af->n_transicoes; i++) {
        const Transicao *t = &af->transicoes[i];
        fprintf(f, "%*s[\"%s\", \"%s\", \"%c\"]%s\n", indent + 4, "",
                t->origem, t->destino, t->simbolo,
                i + 1 < af->n_transicoes ? "," : "");
    }
    fprintf(f, "%*s]\n", indent + 2, "");
    fprintf(f, "%*s}", indent, "");
}

bool exportar_json(const Automato *af1, const Automato *af2, bool tem_af2,
                   const Automato *resultado, bool tem_resultado,
                   const char *caminho, char *erro, int erro_tam) {
    FILE *f = fopen(caminho, "w");
    if (!f) {
        snprintf(erro, erro_tam, "não foi possível criar '%s'", caminho);
        return false;
    }

    fprintf(f, "{\n");
    escrever_af(f, af1, "automato1", 2);

    if (tem_af2) {
        fprintf(f, ",\n");
        escrever_af(f, af2, "automato2", 2);
    }
    if (tem_resultado) {
        fprintf(f, ",\n");
        escrever_af(f, resultado, "resultado", 2);
    }

    fprintf(f, "\n}\n");
    fclose(f);
    return true;
}

bool exportar_um_af(const Automato *af, const char *nome_campo,
                    const char *caminho, char *erro, int erro_tam) {
    FILE *f = fopen(caminho, "w");
    if (!f) {
        snprintf(erro, erro_tam, "não foi possível criar '%s'", caminho);
        return false;
    }
    fprintf(f, "{\n");
    escrever_af(f, af, nome_campo, 2);
    fprintf(f, "\n}\n");
    fclose(f);
    return true;
}
