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

static bool ler_objeto_automato(const char **p, Automato *af, const char *ctx,
                                char *erro, int erro_tam) {
    bool tem_alfabeto = false;
    bool tem_estados = false;
    bool tem_finais = false;
    bool tem_inicial = false;
    bool tem_transicoes = false;

    automato_init(af);
    if (!consumir(p, '{')) {
        snprintf(erro, erro_tam, "%s: esperado '{'", ctx);
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
            snprintf(erro, erro_tam, "%s: chave de campo esperada", ctx);
            return false;
        }
        if (!consumir(p, ':')) {
            snprintf(erro, erro_tam, "%s: esperado ':' após '%s'", ctx, chave);
            return false;
        }

        if (chave_igual(chave, "alfabeto", NULL)) {
            if (tem_alfabeto) {
                snprintf(erro, erro_tam, "%s: campo 'alfabeto' duplicado", ctx);
                return false;
            }
            tem_alfabeto = true;
            if (!ler_array_simbolos(p, af)) {
                snprintf(erro, erro_tam,
                         "%s: 'alfabeto' inválido (use um caractere por "
                         "símbolo)",
                         ctx);
                return false;
            }
        } else if (chave_igual(chave, "estados", NULL)) {
            if (tem_estados) {
                snprintf(erro, erro_tam, "%s: campo 'estados' duplicado", ctx);
                return false;
            }
            tem_estados = true;
            if (!ler_array_estados(p, af->estados, &af->n_estados,
                                   MAX_ESTADOS)) {
                snprintf(erro, erro_tam,
                         "%s: 'estados' inválido ou excede %d estados", ctx,
                         MAX_ESTADOS);
                return false;
            }
        } else if (chave_igual(chave, "estados-finais", "estados_finais")) {
            if (tem_finais) {
                snprintf(erro, erro_tam,
                         "%s: campo 'estados-finais' duplicado", ctx);
                return false;
            }
            tem_finais = true;
            if (!ler_array_estados(p, af->estados_finais, &af->n_finais,
                                   MAX_ESTADOS)) {
                snprintf(erro, erro_tam, "%s: 'estados-finais' inválido", ctx);
                return false;
            }
        } else if (chave_igual(chave, "estado-inicial", "estado_inicial")) {
            if (tem_inicial) {
                snprintf(erro, erro_tam,
                         "%s: campo 'estado-inicial' duplicado", ctx);
                return false;
            }
            tem_inicial = true;
            if (!ler_string(p, af->estado_inicial, MAX_NOME)) {
                snprintf(erro, erro_tam, "%s: 'estado-inicial' inválido", ctx);
                return false;
            }
        } else if (chave_igual(chave, "transicoes", "transições")) {
            if (tem_transicoes) {
                snprintf(erro, erro_tam,
                         "%s: campo 'transicoes' duplicado", ctx);
                return false;
            }
            tem_transicoes = true;
            if (!ler_array_transicoes(p, af)) {
                snprintf(erro, erro_tam,
                         "%s: 'transicoes' inválida ou excede %d entradas", ctx,
                         MAX_TRANSICOES);
                return false;
            }
        } else {
            snprintf(erro, erro_tam, "%s: campo desconhecido '%s'", ctx, chave);
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
        snprintf(erro, erro_tam, "%s: JSON malformado no objeto", ctx);
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
    if (tam > 10L * 1024L * 1024L) {
        fclose(f);
        snprintf(erro, erro_tam, "arquivo JSON muito grande (máx. 10 MiB)");
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
    if (lidos != (size_t)tam) {
        free(buf);
        snprintf(erro, erro_tam, "leitura incompleta de '%s'", caminho);
        return NULL;
    }
    buf[lidos] = '\0';
    return buf;
}

bool carregar_json(const char *caminho, Automato *af1, Automato *af2,
                   bool *af2_presente, Automato *af_resultado,
                   bool *resultado_presente, char *erro, int erro_tam) {
    *af2_presente = false;
    if (resultado_presente != NULL) {
        *resultado_presente = false;
    }
    automato_init(af1);
    automato_init(af2);
    if (af_resultado != NULL) {
        automato_init(af_resultado);
    }

    char *buf = ler_arquivo_inteiro(caminho, erro, erro_tam);
    if (!buf) {
        return false;
    }

    const char *p = buf;
    bool ok = false;
    bool tem_af1 = false;
    bool chave_af1 = false;
    bool chave_af2 = false;
    bool chave_resultado = false;

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
            if (chave_af1) {
                snprintf(erro, erro_tam, "chave 'automato1' duplicada");
                goto fim;
            }
            chave_af1 = true;
            if (!ler_objeto_automato(&p, af1, "automato1", erro, erro_tam)) {
                goto fim;
            }
            tem_af1 = true;
        } else if (strcmp(chave, "automato2") == 0) {
            if (chave_af2) {
                snprintf(erro, erro_tam, "chave 'automato2' duplicada");
                goto fim;
            }
            chave_af2 = true;
            if (!ler_objeto_automato(&p, af2, "automato2", erro, erro_tam)) {
                goto fim;
            }
            *af2_presente = true;
        } else if (strcmp(chave, "resultado") == 0) {
            if (af_resultado == NULL || resultado_presente == NULL) {
                snprintf(erro, erro_tam,
                         "chave 'resultado' não suportada nesta carga");
                goto fim;
            }
            if (chave_resultado) {
                snprintf(erro, erro_tam, "chave 'resultado' duplicada");
                goto fim;
            }
            chave_resultado = true;
            if (!ler_objeto_automato(&p, af_resultado, "resultado", erro,
                                     erro_tam)) {
                goto fim;
            }
            *resultado_presente = true;
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

    {
        char msg[256];
        if (!automato_validar(af1, msg, sizeof(msg))) {
            snprintf(erro, erro_tam, "automato1: %s", msg);
            goto fim;
        }
    }
    if (*af2_presente) {
        char msg[256];
        if (!automato_validar(af2, msg, sizeof(msg))) {
            snprintf(erro, erro_tam, "automato2: %s", msg);
            goto fim;
        }
    }
    if (resultado_presente != NULL && *resultado_presente) {
        char msg[256];
        if (!automato_validar(af_resultado, msg, sizeof(msg))) {
            snprintf(erro, erro_tam, "resultado: %s", msg);
            goto fim;
        }
    }

    ok = true;

fim:
    free(buf);
    return ok;
}

static void fprint_string_json(FILE *f, const char *s) {
    fputc('"', f);
    if (s != NULL) {
        for (const char *p = s; *p != '\0'; p++) {
            unsigned char c = (unsigned char)*p;
            if (c == '"' || c == '\\') {
                fputc('\\', f);
                fputc((char)c, f);
            } else if (c == '\n') {
                fputs("\\n", f);
            } else {
                fputc((char)c, f);
            }
        }
    }
    fputc('"', f);
}

static void escrever_af(FILE *f, const Automato *af, const char *nome,
                        int indent) {
    fprintf(f, "%*s", indent, "");
    fputc('"', f);
    fputs(nome, f);
    fputs("\": {\n", f);

    fprintf(f, "%*s\"alfabeto\": [", indent + 2, "");
    for (int i = 0; i < af->n_alfabeto; i++) {
        fprintf(f, "\"%c\"%s", af->alfabeto[i],
                i + 1 < af->n_alfabeto ? ", " : "");
    }
    fprintf(f, "],\n");

    fprintf(f, "%*s\"estados\": [", indent + 2, "");
    for (int i = 0; i < af->n_estados; i++) {
        if (i > 0) {
            fputs(", ", f);
        }
        fprint_string_json(f, af->estados[i]);
    }
    fprintf(f, "],\n");

    fprintf(f, "%*s\"estados-finais\": [", indent + 2, "");
    for (int i = 0; i < af->n_finais; i++) {
        if (i > 0) {
            fputs(", ", f);
        }
        fprint_string_json(f, af->estados_finais[i]);
    }
    fprintf(f, "],\n");

    fprintf(f, "%*s\"estado-inicial\": ", indent + 2, "");
    fprint_string_json(f, af->estado_inicial);
    fputs(",\n", f);

    fprintf(f, "%*s\"transicoes\": [\n", indent + 2, "");
    for (int i = 0; i < af->n_transicoes; i++) {
        const Transicao *t = &af->transicoes[i];
        fprintf(f, "%*s[", indent + 4, "");
        fprint_string_json(f, t->origem);
        fputs(", ", f);
        fprint_string_json(f, t->destino);
        fprintf(f, ", \"%c\"]%s\n", t->simbolo,
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
