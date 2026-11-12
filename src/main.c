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

static void mostrar_menu(void) {
    printf("\n======== Trabalho LFA — Autômatos (C) ========\n");
    printf("AF1: %s | AF2: %s | Resultado: %s\n",
           tem_af1 ? "carregado" : "vazio",
           tem_af2 ? "carregado" : "vazio",
           tem_resultado ? "carregado" : "vazio");
    printf("----------------------------------------------\n");
    printf(" 1) Carregar JSON (AF1 / AF2)\n");
    printf(" 2) Converter AFN múltiplo-estado → AFN-λ\n");
    printf(" 3) Converter AFN-λ → AFN\n");
    printf(" 4) Converter AFN → AFD\n");
    printf(" 5) Minimizar AFD\n");
    printf(" 6) Testar palavras (terminal)\n");
    printf(" 7) Testar palavras (arquivo)\n");
    printf(" 8) União\n");
    printf(" 9) Interseção\n");
    printf("10) Subtração\n");
    printf("11) Complemento\n");
    printf("12) Exportar\n");
    printf("13) Mostrar autômatos em memória\n");
    printf(" 0) Sair\n");
    printf("Escolha: ");
}

static bool precisa_af1(void) {
    if (!tem_af1) {
        printf("Carregue um JSON com automato1 primeiro.\n");
        return false;
    }
    return true;
}

static bool precisa_dois_afds(void) {
    if (!tem_af1 || !tem_af2) {
        printf("Essa operação precisa de AF1 e AF2 carregados.\n");
        return false;
    }
    return true;
}

static const Automato *escolher_automato_unario(void) {
    int opcoes = 0;
    if (tem_af1) {
        opcoes++;
    }
    if (tem_af2) {
        opcoes++;
    }
    if (tem_resultado) {
        opcoes++;
    }

    if (opcoes == 0) {
        printf("Nenhum autômato carregado. Use a opção 1 primeiro.\n");
        return NULL;
    }
    if (opcoes == 1) {
        if (tem_resultado) {
            return &af_resultado;
        }
        if (tem_af2) {
            return &af2;
        }
        return &af1;
    }

    printf("\nQual autômato usar?\n");
    if (tem_af1) {
        printf(" 1) AF1\n");
    }
    if (tem_af2) {
        printf(" 2) AF2\n");
    }
    if (tem_resultado) {
        printf(" 3) AF Resultado\n");
    }
    printf("Número: ");

    int num = -1;
    if (scanf("%d", &num) != 1) {
        limpar_entrada();
        printf("Número inválido.\n");
        return NULL;
    }
    limpar_entrada();

    if (num == 1 && tem_af1) {
        return &af1;
    }
    if (num == 2 && tem_af2) {
        return &af2;
    }
    if (num == 3 && tem_resultado) {
        return &af_resultado;
    }

    printf("Opção inválida.\n");
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
    if (slot < 1 || slot > 4) {
        printf("Opção inválida (use 1–4 ou 0).\n");
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
        return;
    }
    af_resultado = saida;
    tem_resultado = true;
    printf("[%s] ok — resultado em AF Resultado.\n", nome);
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

    printf("Palavra (use ε ou <vazio> para palavra vazia; linha vazia encerra): ");
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
    printf("Caminho do arquivo de palavras: ");
    if (!ler_linha(caminho, sizeof(caminho)) || caminho[0] == '\0') {
        printf("Caminho vazio.\n");
        return;
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
    if (!precisa_af1()) {
        return;
    }
    char caminho[512];
    char erro[256];
    printf("Caminho de saída JSON: ");
    if (!ler_linha(caminho, sizeof(caminho)) || caminho[0] == '\0') {
        printf("Caminho vazio.\n");
        return;
    }

    if (!exportar_json(&af1, &af2, tem_af2, &af_resultado, tem_resultado,
                       caminho, erro, sizeof(erro))) {
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
