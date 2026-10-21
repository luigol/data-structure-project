/*
 * Segment Tree (soma em intervalo) com Lazy Propagation - versão simplificada
 * Estruturas de Dados / Programação 2 - ECOM008 - UFAL
 *
 * Operações (todas O(log n)):
 *   consultar_soma(arvore, inicio, fim)         soma dos elementos de inicio até fim
 *   somar_intervalo(arvore, inicio, fim, valor) soma 'valor' a todos os elementos do intervalo
 *   somar_ponto(arvore, posicao, valor)         soma 'valor' a um único elemento
 *
 * Compilar: gcc segment_tree_simples.c -o segtree
 * Executar: ./segtree
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Cada nó representa um segmento [inicio_no, fim_no] do vetor.
 * O nó numero_no tem como filhos:
 *   esquerdo = 2 * numero_no      -> primeira metade do segmento
 *   direito  = 2 * numero_no + 1  -> segunda metade do segmento
 * A raiz é o nó 1 e cobre o vetor inteiro [0, tamanho - 1].
 */
typedef struct {
    int tamanho;                    /* quantidade de elementos do vetor original */
    long long *soma_do_segmento;    /* soma dos elementos do segmento de cada nó */
    long long *pendencia;           /* valor ainda não repassado aos filhos (lazy) */
} ArvoreSegmentos;

int minimo(int a, int b) { return a < b ? a : b; }
int maximo(int a, int b) { return a > b ? a : b; }

/* ---------------- Construção ---------------- */

void construir_no(ArvoreSegmentos *arvore, int numero_no, int inicio_no, int fim_no, int vetor[]) {
    arvore->pendencia[numero_no] = 0;

    if (inicio_no == fim_no) {              /* folha: guarda um único elemento */
        arvore->soma_do_segmento[numero_no] = vetor[inicio_no];
        return;
    }

    int meio = (inicio_no + fim_no) / 2;
    construir_no(arvore, 2 * numero_no, inicio_no, meio, vetor);
    construir_no(arvore, 2 * numero_no + 1, meio + 1, fim_no, vetor);

    arvore->soma_do_segmento[numero_no] =
        arvore->soma_do_segmento[2 * numero_no] + arvore->soma_do_segmento[2 * numero_no + 1];
}

ArvoreSegmentos *criar_arvore(int vetor[], int tamanho) {
    ArvoreSegmentos *arvore = malloc(sizeof(ArvoreSegmentos));
    arvore->tamanho = tamanho;
    arvore->soma_do_segmento = calloc(4 * tamanho, sizeof(long long));   /* 4n nós bastam */
    arvore->pendencia = calloc(4 * tamanho, sizeof(long long));
    construir_no(arvore, 1, 0, tamanho - 1, vetor);
    return arvore;
}

void liberar_arvore(ArvoreSegmentos *arvore) {
    free(arvore->soma_do_segmento);
    free(arvore->pendencia);
    free(arvore);
}

/* ---------------- Lazy propagation ---------------- */

/* Repassa a pendência do nó para os dois filhos e zera a pendência do nó */
void propagar_pendencia(ArvoreSegmentos *arvore, int numero_no, int inicio_no, int fim_no) {
    long long valor = arvore->pendencia[numero_no];
    if (valor == 0 || inicio_no == fim_no) return;

    int meio = (inicio_no + fim_no) / 2;
    int filho_esquerdo = 2 * numero_no;
    int filho_direito = 2 * numero_no + 1;

    arvore->soma_do_segmento[filho_esquerdo] += valor * (meio - inicio_no + 1);
    arvore->pendencia[filho_esquerdo] += valor;

    arvore->soma_do_segmento[filho_direito] += valor * (fim_no - meio);
    arvore->pendencia[filho_direito] += valor;

    arvore->pendencia[numero_no] = 0;
}

/* ---------------- Atualização ---------------- */

void somar_no(ArvoreSegmentos *arvore, int numero_no, int inicio_no, int fim_no,
              int inicio_intervalo, int fim_intervalo, long long valor) {
    if (inicio_intervalo > fim_intervalo) return;   /* sem interseção com este nó */

    if (inicio_intervalo == inicio_no && fim_intervalo == fim_no) {
        /* segmento totalmente coberto: atualiza só este nó e deixa os filhos para depois */
        arvore->soma_do_segmento[numero_no] += valor * (fim_no - inicio_no + 1);
        arvore->pendencia[numero_no] += valor;
        return;
    }

    propagar_pendencia(arvore, numero_no, inicio_no, fim_no);

    int meio = (inicio_no + fim_no) / 2;
    somar_no(arvore, 2 * numero_no, inicio_no, meio,
             inicio_intervalo, minimo(fim_intervalo, meio), valor);
    somar_no(arvore, 2 * numero_no + 1, meio + 1, fim_no,
             maximo(inicio_intervalo, meio + 1), fim_intervalo, valor);

    arvore->soma_do_segmento[numero_no] =
        arvore->soma_do_segmento[2 * numero_no] + arvore->soma_do_segmento[2 * numero_no + 1];
}

void somar_intervalo(ArvoreSegmentos *arvore, int inicio, int fim, long long valor) {
    somar_no(arvore, 1, 0, arvore->tamanho - 1, inicio, fim, valor);
}

void somar_ponto(ArvoreSegmentos *arvore, int posicao, long long valor) {
    somar_intervalo(arvore, posicao, posicao, valor);
}

/* ---------------- Consulta ---------------- */

long long consultar_no(ArvoreSegmentos *arvore, int numero_no, int inicio_no, int fim_no,
                       int inicio_consulta, int fim_consulta) {
    if (inicio_consulta > fim_consulta) return 0;   /* sem interseção com este nó */

    if (inicio_consulta == inicio_no && fim_consulta == fim_no)
        return arvore->soma_do_segmento[numero_no]; /* segmento totalmente dentro da consulta */

    propagar_pendencia(arvore, numero_no, inicio_no, fim_no);

    int meio = (inicio_no + fim_no) / 2;
    return consultar_no(arvore, 2 * numero_no, inicio_no, meio,
                        inicio_consulta, minimo(fim_consulta, meio))
         + consultar_no(arvore, 2 * numero_no + 1, meio + 1, fim_no,
                        maximo(inicio_consulta, meio + 1), fim_consulta);
}

long long consultar_soma(ArvoreSegmentos *arvore, int inicio, int fim) {
    return consultar_no(arvore, 1, 0, arvore->tamanho - 1, inicio, fim);
}

/* ---------------- Impressão (para visualizar a árvore) ---------------- */

void imprimir_no(ArvoreSegmentos *arvore, int numero_no, int inicio_no, int fim_no, int profundidade) {
    for (int i = 0; i < profundidade; i++) printf("    ");
    printf("[%d,%d] soma=%lld", inicio_no, fim_no, arvore->soma_do_segmento[numero_no]);
    if (arvore->pendencia[numero_no] != 0)
        printf(" (pendencia=%lld)", arvore->pendencia[numero_no]);
    printf("\n");

    if (inicio_no == fim_no) return;
    int meio = (inicio_no + fim_no) / 2;
    imprimir_no(arvore, 2 * numero_no, inicio_no, meio, profundidade + 1);
    imprimir_no(arvore, 2 * numero_no + 1, meio + 1, fim_no, profundidade + 1);
}

/* ---------------- main ---------------- */
int main(void) {
    /* --- Demonstração --- */
    int vetor[] = {5, 3, 8, 6, 2, 7, 4, 1};
    int tamanho = sizeof(vetor) / sizeof(vetor[0]);
    ArvoreSegmentos *arvore = criar_arvore(vetor, tamanho);

    printf("Vetor: ");
    for (int i = 0; i < tamanho; i++) printf("%d ", vetor[i]);
    printf("\n\nArvore construida:\n");
    imprimir_no(arvore, 1, 0, tamanho - 1, 0);

    printf("\nsoma[1..4] = %lld\n", consultar_soma(arvore, 1, 4));
    printf("soma[0..7] = %lld\n", consultar_soma(arvore, 0, 7));

    somar_ponto(arvore, 2, 10);
    printf("\nApos somar 10 na posicao 2:\nsoma[1..4] = %lld\n", consultar_soma(arvore, 1, 4));

    somar_intervalo(arvore, 2, 5, 3);
    printf("\nApos somar 3 nas posicoes 2..5:\nsoma[0..7] = %lld\n", consultar_soma(arvore, 0, 7));
    printf("soma[3..4] = %lld\n", consultar_soma(arvore, 3, 4));
    liberar_arvore(arvore);

    /* --- Teste aleatório: compara com a soma feita com um laço simples --- */
    srand((unsigned)time(NULL));
    int tamanho_teste = 1000, operacoes = 20000, erros = 0;
    int *vetor_teste = malloc(tamanho_teste * sizeof(int));
    long long *vetor_referencia = malloc(tamanho_teste * sizeof(long long));
    for (int i = 0; i < tamanho_teste; i++) {
        vetor_teste[i] = rand() % 100;
        vetor_referencia[i] = vetor_teste[i];
    }
    ArvoreSegmentos *arvore_teste = criar_arvore(vetor_teste, tamanho_teste);

    for (int k = 0; k < operacoes; k++) {
        int inicio = rand() % tamanho_teste;
        int fim = rand() % tamanho_teste;
        if (inicio > fim) { int troca = inicio; inicio = fim; fim = troca; }

        if (rand() % 2 == 0) {
            long long valor = rand() % 21 - 10;
            somar_intervalo(arvore_teste, inicio, fim, valor);
            for (int i = inicio; i <= fim; i++) vetor_referencia[i] += valor;
        } else {
            long long esperado = 0;
            for (int i = inicio; i <= fim; i++) esperado += vetor_referencia[i];
            if (consultar_soma(arvore_teste, inicio, fim) != esperado) erros++;
        }
    }
    printf("\nTeste aleatorio (%d operacoes): %s\n", operacoes, erros ? "FALHOU" : "tudo correto");

    liberar_arvore(arvore_teste);
    free(vetor_teste);
    free(vetor_referencia);
    return erros != 0;
}
