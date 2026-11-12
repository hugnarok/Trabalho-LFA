#include "automato.h"
#include "funcoes.h"
#include "json_io.h"

#include <stdio.h>
#include <string.h>

static Automato af1;
static Automato af2;
static Automato af_resultado;
static bool tem_af1 = false;
static bool tem_af2 = false;
static bool tem_resultado = false;

static void acao_copiar_slots(void);

static void limpar_entrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

static void remover_cr(char *buf) {
    size_t n = strlen(buf);
    if (n > 0 && buf[n - 1] == '\r') {
        buf[n - 1] = '\0';
    }
}

static bool ler_linha(char *buf, int tam) {
    if (!fgets(buf, tam, stdin)) {
        buf[0] = '\0';
        return false;
    }
    size_t n = strlen(buf);
    bool linha_cheia = (n == (size_t)(tam - 1) && buf[n - 1] != '\n');
    if (n > 0 && buf[n - 1] == '\n') {
        buf[n - 1] = '\0';
    }
    remover_cr(buf);
    if (linha_cheia) {
        int c;
        bool resto = false;
        while ((c = getchar()) != '\n' && c != EOF) {
            resto = true;
        }
        if (resto) {
            printf("Aviso: linha truncada (máx. %d caracteres).\n", tam - 1);
        }
    }
    return true;
}

static bool palavra_vazia_especial(const char *s) {
    return strcmp(s, "ε") == 0 || strcmp(s, "<vazio>") == 0 ||
           strcmp(s, "<epsilon>") == 0 || strcmp(s, "<e>") == 0;
}

static void copiar_palavra_teste(const char *entrada, char *palavra, int tam) {
    if (palavra_vazia_especial(entrada)) {
        palavra[0] = '\0';
        return;
    }
    snprintf(palavra, (size_t)tam, "%s", entrada);
}

static void resumo_automato(const Automato *af, bool carregado, char *buf,
                            int tam) {
    if (!carregado || af->n_estados <= 0) {
        snprintf(buf, (size_t)tam, "vazio");
        return;
    }
    const char *tipo;
    if (automato_tem_lambda(af)) {
        tipo = "AFN-λ";
    } else if (automato_eh_deterministico(af)) {
        tipo = "AFD";
    } else {
        tipo = "AFN";
    }
    snprintf(buf, (size_t)tam, "%s · %d est. · |Σ|=%d", tipo, af->n_estados,
             af->n_alfabeto);
}

static void mostrar_menu(void) {
    char r1[64], r2[64], r3[64];
    resumo_automato(&af1, tem_af1, r1, (int)sizeof(r1));
    resumo_automato(&af2, tem_af2, r2, (int)sizeof(r2));
    resumo_automato(&af_resultado, tem_resultado, r3, (int)sizeof(r3));

    printf("\n======== Trabalho LFA — Autômatos (C) ========\n");
    printf("  AF1:        %s\n", r1);
    printf("  AF2:        %s\n", r2);
    printf("  Resultado:  %s\n", r3);
    printf("----------------------------------------------\n");
    printf(" 1) Carregar JSON\n");
    printf(" 2) Converter AFN múltiplo-estado → AFN-λ\n");
    printf(" 3) Converter AFN-λ → AFN\n");
    printf(" 4) Converter AFN → AFD\n");
    printf(" 5) Minimizar AFD\n");
    printf(" 6) Testar palavras (terminal)\n");
    printf(" 7) Testar palavras (arquivo)\n");
    printf(" 8) União\n");
    printf(" 9) Interseção\n");
    printf(" 10) Subtração\n");
    printf(" 11) Complemento\n");
    printf(" 12) Exportar\n");
    printf(" 13) Mostrar autômatos em memória\n");
    printf(" 0) Sair\n");
    printf("Escolha: ");
}

static bool precisa_dois_afds(void) {
    if (!tem_af1 || !tem_af2) {
        printf("Essa operação precisa de AF1 e AF2 carregados.\n");
        return false;
    }
    return true;
}

static const Automato *escolher_automato_unario(void) {
    if (!tem_af1 && !tem_af2 && !tem_resultado) {
        printf("Nenhum autômato carregado. Use a opção 1 primeiro.\n");
        return NULL;
    }

    char r1[64], r2[64], r3[64];
    resumo_automato(&af1, tem_af1, r1, (int)sizeof(r1));
    resumo_automato(&af2, tem_af2, r2, (int)sizeof(r2));
    resumo_automato(&af_resultado, tem_resultado, r3, (int)sizeof(r3));

    printf("\nQual autômato usar na operação?\n");
    printf(" 1) AF1 — %s\n", r1);
    printf(" 2) AF2 — %s\n", r2);
    printf(" 3) AF Resultado — %s\n", r3);
    printf("Número: ");

    int num = -1;
    if (scanf("%d", &num) != 1) {
        limpar_entrada();
        printf("Número inválido.\n");
        return NULL;
    }
    limpar_entrada();

    if (num == 1) {
        if (!tem_af1) {
            printf("AF1 está vazio.\n");
            return NULL;
        }
        return &af1;
    }
    if (num == 2) {
        if (!tem_af2) {
            printf("AF2 está vazio.\n");
            return NULL;
        }
        return &af2;
    }
    if (num == 3) {
        if (!tem_resultado) {
            printf("AF Resultado está vazio.\n");
            return NULL;
        }
        return &af_resultado;
    }

    printf("Opção inválida (use 1, 2 ou 3).\n");
    return NULL;
}

typedef struct {
    const char *nome;
    const char *caminho;
} AutomatoExemplo;

static const AutomatoExemplo AUTOMATOS_EXEMPLO[] = {
    {"AFD simples", "data/afd_simples.json"},
    {"AFD parcial (complemento)", "data/afd_parcial.json"},
    {"AFN-λ", "data/afn_lambda.json"},
    {"AFN não determinístico", "data/afn_nao_deterministico.json"},
    {"Dois AFDs (união/interseção)", "data/dois_afds.json"},
    {"AFDs com alfabetos diferentes", "data/afds_alfabetos_diferentes.json"},
};
static const int N_AUTOMATOS_EXEMPLO =
    (int)(sizeof(AUTOMATOS_EXEMPLO) / sizeof(AUTOMATOS_EXEMPLO[0]));

typedef enum {
    FONTE_ARQUIVO_JSON = 0,
    FONTE_MEM_AF1 = 1,
    FONTE_MEM_AF2 = 2,
    FONTE_MEM_RESULTADO = 3
} FonteCarregamento;

#define OPC_MEM_AF1 7
#define OPC_MEM_AF2 8
#define OPC_MEM_RESULTADO 9

static bool escolher_fonte_carregamento(FonteCarregamento *fonte, char *caminho,
                                          int tam) {
    printf("\nOrigem do autômato:\n");
    printf("--- Arquivos JSON ---\n");
    for (int i = 0; i < N_AUTOMATOS_EXEMPLO; i++) {
        printf("  %d) %s\n", i + 1, AUTOMATOS_EXEMPLO[i].nome);
    }
    printf("  0) Digitar outro caminho\n");
    printf("--- Já em memória ---\n");
    printf("  %d) AF Resultado%s\n", OPC_MEM_RESULTADO,
           tem_resultado ? "" : " (vazio)");
    printf("  %d) AF1%s\n", OPC_MEM_AF1, tem_af1 ? "" : " (vazio)");
    printf("  %d) AF2%s\n", OPC_MEM_AF2, tem_af2 ? "" : " (vazio)");
    printf("Número: ");

    int num = -1;
    if (scanf("%d", &num) != 1) {
        limpar_entrada();
        printf("Número inválido.\n");
        return false;
    }
    limpar_entrada();

    if (num == OPC_MEM_RESULTADO) {
        if (!tem_resultado) {
            printf("AF Resultado está vazio. Faça uma conversão/operação "
                   "antes.\n");
            return false;
        }
        *fonte = FONTE_MEM_RESULTADO;
        caminho[0] = '\0';
        return true;
    }
    if (num == OPC_MEM_AF1) {
        if (!tem_af1) {
            printf("AF1 está vazio.\n");
            return false;
        }
        *fonte = FONTE_MEM_AF1;
        caminho[0] = '\0';
        return true;
    }
    if (num == OPC_MEM_AF2) {
        if (!tem_af2) {
            printf("AF2 está vazio.\n");
            return false;
        }
        *fonte = FONTE_MEM_AF2;
        caminho[0] = '\0';
        return true;
    }

    if (num == 0) {
        printf("Caminho do JSON: ");
        if (!ler_linha(caminho, tam) || caminho[0] == '\0') {
            printf("Caminho vazio.\n");
            return false;
        }
        *fonte = FONTE_ARQUIVO_JSON;
        return true;
    }

    if (num < 1 || num > N_AUTOMATOS_EXEMPLO) {
        printf("Número fora da lista (1–%d, %d–%d ou 0).\n",
               N_AUTOMATOS_EXEMPLO, OPC_MEM_AF1, OPC_MEM_RESULTADO);
        return false;
    }

    snprintf(caminho, (size_t)tam, "%s", AUTOMATOS_EXEMPLO[num - 1].caminho);
    printf("Selecionado: %s (%s)\n", AUTOMATOS_EXEMPLO[num - 1].nome, caminho);
    *fonte = FONTE_ARQUIVO_JSON;
    return true;
}

static bool copiar_da_memoria(FonteCarregamento fonte, Automato *destino,
                              char *erro, int erro_tam) {
    const Automato *origem = NULL;
    const char *nome = NULL;

    if (fonte == FONTE_MEM_AF1) {
        origem = &af1;
        nome = "AF1";
    } else if (fonte == FONTE_MEM_AF2) {
        origem = &af2;
        nome = "AF2";
    } else if (fonte == FONTE_MEM_RESULTADO) {
        origem = &af_resultado;
        nome = "AF Resultado";
    } else {
        snprintf(erro, erro_tam, "fonte em memória inválida");
        return false;
    }

    if (!automato_validar(origem, erro, erro_tam)) {
        snprintf(erro, erro_tam, "%s: %s", nome, erro);
        return false;
    }

    *destino = *origem;
    return true;
}

static bool carregar_do_arquivo(const char *caminho, char *erro, int erro_tam,
                                Automato *novo1, Automato *novo2,
                                Automato *novo_res, bool *tem2, bool *tem_res) {
    return carregar_json(caminho, novo1, novo2, tem2, novo_res, tem_res, erro,
                         erro_tam);
}

typedef enum {
    JSON_BLOCO_AUTOMATO1 = 1,
    JSON_BLOCO_AUTOMATO2 = 2,
    JSON_BLOCO_RESULTADO = 3
} JsonBloco;

static const char *nome_bloco_json(JsonBloco bloco) {
    switch (bloco) {
    case JSON_BLOCO_AUTOMATO1:
        return "automato1";
    case JSON_BLOCO_AUTOMATO2:
        return "automato2";
    case JSON_BLOCO_RESULTADO:
        return "resultado";
    default:
        return "?";
    }
}

static bool escolher_bloco_json(bool tem2, bool tem_res, JsonBloco *bloco) {
    if (!tem2 && !tem_res) {
        *bloco = JSON_BLOCO_AUTOMATO1;
        printf("\n(Este JSON só tem automato1 — será usado automaticamente.)\n");
        return true;
    }

    printf("\nO arquivo tem várias partes. Qual delas copiar para o slot "
           "escolhido (AF1/AF2/Resultado)?\n");
    printf(" 1) automato1 — primeiro autômato do arquivo\n");
    if (tem2) {
        printf(" 2) automato2 — segundo autômato (união/interseção etc.)\n");
    }
    if (tem_res) {
        printf(" 3) resultado — autômato exportado após conversão/operação\n");
    }
    printf(" 0) Cancelar\n");
    printf("Escolha: ");

    int op = -1;
    if (scanf("%d", &op) != 1) {
        limpar_entrada();
        printf("Opção inválida.\n");
        return false;
    }
    limpar_entrada();

    if (op == 0) {
        return false;
    }
    if (op == 1) {
        *bloco = JSON_BLOCO_AUTOMATO1;
        return true;
    }
    if (op == 2) {
        if (!tem2) {
            printf("Este arquivo não contém automato2.\n");
            return false;
        }
        *bloco = JSON_BLOCO_AUTOMATO2;
        return true;
    }
    if (op == 3) {
        if (!tem_res) {
            printf("Este arquivo não contém resultado.\n");
            return false;
        }
        *bloco = JSON_BLOCO_RESULTADO;
        return true;
    }

    printf("Opção inválida.\n");
    return false;
}

static bool copiar_bloco_json(const Automato *novo1, const Automato *novo2,
                              const Automato *novo_res, JsonBloco bloco,
                              bool tem2, bool tem_res, Automato *destino,
                              char *erro, int erro_tam) {
    const Automato *origem = NULL;

    if (bloco == JSON_BLOCO_AUTOMATO1) {
        origem = novo1;
    } else if (bloco == JSON_BLOCO_AUTOMATO2) {
        if (!tem2) {
            snprintf(erro, erro_tam, "arquivo sem automato2");
            return false;
        }
        origem = novo2;
    } else if (bloco == JSON_BLOCO_RESULTADO) {
        if (!tem_res) {
            snprintf(erro, erro_tam, "arquivo sem resultado");
            return false;
        }
        origem = novo_res;
    } else {
        snprintf(erro, erro_tam, "bloco JSON inválido");
        return false;
    }

    *destino = *origem;
    return true;
}

static void acao_carregar(void) {
    printf("\n--- Carregar JSON ---\n");
    printf(" 1) Carregar no AF1\n");
    printf(" 2) Carregar no AF2\n");
    printf(" 3) Carregar no AF Resultado\n");
    printf(" 4) Carregar AF1, AF2 e Resultado do mesmo arquivo (carga "
           "completa)\n");
    printf(" 5) Copiar entre slots (ex.: Resultado → AF1)\n");
    printf(" 0) Voltar\n");
    printf("Escolha: ");

    int slot = -1;
    if (scanf("%d", &slot) != 1) {
        limpar_entrada();
        if (feof(stdin)) {
            printf("Entrada encerrada.\n");
        } else {
            printf("Opção inválida.\n");
        }
        return;
    }
    limpar_entrada();

    if (slot == 0) {
        return;
    }
    if (slot < 1 || slot > 5) {
        printf("Opção inválida (use 1–5 ou 0).\n");
        return;
    }

    if (slot == 5) {
        acao_copiar_slots();
        return;
    }

    FonteCarregamento fonte;
    char caminho[512];
    char erro[256];
    if (slot == 4) {
        printf("\nCarga completa: escolha o arquivo JSON.\n");
        fonte = FONTE_ARQUIVO_JSON;
        if (!escolher_fonte_carregamento(&fonte, caminho, sizeof(caminho))) {
            return;
        }
        if (fonte != FONTE_ARQUIVO_JSON) {
            printf("Carga completa só funciona com arquivo JSON.\n");
            return;
        }
    } else if (!escolher_fonte_carregamento(&fonte, caminho, sizeof(caminho))) {
        return;
    }

    Automato *destino = NULL;
    const char *nome_slot = NULL;
    if (slot == 1) {
        destino = &af1;
        nome_slot = "AF1";
        tem_af1 = true;
    } else if (slot == 2) {
        destino = &af2;
        nome_slot = "AF2";
        tem_af2 = true;
    } else if (slot == 3) {
        destino = &af_resultado;
        nome_slot = "AF Resultado";
        tem_resultado = true;
    }

    if (slot != 4 && fonte != FONTE_ARQUIVO_JSON) {
        const char *nome_fonte = fonte == FONTE_MEM_AF1      ? "AF1"
                                 : fonte == FONTE_MEM_AF2    ? "AF2"
                                                             : "AF Resultado";
        if (!copiar_da_memoria(fonte, destino, erro, sizeof(erro))) {
            printf("Erro: %s\n", erro);
            return;
        }
        printf("%s atualizado com cópia de %s (memória).\n", nome_slot,
               nome_fonte);
        return;
    }

    Automato novo1, novo2, novo_res;
    bool tem2 = false;
    bool tem_res = false;
    if (!carregar_do_arquivo(caminho, erro, sizeof(erro), &novo1, &novo2,
                             &novo_res, &tem2, &tem_res)) {
        printf("Erro: %s\n", erro);
        return;
    }

    if (slot == 4) {
        af1 = novo1;
        tem_af1 = true;
        if (tem2) {
            af2 = novo2;
            tem_af2 = true;
        } else {
            automato_init(&af2);
            tem_af2 = false;
        }
        if (tem_res) {
            af_resultado = novo_res;
            tem_resultado = true;
        } else {
            automato_init(&af_resultado);
            tem_resultado = false;
        }
        printf("JSON carregado. AF1 ok%s%s.\n", tem_af2 ? ", AF2 ok" : "",
               tem_res ? ", Resultado ok" : "");
        return;
    }

    JsonBloco bloco;
    if (!escolher_bloco_json(tem2, tem_res, &bloco)) {
        return;
    }

    if (!copiar_bloco_json(&novo1, &novo2, &novo_res, bloco, tem2, tem_res,
                           destino, erro, sizeof(erro))) {
        printf("Erro: %s\n", erro);
        return;
    }

    printf("%s carregado (%s) de '%s'.\n", nome_slot, nome_bloco_json(bloco),
           caminho);
}

static void perguntar_copiar_resultado(void) {
    printf("\nCopiar AF Resultado para qual slot?\n");
    printf(" 1) AF1   2) AF2   3) AF1 e AF2   0) Não copiar\n");
    printf("Escolha: ");

    int op = -1;
    if (scanf("%d", &op) != 1) {
        limpar_entrada();
        return;
    }
    limpar_entrada();

    if (op == 0) {
        return;
    }
    if (op != 1 && op != 2 && op != 3) {
        printf("Opção inválida.\n");
        return;
    }

    char erro[256];
    bool copiou_af1 = false;
    bool copiou_af2 = false;

    if (op == 1 || op == 3) {
        if (!copiar_da_memoria(FONTE_MEM_RESULTADO, &af1, erro, sizeof(erro))) {
            printf("Erro ao copiar para AF1: %s\n", erro);
        } else {
            tem_af1 = true;
            copiou_af1 = true;
        }
    }
    if (op == 2 || op == 3) {
        if (!copiar_da_memoria(FONTE_MEM_RESULTADO, &af2, erro, sizeof(erro))) {
            printf("Erro ao copiar para AF2: %s\n", erro);
        } else {
            tem_af2 = true;
            copiou_af2 = true;
        }
    }

    if (copiou_af1 && copiou_af2) {
        printf("AF1 e AF2 atualizados com cópia do Resultado.\n");
    } else if (copiou_af1) {
        printf("AF1 atualizado com cópia do Resultado.\n");
    } else if (copiou_af2) {
        printf("AF2 atualizado com cópia do Resultado.\n");
    }
}

static void aplicar_unario(bool (*fn)(const Automato *, Automato *, char *, int),
                           const char *nome) {
    const Automato *entrada = escolher_automato_unario();
    if (entrada == NULL) {
        return;
    }
    char erro[256];
    Automato saida;
    if (!fn(entrada, &saida, erro, sizeof(erro))) {
        printf("[%s] %s\n", nome, erro);
        if (strstr(erro, "determin") != NULL ||
            strstr(erro, "AFD") != NULL) {
            printf("Dica: use a opção 4 (AFN → AFD) antes.\n");
        }
        return;
    }
    af_resultado = saida;
    tem_resultado = true;
    printf("[%s] ok — resultado em AF Resultado.\n", nome);
    perguntar_copiar_resultado();
}

static void aplicar_binario(
    bool (*fn)(const Automato *, const Automato *, Automato *, char *, int),
    const char *nome) {
    if (!precisa_dois_afds()) {
        return;
    }
    char erro[256];
    Automato saida;
    if (!fn(&af1, &af2, &saida, erro, sizeof(erro))) {
        printf("[%s] %s\n", nome, erro);
        return;
    }
    af_resultado = saida;
    tem_resultado = true;
    printf("[%s] ok — resultado em AF Resultado.\n", nome);
    perguntar_copiar_resultado();
}

static bool slot_ocupado(int slot) {
    if (slot == 1) {
        return tem_af1;
    }
    if (slot == 2) {
        return tem_af2;
    }
    if (slot == 3) {
        return tem_resultado;
    }
    return false;
}

static Automato *ponteiro_slot(int slot) {
    if (slot == 1) {
        return &af1;
    }
    if (slot == 2) {
        return &af2;
    }
    if (slot == 3) {
        return &af_resultado;
    }
    return NULL;
}

static void marcar_slot_carregado(int slot) {
    if (slot == 1) {
        tem_af1 = true;
    } else if (slot == 2) {
        tem_af2 = true;
    } else if (slot == 3) {
        tem_resultado = true;
    }
}

static const char *nome_slot(int slot) {
    if (slot == 1) {
        return "AF1";
    }
    if (slot == 2) {
        return "AF2";
    }
    return "AF Resultado";
}

static bool copiar_slot_para_slot(int origem_slot, int destino_slot, char *erro,
                                  int erro_tam) {
    if (origem_slot == destino_slot) {
        snprintf(erro, erro_tam, "origem e destino são o mesmo slot");
        return false;
    }
    if (!slot_ocupado(origem_slot)) {
        snprintf(erro, erro_tam, "%s está vazio", nome_slot(origem_slot));
        return false;
    }

    FonteCarregamento fonte = (FonteCarregamento)origem_slot;
    Automato *dest = ponteiro_slot(destino_slot);
    if (dest == NULL) {
        snprintf(erro, erro_tam, "slot destino inválido");
        return false;
    }
    if (!copiar_da_memoria(fonte, dest, erro, erro_tam)) {
        return false;
    }
    marcar_slot_carregado(destino_slot);
    return true;
}

static void acao_copiar_slots(void) {
    printf("\n--- Copiar entre slots ---\n");
    printf("Atalhos:\n");
    printf(" 1) Resultado → AF1\n");
    printf(" 2) Resultado → AF2\n");
    printf(" 3) AF1 → AF2\n");
    printf(" 4) Escolher origem e destino\n");
    printf(" 0) Voltar\n");
    printf("Escolha: ");

    int op = -1;
    if (scanf("%d", &op) != 1) {
        limpar_entrada();
        printf("Opção inválida.\n");
        return;
    }
    limpar_entrada();

    int origem = 0;
    int destino = 0;
    if (op == 0) {
        return;
    }
    if (op == 1) {
        origem = 3;
        destino = 1;
    } else if (op == 2) {
        origem = 3;
        destino = 2;
    } else if (op == 3) {
        origem = 1;
        destino = 2;
    } else if (op == 4) {
        printf("Origem: 1=AF1  2=AF2  3=Resultado: ");
        if (scanf("%d", &origem) != 1) {
            limpar_entrada();
            return;
        }
        printf("Destino: 1=AF1  2=AF2  3=Resultado: ");
        if (scanf("%d", &destino) != 1) {
            limpar_entrada();
            return;
        }
        limpar_entrada();
    } else {
        printf("Opção inválida.\n");
        return;
    }

    char erro[256];
    if (!copiar_slot_para_slot(origem, destino, erro, sizeof(erro))) {
        printf("Erro: %s\n", erro);
        return;
    }
    printf("%s atualizado com cópia de %s.\n", nome_slot(destino),
           nome_slot(origem));
}

static void acao_testar_terminal(void) {
    const Automato *af = escolher_automato_unario();
    if (af == NULL) {
        return;
    }
    char entrada[512];
    char palavra[512];
    char erro[256];
    bool aceita = false;

    char resumo[64];
    resumo_automato(af, true, resumo, (int)sizeof(resumo));
    printf("Autômato: %s\n", resumo);
    printf("Alfabeto: {");
    for (int i = 0; i < af->n_alfabeto; i++) {
        printf("%c%s", af->alfabeto[i], i + 1 < af->n_alfabeto ? ", " : "");
    }
    printf("}\n");
    printf("Palavra (ε ou <vazio> = vazia; linha vazia encerra): ");
    while (ler_linha(entrada, sizeof(entrada))) {
        if (entrada[0] == '\0') {
            break;
        }
        copiar_palavra_teste(entrada, palavra, sizeof(palavra));
        if (!testar_palavra(af, palavra, &aceita, erro, sizeof(erro))) {
            printf("Erro: %s\n", erro);
            break;
        }
        if (palavra[0] == '\0') {
            printf("ε → %s\n", aceita ? "ACEITA" : "REJEITA");
        } else {
            printf("%s → %s\n", palavra, aceita ? "ACEITA" : "REJEITA");
        }
        printf("Palavra: ");
    }
}

static void acao_testar_arquivo(void) {
    const Automato *af = escolher_automato_unario();
    if (af == NULL) {
        return;
    }
    char caminho[512];
    char erro[256];
    printf("Arquivo de palavras [Enter = data/palavras.txt]: ");
    if (!ler_linha(caminho, sizeof(caminho))) {
        return;
    }
    if (caminho[0] == '\0') {
        snprintf(caminho, sizeof(caminho), "data/palavras.txt");
        printf("Usando %s\n", caminho);
    }

    FILE *f = fopen(caminho, "r");
    if (!f) {
        printf("Não foi possível abrir '%s'.\n", caminho);
        return;
    }

    char linha[512];
    char palavra[512];
    while (fgets(linha, sizeof(linha), f)) {
        size_t n = strlen(linha);
        if (n > 0 && linha[n - 1] == '\n') {
            linha[n - 1] = '\0';
        }
        remover_cr(linha);
        copiar_palavra_teste(linha, palavra, sizeof(palavra));
        bool aceita = false;
        if (!testar_palavra(af, palavra, &aceita, erro, sizeof(erro))) {
            if (palavra[0] == '\0') {
                printf("Erro na palavra vazia: %s\n", erro);
            } else {
                printf("Erro em '%s': %s\n", palavra, erro);
            }
            break;
        }
        if (palavra[0] == '\0') {
            printf("ε → %s\n", aceita ? "ACEITA" : "REJEITA");
        } else {
            printf("%s → %s\n", palavra, aceita ? "ACEITA" : "REJEITA");
        }
    }
    fclose(f);
}

static void acao_exportar(void) {
    if (!tem_af1 && !tem_af2 && !tem_resultado) {
        printf("Nada para exportar. Carregue ou calcule um autômato primeiro.\n");
        return;
    }

    printf("\n--- Exportar JSON ---\n");
    printf(" 1) Arquivo completo (AF1 + AF2 + Resultado, se existirem)\n");
    printf(" 2) Só AF Resultado\n");
    printf(" 3) Só AF1\n");
    printf(" 4) Só AF2\n");
    printf(" 0) Voltar\n");
    printf("Escolha: ");

    int modo = -1;
    if (scanf("%d", &modo) != 1) {
        limpar_entrada();
        printf("Opção inválida.\n");
        return;
    }
    limpar_entrada();
    if (modo == 0) {
        return;
    }

    char caminho[512];
    char erro[256];
    printf("Caminho de saída JSON: ");
    if (!ler_linha(caminho, sizeof(caminho)) || caminho[0] == '\0') {
        printf("Caminho vazio.\n");
        return;
    }

    bool ok = false;
    if (modo == 1) {
        if (!tem_af1) {
            printf("Exportação completa exige pelo menos AF1 carregado.\n");
            return;
        }
        ok = exportar_json(&af1, &af2, tem_af2, &af_resultado, tem_resultado,
                            caminho, erro, sizeof(erro));
    } else if (modo == 2) {
        if (!tem_resultado) {
            printf("AF Resultado está vazio.\n");
            return;
        }
        ok = exportar_um_af(&af_resultado, "resultado", caminho, erro,
                              sizeof(erro));
    } else if (modo == 3) {
        if (!tem_af1) {
            printf("AF1 está vazio.\n");
            return;
        }
        ok = exportar_um_af(&af1, "automato1", caminho, erro, sizeof(erro));
    } else if (modo == 4) {
        if (!tem_af2) {
            printf("AF2 está vazio.\n");
            return;
        }
        ok = exportar_um_af(&af2, "automato2", caminho, erro, sizeof(erro));
    } else {
        printf("Opção inválida.\n");
        return;
    }

    if (!ok) {
        printf("Erro: %s\n", erro);
        return;
    }
    printf("Exportado para '%s'.\n", caminho);
}

static void acao_mostrar(void) {
    if (tem_af1) {
        printf("\n--- AF1 ---\n");
        automato_imprimir(&af1);
    }
    if (tem_af2) {
        printf("\n--- AF2 ---\n");
        automato_imprimir(&af2);
    }
    if (tem_resultado) {
        printf("\n--- AF Resultado ---\n");
        automato_imprimir(&af_resultado);
    }
    if (!tem_af1 && !tem_af2 && !tem_resultado) {
        printf("Nenhum autômato em memória.\n");
    }
}

int main(void) {
    automato_init(&af1);
    automato_init(&af2);
    automato_init(&af_resultado);

    int opcao = -1;
    while (opcao != 0) {
        mostrar_menu();
        if (scanf("%d", &opcao) != 1) {
            limpar_entrada();
            if (feof(stdin)) {
                printf("\nFim da entrada. Saindo.\n");
                opcao = 0;
                break;
            }
            printf("Opção inválida.\n");
            continue;
        }
        limpar_entrada();

        switch (opcao) {
        case 1:
            acao_carregar();
            break;
        case 2:
            aplicar_unario(afn_multi_para_afn_lambda, "AFN multi → AFN-λ");
            break;
        case 3:
            aplicar_unario(afn_lambda_para_afn, "AFN-λ → AFN");
            break;
        case 4:
            aplicar_unario(afn_para_afd, "AFN → AFD");
            break;
        case 5:
            aplicar_unario(minimizar_afd, "Minimizar AFD");
            break;
        case 6:
            acao_testar_terminal();
            break;
        case 7:
            acao_testar_arquivo();
            break;
        case 8:
            aplicar_binario(uniao, "União");
            break;
        case 9:
            aplicar_binario(intersecao, "Interseção");
            break;
        case 10:
            aplicar_binario(subtracao, "Subtração");
            break;
        case 11:
            aplicar_unario(complemento, "Complemento");
            break;
        case 12:
            acao_exportar();
            break;
        case 13:
            acao_mostrar();
            break;
        case 0:
            printf("Saindo.\n");
            break;
        default:
            printf("Opção inválida.\n");
            break;
        }
    }
    return 0;
}
