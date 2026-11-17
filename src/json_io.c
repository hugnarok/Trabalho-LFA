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

/*
 * Lê uma string JSON. Trata os escapes simples (\" \\ \/ \n \t \r).
 * Escapes \uXXXX não são suportados: escreva o caractere direto no arquivo.
 */
static bool ler_string(const char **p, char *out, int out_tam) {
    pular_espacos(p);
    if (**p != '"') {
        return false;
    }
    (*p)++;

    int i = 0;
    while (**p && **p != '"') {
        char c = **p;

        /* Controle cru (tab, newline etc.) não é permitido em string JSON. */
        if ((unsigned char)c < 0x20) {
            return false;
        }

        if (c == '\\') {
            (*p)++; /* pula a barra e olha o caractere seguinte */
            char escapado = **p;
            if (escapado == 'n') {
                c = '\n';
            } else if (escapado == 't') {
                c = '\t';
            } else if (escapado == 'r') {
                c = '\r';
            } else if (escapado == '"' || escapado == '\\' || escapado == '/') {
                c = escapado;
            } else {
                return false; /* escape não suportado */
            }
        }

        if (i + 1 >= out_tam) {
            return false;
        }
        out[i++] = c;
        (*p)++;
    }
    if (**p != '"') {
        return false;
    }
    (*p)++;
    out[i] = '\0';
    return true;
}

/* Avança p até depois do fim da string que começa em p (aspas inclusas). */
static bool pular_string(const char **p) {
    if (**p != '"') {
        return false;
    }
    (*p)++; /* aspas de abertura */
    while (**p && **p != '"') {
        unsigned char c = (unsigned char)**p;
        if (c < 0x20) {
            return false; /* controle cru não é permitido */
        }
        if (c == '\\') {
            (*p)++;
            char escapado = **p;
            if (escapado == '\0') {
                return false;
            }
            if (escapado == 'u') {
                for (int i = 0; i < 4; i++) {
                    (*p)++;
                    if (!isxdigit((unsigned char)**p)) {
                        return false;
                    }
                }
            } else if (strchr("\"\\/bfnrt", escapado) == NULL) {
                return false; /* escape inválido */
            }
        }
        (*p)++;
    }
    if (**p != '"') {
        return false;
    }
    (*p)++; /* aspas de fechamento */
    return true;
}

/* Após um valor, só pode vir espaço, vírgula, ']' , '}' ou fim do arquivo. */
static bool fim_de_valor(char c) {
    return c == '\0' || c == ',' || c == '}' || c == ']' ||
           isspace((unsigned char)c);
}

static bool pular_numero(const char **p) {
    const char *s = *p;
    if (*s == '-') {
        s++;
    }
    if (*s == '0') {
        s++;
    } else if (*s >= '1' && *s <= '9') {
        while (*s >= '0' && *s <= '9') {
            s++;
        }
    } else {
        return false;
    }
    if (*s == '.') {
        s++;
        if (!(*s >= '0' && *s <= '9')) {
            return false;
        }
        while (*s >= '0' && *s <= '9') {
            s++;
        }
    }
    if (*s == 'e' || *s == 'E') {
        s++;
        if (*s == '+' || *s == '-') {
            s++;
        }
        if (!(*s >= '0' && *s <= '9')) {
            return false;
        }
        while (*s >= '0' && *s <= '9') {
            s++;
        }
    }
    if (!fim_de_valor(*s)) {
        return false;
    }
    *p = s;
    return true;
}

static bool pular_valor_rec(const char **p, int profundidade);

/*
 * Pula um valor JSON qualquer, validando a estrutura. Usado para ignorar
 * campos que o programa não conhece (ex.: "nome", "tipo"), mas sem aceitar
 * lixo como 'tru' ou colchetes/chaves desemparelhados.
 */
static bool pular_valor_rec(const char **p, int profundidade) {
    if (profundidade > 64) {
        return false; /* aninhamento excessivo */
    }

    pular_espacos(p);
    char c = **p;

    if (c == '"') {
        return pular_string(p);
    }

    if (c == '{') {
        (*p)++;
        pular_espacos(p);
        if (**p == '}') {
            (*p)++;
            return true;
        }
        while (1) {
            pular_espacos(p);
            if (**p != '"' || !pular_string(p)) {
                return false;
            }
            if (!consumir(p, ':')) {
                return false;
            }
            if (!pular_valor_rec(p, profundidade + 1)) {
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

    if (c == '[') {
        (*p)++;
        pular_espacos(p);
        if (**p == ']') {
            (*p)++;
            return true;
        }
        while (1) {
            if (!pular_valor_rec(p, profundidade + 1)) {
                return false;
            }
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

    if (c == 't') {
        if (strncmp(*p, "true", 4) == 0 && fim_de_valor((*p)[4])) {
            *p += 4;
            return true;
        }
        return false;
    }
    if (c == 'f') {
        if (strncmp(*p, "false", 5) == 0 && fim_de_valor((*p)[5])) {
            *p += 5;
            return true;
        }
        return false;
    }
    if (c == 'n') {
        if (strncmp(*p, "null", 4) == 0 && fim_de_valor((*p)[4])) {
            *p += 4;
            return true;
        }
        return false;
    }

    return pular_numero(p);
}

static bool pular_valor(const char **p) {
    return pular_valor_rec(p, 0);
}

/* "λ" e "ε" (UTF-8) também são aceitos como lambda nas transições. */
static bool eh_lambda_unicode(const char *s) {
    return strcmp(s, "λ") == 0 || strcmp(s, "ε") == 0;
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
    if (!ler_string(p, simb, sizeof(simb))) {
        return false;
    }
    if (eh_lambda_unicode(simb)) {
        t->simbolo = SIMBOLO_LAMBDA;
    } else if (simb[0] != '\0' && simb[1] == '\0') {
        t->simbolo = simb[0];
    } else {
        return false;
    }
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

/* Remove nomes repetidos de uma lista, mantendo a primeira ocorrência. */
static void remover_nomes_repetidos(char lista[][MAX_NOME], int *n) {
    int unicos = 0;
    for (int i = 0; i < *n; i++) {
        bool repetido = false;
        for (int j = 0; j < unicos; j++) {
            if (strcmp(lista[j], lista[i]) == 0) {
                repetido = true;
            }
        }
        if (!repetido) {
            if (unicos != i) {
                strcpy(lista[unicos], lista[i]);
            }
            unicos++;
        }
    }
    *n = unicos;
}

/*
 * O JSON pode listar a mesma coisa duas vezes (um estado, um símbolo, uma
 * transição). Isso não muda o autômato, então só removemos as repetições.
 */
static void remover_duplicatas(Automato *af) {
    remover_nomes_repetidos(af->estados, &af->n_estados);
    remover_nomes_repetidos(af->estados_finais, &af->n_finais);

    int unicos = 0;
    for (int i = 0; i < af->n_alfabeto; i++) {
        bool repetido = false;
        for (int j = 0; j < unicos; j++) {
            if (af->alfabeto[j] == af->alfabeto[i]) {
                repetido = true;
            }
        }
        if (!repetido) {
            af->alfabeto[unicos] = af->alfabeto[i];
            unicos++;
        }
    }
    af->n_alfabeto = unicos;

    unicos = 0;
    for (int i = 0; i < af->n_transicoes; i++) {
        Transicao atual = af->transicoes[i];
        bool repetida = false;
        for (int j = 0; j < unicos; j++) {
            Transicao anterior = af->transicoes[j];
            if (anterior.simbolo == atual.simbolo &&
                strcmp(anterior.origem, atual.origem) == 0 &&
                strcmp(anterior.destino, atual.destino) == 0) {
                repetida = true;
            }
        }
        if (!repetida) {
            af->transicoes[unicos] = atual;
            unicos++;
        }
    }
    af->n_transicoes = unicos;
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
        snprintf(erro, erro_tam, "%s: autômato vazio", ctx);
        return false;
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
            /* Campo extra (ex.: "nome", "tipo"): ignora o valor. */
            if (!pular_valor(p)) {
                snprintf(erro, erro_tam, "%s: valor inválido no campo '%s'",
                         ctx, chave);
                return false;
            }
        }

        pular_espacos(p);
        if (**p == ',') {
            (*p)++;
            continue;
        }
        if (**p == '}') {
            (*p)++;
            break;
        }
        snprintf(erro, erro_tam, "%s: JSON malformado no objeto", ctx);
        return false;
    }

    /* estados-finais e transicoes podem faltar; os outros são obrigatórios */
    if (!tem_alfabeto) {
        snprintf(erro, erro_tam, "%s: campo 'alfabeto' ausente", ctx);
        return false;
    }
    if (!tem_estados) {
        snprintf(erro, erro_tam, "%s: campo 'estados' ausente", ctx);
        return false;
    }
    if (!tem_inicial) {
        snprintf(erro, erro_tam, "%s: campo 'estado-inicial' ausente", ctx);
        return false;
    }
    return true;
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
    /* Pula BOM UTF-8 (arquivos salvos por alguns editores no Windows). */
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB &&
        (unsigned char)p[2] == 0xBF) {
        p += 3;
    }
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
            /* Chave extra na raiz: ignora o valor. */
            if (!pular_valor(&p)) {
                snprintf(erro, erro_tam, "JSON inválido no campo '%s'", chave);
                goto fim;
            }
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

    /* Depois do objeto raiz só pode haver espaços em branco. */
    pular_espacos(&p);
    if (*p != '\0') {
        snprintf(erro, erro_tam,
                 "JSON inválido: conteúdo extra após o objeto raiz");
        goto fim;
    }

    if (!tem_af1) {
        snprintf(erro, erro_tam, "JSON sem automato1");
        goto fim;
    }

    remover_duplicatas(af1);
    if (*af2_presente) {
        remover_duplicatas(af2);
    }
    if (resultado_presente != NULL && *resultado_presente) {
        remover_duplicatas(af_resultado);
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
            switch (c) {
            case '"':
                fputs("\\\"", f);
                break;
            case '\\':
                fputs("\\\\", f);
                break;
            case '\b':
                fputs("\\b", f);
                break;
            case '\f':
                fputs("\\f", f);
                break;
            case '\n':
                fputs("\\n", f);
                break;
            case '\r':
                fputs("\\r", f);
                break;
            case '\t':
                fputs("\\t", f);
                break;
            default:
                if (c < 0x20 || c == 0x7F) {
                    fprintf(f, "\\u%04X", (unsigned)c);
                } else {
                    fputc((char)c, f);
                }
                break;
            }
        }
    }
    fputc('"', f);
}

/* Escreve um símbolo de um caractere como string JSON (com escape correto). */
static void fprint_simbolo_json(FILE *f, char simbolo) {
    char s[2] = {simbolo, '\0'};
    fprint_string_json(f, s);
}

static void escrever_af(FILE *f, const Automato *af, const char *nome,
                        int indent) {
    fprintf(f, "%*s", indent, "");
    fputc('"', f);
    fputs(nome, f);
    fputs("\": {\n", f);

    fprintf(f, "%*s\"alfabeto\": [", indent + 2, "");
    for (int i = 0; i < af->n_alfabeto; i++) {
        fprint_simbolo_json(f, af->alfabeto[i]);
        fputs(i + 1 < af->n_alfabeto ? ", " : "", f);
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
        fputs(", ", f);
        fprint_simbolo_json(f, t->simbolo);
        fprintf(f, "]%s\n", i + 1 < af->n_transicoes ? "," : "");
    }
    fprintf(f, "%*s]\n", indent + 2, "");
    fprintf(f, "%*s}", indent, "");
}

static bool validar_para_exportar(const Automato *af, const char *nome,
                                  char *erro, int erro_tam) {
    char msg[256];
    if (!automato_validar(af, msg, sizeof(msg))) {
        snprintf(erro, erro_tam, "%s: %s", nome, msg);
        return false;
    }
    return true;
}

bool exportar_json(const Automato *af1, const Automato *af2, bool tem_af2,
                   const Automato *resultado, bool tem_resultado,
                   const char *caminho, char *erro, int erro_tam) {
    if (!validar_para_exportar(af1, "automato1", erro, erro_tam)) {
        return false;
    }
    if (tem_af2 && !validar_para_exportar(af2, "automato2", erro, erro_tam)) {
        return false;
    }
    if (tem_resultado &&
        !validar_para_exportar(resultado, "resultado", erro, erro_tam)) {
        return false;
    }

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
    if (!validar_para_exportar(af, nome_campo, erro, erro_tam)) {
        return false;
    }

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
