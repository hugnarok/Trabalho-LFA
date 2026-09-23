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

static void ler_linha(char *buf, int tam) {
    if (!fgets(buf, tam, stdin)) {
        buf[0] = '\0';
        return;
    }
    size_t n = strlen(buf);
    if (n > 0 && buf[n - 1] == '\n') {
        buf[n - 1] = '\0';
    }
}

static void mostrar_menu(void) {
    printf("\n======== Trabalho LFA — Autômatos (C) ========\n");
    printf("AF1: %s | AF2: %s | Resultado: %s\n",
           tem_af1 ? "carregado" : "vazio",
           tem_af2 ? "carregado" : "vazio",
           tem_resultado ? "carregado" : "vazio");
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

typedef struct {
    const char *nome;
    const char *caminho;
} AutomatoExemplo;

static const AutomatoExemplo AUTOMATOS_EXEMPLO[] = {
    {"AFD simples", "data/afd_simples.json"},
    {"AFN-λ", "data/afn_lambda.json"},
    {"AFN não determinístico", "data/afn_nao_deterministico.json"},
    {"Dois AFDs (união/interseção)", "data/dois_afds.json"},
};
static const int N_AUTOMATOS_EXEMPLO =
    (int)(sizeof(AUTOMATOS_EXEMPLO) / sizeof(AUTOMATOS_EXEMPLO[0]));

static bool escolher_caminho_json(char *caminho, int tam) {
    printf("\nAutômatos disponíveis:\n");
    for (int i = 0; i < N_AUTOMATOS_EXEMPLO; i++) {
        printf("  %d) %s\n", i + 1, AUTOMATOS_EXEMPLO[i].nome);
    }
    printf("  0) Digitar outro caminho\n");
    printf("Número do autômato: ");

    int num = -1;
    if (scanf("%d", &num) != 1) {
        limpar_entrada();
        printf("Número inválido.\n");
        return false;
    }
    limpar_entrada();

    if (num == 0) {
        printf("Caminho do JSON: ");
        ler_linha(caminho, tam);
        if (caminho[0] == '\0') {
            printf("Caminho vazio.\n");
            return false;
        }
        return true;
    }

    if (num < 1 || num > N_AUTOMATOS_EXEMPLO) {
        printf("Número fora da lista (1–%d ou 0).\n", N_AUTOMATOS_EXEMPLO);
        return false;
    }

    snprintf(caminho, (size_t)tam, "%s", AUTOMATOS_EXEMPLO[num - 1].caminho);
    printf("Selecionado: %s (%s)\n", AUTOMATOS_EXEMPLO[num - 1].nome, caminho);
    return true;
}

static void acao_carregar(void) {
    char caminho[512];
    char erro[256];

    if (!escolher_caminho_json(caminho, sizeof(caminho))) {
        return;
    }

    Automato novo1, novo2;
    bool tem2 = false;
    if (!carregar_json(caminho, &novo1, &novo2, &tem2, erro, sizeof(erro))) {
        printf("Erro: %s\n", erro);
        return;
    }

    af1 = novo1;
    tem_af1 = true;
    if (tem2) {
        af2 = novo2;
        tem_af2 = true;
    } else {
        automato_init(&af2);
        tem_af2 = false;
    }
    tem_resultado = false;
    printf("JSON carregado. AF1 ok%s.\n", tem_af2 ? ", AF2 ok" : "");
}

static void aplicar_unario(bool (*fn)(const Automato *, Automato *, char *, int),
                           const char *nome) {
    if (!precisa_af1()) {
        return;
    }
    char erro[256];
    Automato saida;
    if (!fn(&af1, &saida, erro, sizeof(erro))) {
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
    if (!precisa_af1()) {
        return;
    }
    char palavra[512];
    char erro[256];
    bool aceita = false;

    printf("Palavra (linha vazia encerra): ");
    while (1) {
        ler_linha(palavra, sizeof(palavra));
        if (palavra[0] == '\0') {
            break;
        }
        if (!testar_palavra(&af1, palavra, &aceita, erro, sizeof(erro))) {
            printf("Erro: %s\n", erro);
            break;
        }
        printf("%s → %s\n", palavra, aceita ? "ACEITA" : "REJEITA");
        printf("Palavra: ");
    }
}

static void acao_testar_arquivo(void) {
    if (!precisa_af1()) {
        return;
    }
    char caminho[512];
    char erro[256];
    printf("Caminho do arquivo de palavras: ");
    ler_linha(caminho, sizeof(caminho));

    FILE *f = fopen(caminho, "r");
    if (!f) {
        printf("Não foi possível abrir '%s'.\n", caminho);
        return;
    }

    char linha[512];
    while (fgets(linha, sizeof(linha), f)) {
        size_t n = strlen(linha);
        if (n > 0 && linha[n - 1] == '\n') {
            linha[n - 1] = '\0';
        }
        if (linha[0] == '\0') {
            continue;
        }
        bool aceita = false;
        if (!testar_palavra(&af1, linha, &aceita, erro, sizeof(erro))) {
            printf("Erro em '%s': %s\n", linha, erro);
            break;
        }
        printf("%s → %s\n", linha, aceita ? "ACEITA" : "REJEITA");
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
    ler_linha(caminho, sizeof(caminho));

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
