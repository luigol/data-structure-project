#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM 256   // quantidade de caracteres possiveis (tabela ASCII estendida)

/* ============ ESTRUTURAS ============ */
typedef struct no {
    unsigned char caracter;
    int frequencia;
    struct no *esq, *dir, *proximo;
} No;

typedef struct {
    No *inicio;
    int tam;
} Lista;

/* ============ PARTE 1: TABELA DE FREQUENCIA ============ */
void inicializa_tabela_com_zero(unsigned int tab[]) {
    for (int i = 0; i < TAM; i++)
        tab[i] = 0;
}

void preenche_tab_frequencia(unsigned char texto[], unsigned int tab[]) {
    for (int i = 0; texto[i] != '\0'; i++)
        tab[texto[i]]++;
}

/* ============ PARTE 2: LISTA ENCADEADA ORDENADA (fila de prioridade) ============ */
void criar_lista(Lista *lista) {
    lista->inicio = NULL;
    lista->tam = 0;
}

void inserir_ordenado(Lista *lista, No *no) {
    if (lista->inicio == NULL) {                       // lista vazia
        lista->inicio = no;
    } else if (no->frequencia < lista->inicio->frequencia) {  // menor que o primeiro
        no->proximo = lista->inicio;
        lista->inicio = no;
    } else {                                           // meio ou fim
        No *aux = lista->inicio;
        while (aux->proximo && aux->proximo->frequencia <= no->frequencia)
            aux = aux->proximo;
        no->proximo = aux->proximo;
        aux->proximo = no;
    }
    lista->tam++;
}

void preencher_lista(unsigned int tab[], Lista *lista) {
    for (int i = 0; i < TAM; i++) {
        if (tab[i] > 0) {
            No *novo = malloc(sizeof(No));
            if (novo) {
                novo->caracter = i;
                novo->frequencia = tab[i];
                novo->dir = NULL;
                novo->esq = NULL;
                novo->proximo = NULL;
                inserir_ordenado(lista, novo);
            } else {
                printf("\tERRO ao alocar memoria em preencher_lista!\n");
                break;
            }
        }
    }
}

No *remove_no_inicio(Lista *lista) {
    No *aux = NULL;
    if (lista->inicio) {
        aux = lista->inicio;
        lista->inicio = aux->proximo;
        aux->proximo = NULL;
        lista->tam--;
    }
    return aux;
}

/* ============ PARTE 3: ARVORE DE HUFFMAN ============ */
No *montar_arvore(Lista *lista) {
    No *primeiro, *segundo, *novo;
    while (lista->tam > 1) {
        primeiro = remove_no_inicio(lista);
        segundo  = remove_no_inicio(lista);
        novo = malloc(sizeof(No));
        if (novo) {
            novo->caracter = '+';                       // no interno (valor irrelevante)
            novo->frequencia = primeiro->frequencia + segundo->frequencia;
            novo->esq = primeiro;
            novo->dir = segundo;
            novo->proximo = NULL;
            inserir_ordenado(lista, novo);
        } else {
            printf("\tERRO ao alocar memoria em montar_arvore!\n");
            break;
        }
    }
    return lista->inicio;                               // raiz
}

void imprimir_arvore(No *raiz, int tam) {
    if (raiz->esq == NULL && raiz->dir == NULL) {
        printf("\tFolha: %c\tAltura: %d\n", raiz->caracter, tam);
    } else {
        imprimir_arvore(raiz->esq, tam + 1);
        imprimir_arvore(raiz->dir, tam + 1);
    }
}

int altura_arvore(No *raiz) {
    if (raiz == NULL) return -1;
    int esq = altura_arvore(raiz->esq) + 1;
    int dir = altura_arvore(raiz->dir) + 1;
    return esq > dir ? esq : dir;
}

/* ============ PARTE 4: DICIONARIO ============ */
char **aloca_dicionario(int colunas) {
    char **dicionario = malloc(sizeof(char *) * TAM);
    for (int i = 0; i < TAM; i++)
        dicionario[i] = calloc(colunas, sizeof(char));
    return dicionario;
}

void gerar_dicionario(char **dicionario, No *raiz, char *caminho, int colunas) {
    char esquerda[colunas], direita[colunas];
    if (raiz->esq == NULL && raiz->dir == NULL) {       // folha
        strcpy(dicionario[raiz->caracter], caminho);
    } else {
        strcpy(esquerda, caminho);
        strcpy(direita, caminho);
        strcat(esquerda, "0");
        strcat(direita, "1");
        gerar_dicionario(dicionario, raiz->esq, esquerda, colunas);
        gerar_dicionario(dicionario, raiz->dir, direita, colunas);
    }
}

void imprime_dicionario(char **dicionario) {
    printf("\n\tDicionario:\n");
    for (int i = 0; i < TAM; i++)
        if (strlen(dicionario[i]) > 0)
            printf("\t%3d: %s\n", i, dicionario[i]);
}

/* ============ PARTE 5: CODIFICAR ============ */
int calcula_tamanho_string(char **dicionario, unsigned char *texto) {
    int tam = 0;
    for (int i = 0; texto[i] != '\0'; i++)
        tam += strlen(dicionario[texto[i]]);
    return tam + 1;                                     // +1 para o '\0'
}

char *codificar(char **dicionario, unsigned char *texto) {
    int tam = calcula_tamanho_string(dicionario, texto);
    char *codigo = calloc(tam, sizeof(char));
    for (int i = 0; texto[i] != '\0'; i++)
        strcat(codigo, dicionario[texto[i]]);
    return codigo;
}

/* ============ PARTE 6: DECODIFICAR ============ */
char *decodificar(unsigned char texto[], No *raiz) {
    No *aux = raiz;
    char temp[2];
    char *decodificado = calloc(strlen((char *)texto) + 1, sizeof(char));
    for (int i = 0; texto[i] != '\0'; i++) {
        aux = (texto[i] == '0') ? aux->esq : aux->dir;
        if (aux->esq == NULL && aux->dir == NULL) {     // chegou numa folha
            temp[0] = aux->caracter;
            temp[1] = '\0';
            strcat(decodificado, temp);
            aux = raiz;                                 // volta para a raiz
        }
    }
    return decodificado;
}

/* ============ PARTE 7: LIBERAR MEMORIA ============ */
void liberar_arvore(No *raiz) {
    if (raiz) {
        liberar_arvore(raiz->esq);
        liberar_arvore(raiz->dir);
        free(raiz);
    }
}

void liberar_dicionario(char **dicionario) {
    for (int i = 0; i < TAM; i++) free(dicionario[i]);
    free(dicionario);
}

/* ============ MAIN ============ */
int main() {
    unsigned char texto[] = "Vamos aprender a programar";
    unsigned int tabela_frequencia[TAM];
    Lista lista;
    No *arvore;
    char **dicionario;
    int colunas;
    char *codificado, *decodificado;

    inicializa_tabela_com_zero(tabela_frequencia);
    preenche_tab_frequencia(texto, tabela_frequencia);

    criar_lista(&lista);
    preencher_lista(tabela_frequencia, &lista);

    arvore = montar_arvore(&lista);
    printf("\n\tArvore de Huffman\n");
    imprimir_arvore(arvore, 0);

    colunas = altura_arvore(arvore) + 1;
    dicionario = aloca_dicionario(colunas);
    gerar_dicionario(dicionario, arvore, "", colunas);
    imprime_dicionario(dicionario);

    codificado = codificar(dicionario, texto);
    printf("\n\tTexto codificado: %s\n", codificado);

    decodificado = decodificar((unsigned char *)codificado, arvore);
    printf("\n\tTexto decodificado: %s\n\n", decodificado);

    free(codificado);
    free(decodificado);
    liberar_dicionario(dicionario);
    liberar_arvore(arvore);
    return 0;
}
