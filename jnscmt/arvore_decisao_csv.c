/*
 * Árvore de Decisão com Entropia (ID3) - versão simples com CSV
 * Estruturas de Dados / Programação 2 - ECOM008 - UFAL
 *
 * Compilar: gcc arvore_decisao_csv.c -o arvore -lm
 * Executar: ./arvore jogar_tenis.csv
 *
 * Formato do CSV:
 *   - primeira linha: nomes das colunas
 *   - demais linhas: os exemplos
 *   - a ÚLTIMA coluna é a classe (o que queremos prever)
 *   - todos os valores são categorias (texto) e nenhuma célula pode ficar vazia
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_EXEMPLOS 1000
#define MAX_COLUNAS  10
#define MAX_VALORES  10      /* máximo de valores distintos por coluna */
#define TAM_TEXTO    30

/* ---------------- Dados (preenchidos a partir do CSV) ---------------- */
char nomes_colunas[MAX_COLUNAS][TAM_TEXTO];
char tabela[MAX_EXEMPLOS][MAX_COLUNAS][TAM_TEXTO];
int numero_exemplos = 0;
int numero_atributos = 0;     /* colunas sem contar a classe */
int coluna_classe = 0;        /* índice da última coluna */

/* ---------------- Árvore ---------------- */
typedef struct No {
    int coluna;                              /* atributo testado (-1 se for folha) */
    char classe[TAM_TEXTO];                  /* classe prevista (usada nas folhas) */
    int quantidade_filhos;
    char valores[MAX_VALORES][TAM_TEXTO];    /* valores possíveis do atributo */
    struct No *filhos[MAX_VALORES];          /* filhos[i] corresponde a valores[i] */
} No;

/* ---------------- Leitura do CSV ---------------- */

/* Tira espaços e quebras de linha das pontas; retorna o início do texto limpo */
char *limpar_texto(char *texto) {
    while (*texto == ' ' || *texto == '\t') texto++;
    int tamanho = strlen(texto);
    while (tamanho > 0 && (texto[tamanho - 1] == '\n' || texto[tamanho - 1] == '\r' ||
                           texto[tamanho - 1] == ' '  || texto[tamanho - 1] == '\t'))
        texto[--tamanho] = '\0';
    return texto;
}

/* Separa a linha pelas vírgulas e guarda cada campo em 'campos'; retorna quantos */
int dividir_linha(char *linha, char campos[][TAM_TEXTO]) {
    int quantidade = 0;
    char *pedaco = strtok(linha, ",");
    while (pedaco != NULL && quantidade < MAX_COLUNAS) {
        strncpy(campos[quantidade], limpar_texto(pedaco), TAM_TEXTO - 1);
        campos[quantidade][TAM_TEXTO - 1] = '\0';
        quantidade++;
        pedaco = strtok(NULL, ",");
    }
    return quantidade;
}

/* Lê o arquivo para 'nomes_colunas' e 'tabela'; retorna 1 se deu certo */
int carregar_csv(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) return 0;

    char linha[512];
    int lendo_cabecalho = 1;

    while (fgets(linha, sizeof(linha), arquivo) != NULL && numero_exemplos < MAX_EXEMPLOS) {
        if (strlen(limpar_texto(linha)) == 0) continue;     /* ignora linhas vazias */

        if (lendo_cabecalho) {
            int colunas = dividir_linha(linha, nomes_colunas);
            numero_atributos = colunas - 1;
            coluna_classe = colunas - 1;
            lendo_cabecalho = 0;
        } else {
            dividir_linha(linha, tabela[numero_exemplos]);
            numero_exemplos++;
        }
    }
    fclose(arquivo);
    return numero_exemplos > 0 && numero_atributos > 0;
}

/* ---------------- Funções auxiliares ---------------- */

/* Preenche 'valores' com os valores distintos da coluna e retorna a quantidade */
int listar_valores(int coluna, char valores[][TAM_TEXTO]) {
    int quantidade = 0;
    for (int i = 0; i < numero_exemplos; i++) {
        int ja_existe = 0;
        for (int j = 0; j < quantidade; j++)
            if (strcmp(valores[j], tabela[i][coluna]) == 0) ja_existe = 1;

        if (!ja_existe) {
            if (quantidade == MAX_VALORES) {
                printf("Erro: a coluna %s tem valores demais (aumente MAX_VALORES)\n", nomes_colunas[coluna]);
                exit(1);
            }
            strcpy(valores[quantidade++], tabela[i][coluna]);
        }
    }
    return quantidade;
}

/* Copia para 'saida' os exemplos cuja coluna tem o valor dado; retorna quantos */
int filtrar_exemplos(int exemplos[], int quantidade, int coluna, const char *valor, int saida[]) {
    int total = 0;
    for (int i = 0; i < quantidade; i++)
        if (strcmp(tabela[exemplos[i]][coluna], valor) == 0)
            saida[total++] = exemplos[i];
    return total;
}

/* ---------------- Entropia e ganho ---------------- */

/* H(S) = - soma( p * log2(p) ) para cada classe */
double calcular_entropia(int exemplos[], int quantidade) {
    char classes[MAX_VALORES][TAM_TEXTO];
    int numero_classes = listar_valores(coluna_classe, classes);
    int da_classe[MAX_EXEMPLOS];
    double entropia = 0.0;

    for (int c = 0; c < numero_classes; c++) {
        int contagem = filtrar_exemplos(exemplos, quantidade, coluna_classe, classes[c], da_classe);
        if (contagem > 0) {
            double proporcao = (double)contagem / quantidade;
            entropia -= proporcao * log2(proporcao);
        }
    }
    return entropia;
}

/* Ganho = entropia antes da divisão - média ponderada da entropia depois */
double calcular_ganho(int exemplos[], int quantidade, int coluna) {
    char valores[MAX_VALORES][TAM_TEXTO];
    int numero_valores = listar_valores(coluna, valores);
    int subconjunto[MAX_EXEMPLOS];
    double entropia_depois = 0.0;

    for (int v = 0; v < numero_valores; v++) {
        int tamanho = filtrar_exemplos(exemplos, quantidade, coluna, valores[v], subconjunto);
        if (tamanho > 0)
            entropia_depois += ((double)tamanho / quantidade) * calcular_entropia(subconjunto, tamanho);
    }
    return calcular_entropia(exemplos, quantidade) - entropia_depois;
}

/* Devolve a classe que mais aparece nos exemplos (copiada para 'resultado') */
void classe_majoritaria(int exemplos[], int quantidade, char resultado[]) {
    char classes[MAX_VALORES][TAM_TEXTO];
    int numero_classes = listar_valores(coluna_classe, classes);
    int da_classe[MAX_EXEMPLOS];
    int maior_contagem = -1;

    for (int c = 0; c < numero_classes; c++) {
        int contagem = filtrar_exemplos(exemplos, quantidade, coluna_classe, classes[c], da_classe);
        if (contagem > maior_contagem) {
            maior_contagem = contagem;
            strcpy(resultado, classes[c]);
        }
    }
}

/* ---------------- Construção (ID3) ---------------- */

No *criar_no(void) {
    No *no = calloc(1, sizeof(No));
    no->coluna = -1;
    return no;
}

/* atributo_usado[a] = 1 se o atributo a já foi testado neste caminho */
No *construir_arvore(int exemplos[], int quantidade, int atributo_usado[]) {
    No *no = criar_no();
    classe_majoritaria(exemplos, quantidade, no->classe);

    /* Caso base 1: todos os exemplos têm a mesma classe */
    if (calcular_entropia(exemplos, quantidade) == 0.0) return no;

    /* Escolhe o atributo com maior ganho de informação */
    int melhor_atributo = -1;
    double maior_ganho = 0.0;
    for (int a = 0; a < numero_atributos; a++) {
        if (atributo_usado[a]) continue;
        double ganho = calcular_ganho(exemplos, quantidade, a);
        if (ganho > maior_ganho) {
            maior_ganho = ganho;
            melhor_atributo = a;
        }
    }

    /* Caso base 2: nenhum atributo ajuda mais */
    if (melhor_atributo == -1) return no;

    no->coluna = melhor_atributo;
    no->quantidade_filhos = listar_valores(melhor_atributo, no->valores);
    atributo_usado[melhor_atributo] = 1;

    for (int v = 0; v < no->quantidade_filhos; v++) {
        int subconjunto[MAX_EXEMPLOS];
        int tamanho = filtrar_exemplos(exemplos, quantidade, melhor_atributo, no->valores[v], subconjunto);

        if (tamanho == 0) {   /* valor sem exemplos: folha com a classe majoritária do pai */
            no->filhos[v] = criar_no();
            strcpy(no->filhos[v]->classe, no->classe);
        } else {
            no->filhos[v] = construir_arvore(subconjunto, tamanho, atributo_usado);
        }
    }

    atributo_usado[melhor_atributo] = 0;   /* libera o atributo para outros ramos */
    return no;
}

/* ---------------- Uso da árvore ---------------- */

/* Desce pela árvore seguindo os valores do exemplo até chegar a uma folha */
const char *classificar(No *no, char exemplo[][TAM_TEXTO]) {
    while (no->coluna != -1) {
        No *proximo = NULL;
        for (int i = 0; i < no->quantidade_filhos; i++)
            if (strcmp(no->valores[i], exemplo[no->coluna]) == 0)
                proximo = no->filhos[i];
        if (proximo == NULL) return no->classe;   /* valor desconhecido */
        no = proximo;
    }
    return no->classe;
}

void imprimir_arvore(No *no, int profundidade) {
    if (no->coluna == -1) {
        printf("=> %s\n", no->classe);
        return;
    }
    printf("%s?\n", nomes_colunas[no->coluna]);
    for (int i = 0; i < no->quantidade_filhos; i++) {
        for (int espaco = 0; espaco <= profundidade; espaco++) printf("    ");
        printf("[%s] ", no->valores[i]);
        imprimir_arvore(no->filhos[i], profundidade + 1);
    }
}

void liberar_arvore(No *no) {
    for (int i = 0; i < no->quantidade_filhos; i++)
        liberar_arvore(no->filhos[i]);
    free(no);
}

/* Pergunta o valor de cada atributo; retorna 0 se o usuário digitar linha vazia */
int ler_novo_exemplo(char exemplo[][TAM_TEXTO]) {
    char entrada[100];
    for (int a = 0; a < numero_atributos; a++) {
        printf("%s: ", nomes_colunas[a]);
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) return 0;
        char *texto = limpar_texto(entrada);
        if (strlen(texto) == 0) return 0;
        strncpy(exemplo[a], texto, TAM_TEXTO - 1);
        exemplo[a][TAM_TEXTO - 1] = '\0';
    }
    return 1;
}

/* ---------------- main ---------------- */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s arquivo.csv\n", argv[0]);
        return 1;
    }
    if (!carregar_csv(argv[1])) {
        printf("Erro ao ler o arquivo %s\n", argv[1]);
        return 1;
    }

    int exemplos[MAX_EXEMPLOS];
    for (int i = 0; i < numero_exemplos; i++) exemplos[i] = i;

    printf("Exemplos: %d | Atributos: %d\n", numero_exemplos, numero_atributos);
    printf("Entropia inicial H(S) = %.4f\n\n", calcular_entropia(exemplos, numero_exemplos));

    printf("Ganho de informacao de cada atributo na raiz:\n");
    for (int a = 0; a < numero_atributos; a++)
        printf("  %-12s %.4f\n", nomes_colunas[a], calcular_ganho(exemplos, numero_exemplos, a));

    int atributo_usado[MAX_COLUNAS] = {0};
    No *raiz = construir_arvore(exemplos, numero_exemplos, atributo_usado);

    printf("\nArvore de decisao:\n");
    imprimir_arvore(raiz, 0);

    /* Testa a árvore nos próprios exemplos de treino */
    int acertos = 0;
    for (int i = 0; i < numero_exemplos; i++)
        if (strcmp(classificar(raiz, tabela[i]), tabela[i][coluna_classe]) == 0) acertos++;
    printf("\nAcuracia no treino: %d/%d\n", acertos, numero_exemplos);

    /* Classifica exemplos novos digitados pelo usuário */
    printf("\nClassificar novo exemplo (deixe vazio para sair):\n");
    char novo_exemplo[MAX_COLUNAS][TAM_TEXTO];
    while (ler_novo_exemplo(novo_exemplo))
        printf("=> %s: %s\n\n", nomes_colunas[coluna_classe], classificar(raiz, novo_exemplo));

    liberar_arvore(raiz);
    return 0;
}
