#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define QUANTIDADE_DE_BYTES_POSSIVEIS 256

#define TAMANHO_DO_BLOCO_DE_LEITURA 65536

#define BITS_DO_CAMPO_DE_LIXO 3
#define BITS_DO_CAMPO_DE_TAMANHO_DA_ARVORE 13

#define TAMANHO_MAXIMO_DA_ARVORE_EM_BYTES ((1 << BITS_DO_CAMPO_DE_TAMANHO_DA_ARVORE) - 1)

#define CARACTERE_DE_NO_INTERNO '*'
#define CARACTERE_DE_ESCAPE '\\'

typedef struct NoDaArvore {
    uint8_t valor_do_byte;
    uint64_t frequencia;
    struct NoDaArvore *filho_esquerdo;
    struct NoDaArvore *filho_direito;
} NoDaArvore;

static int no_eh_folha(const NoDaArvore *no) {
    return no->filho_esquerdo == NULL && no->filho_direito == NULL;
}

static NoDaArvore *criar_no(uint8_t valor_do_byte, uint64_t frequencia,
                            NoDaArvore *filho_esquerdo, NoDaArvore *filho_direito) {
    NoDaArvore *no = malloc(sizeof(NoDaArvore));
    if (no == NULL) {
        fprintf(stderr, "Sem memoria\n");
        exit(1);
    }
    no->valor_do_byte = valor_do_byte;
    no->frequencia = frequencia;
    no->filho_esquerdo = filho_esquerdo;
    no->filho_direito = filho_direito;
    return no;
}

static void liberar_arvore_de_huffman(NoDaArvore *no) {
    if (no == NULL) return;
    liberar_arvore_de_huffman(no->filho_esquerdo);
    liberar_arvore_de_huffman(no->filho_direito);
    free(no);
}

typedef struct {
    NoDaArvore *elementos[QUANTIDADE_DE_BYTES_POSSIVEIS];
    int quantidade_de_elementos;
} FilaDePrioridade;

static void trocar_nos(NoDaArvore **primeiro, NoDaArvore **segundo) {
    NoDaArvore *auxiliar = *primeiro;
    *primeiro = *segundo;
    *segundo = auxiliar;
}

static void inserir_na_fila(FilaDePrioridade *fila, NoDaArvore *no) {
    int posicao = fila->quantidade_de_elementos++;
    fila->elementos[posicao] = no;

    while (posicao > 0) {
        int posicao_do_pai = (posicao - 1) / 2;
        if (fila->elementos[posicao_do_pai]->frequencia <= fila->elementos[posicao]->frequencia)
            break;
        trocar_nos(&fila->elementos[posicao_do_pai], &fila->elementos[posicao]);
        posicao = posicao_do_pai;
    }
}

static NoDaArvore *remover_menor_da_fila(FilaDePrioridade *fila) {
    NoDaArvore *menor_no = fila->elementos[0];
    fila->quantidade_de_elementos--;
    fila->elementos[0] = fila->elementos[fila->quantidade_de_elementos];

    int posicao = 0;
    for (;;) {
        int posicao_filho_esquerdo = 2 * posicao + 1;
        int posicao_filho_direito = posicao_filho_esquerdo + 1;
        int posicao_do_menor = posicao;

        if (posicao_filho_esquerdo < fila->quantidade_de_elementos &&
            fila->elementos[posicao_filho_esquerdo]->frequencia < fila->elementos[posicao_do_menor]->frequencia)
            posicao_do_menor = posicao_filho_esquerdo;

        if (posicao_filho_direito < fila->quantidade_de_elementos &&
            fila->elementos[posicao_filho_direito]->frequencia < fila->elementos[posicao_do_menor]->frequencia)
            posicao_do_menor = posicao_filho_direito;

        if (posicao_do_menor == posicao) break;
        trocar_nos(&fila->elementos[posicao_do_menor], &fila->elementos[posicao]);
        posicao = posicao_do_menor;
    }
    return menor_no;
}

static NoDaArvore *construir_arvore_de_huffman(
        const uint64_t frequencia_de_cada_byte[QUANTIDADE_DE_BYTES_POSSIVEIS]) {

    FilaDePrioridade fila = { .quantidade_de_elementos = 0 };

    for (int valor = 0; valor < QUANTIDADE_DE_BYTES_POSSIVEIS; valor++) {
        if (frequencia_de_cada_byte[valor] > 0) {
            inserir_na_fila(&fila, criar_no((uint8_t)valor, frequencia_de_cada_byte[valor], NULL, NULL));
        }
    }

    if (fila.quantidade_de_elementos == 0) return NULL;

    while (fila.quantidade_de_elementos > 1) {
        NoDaArvore *menor = remover_menor_da_fila(&fila);
        NoDaArvore *segundo_menor = remover_menor_da_fila(&fila);
        NoDaArvore *pai = criar_no(0, menor->frequencia + segundo_menor->frequencia,
                                   menor, segundo_menor);
        inserir_na_fila(&fila, pai);
    }
    return remover_menor_da_fila(&fila);
}

typedef struct {
    uint8_t bits[QUANTIDADE_DE_BYTES_POSSIVEIS];
    int quantidade_de_bits;
} CodigoDoByte;

static void gerar_codigos_percorrendo_arvore(
        const NoDaArvore *no_atual,
        CodigoDoByte tabela_de_codigos[QUANTIDADE_DE_BYTES_POSSIVEIS],
        uint8_t caminho_ate_aqui[QUANTIDADE_DE_BYTES_POSSIVEIS],
        int profundidade) {

    if (no_eh_folha(no_atual)) {
        memcpy(tabela_de_codigos[no_atual->valor_do_byte].bits, caminho_ate_aqui, profundidade);
        tabela_de_codigos[no_atual->valor_do_byte].quantidade_de_bits = profundidade;
        return;
    }

    caminho_ate_aqui[profundidade] = 0;
    gerar_codigos_percorrendo_arvore(no_atual->filho_esquerdo, tabela_de_codigos,
                                     caminho_ate_aqui, profundidade + 1);

    caminho_ate_aqui[profundidade] = 1;
    gerar_codigos_percorrendo_arvore(no_atual->filho_direito, tabela_de_codigos,
                                     caminho_ate_aqui, profundidade + 1);
}

static int serializar_arvore_em_pre_ordem(const NoDaArvore *no, uint8_t *destino, int posicao) {
    if (no_eh_folha(no)) {
        if (no->valor_do_byte == CARACTERE_DE_NO_INTERNO ||
            no->valor_do_byte == CARACTERE_DE_ESCAPE) {
            destino[posicao++] = CARACTERE_DE_ESCAPE;
        }
        destino[posicao++] = no->valor_do_byte;
        return posicao;
    }

    destino[posicao++] = CARACTERE_DE_NO_INTERNO;
    posicao = serializar_arvore_em_pre_ordem(no->filho_esquerdo, destino, posicao);
    posicao = serializar_arvore_em_pre_ordem(no->filho_direito, destino, posicao);
    return posicao;
}

static NoDaArvore *reconstruir_arvore_a_partir_da_pre_ordem(
        const uint8_t *arvore_serializada, int tamanho_da_arvore, int *posicao) {

    if (*posicao >= tamanho_da_arvore) return NULL;
    uint8_t byte_atual = arvore_serializada[(*posicao)++];

    if (byte_atual == CARACTERE_DE_NO_INTERNO) {
        NoDaArvore *filho_esquerdo =
            reconstruir_arvore_a_partir_da_pre_ordem(arvore_serializada, tamanho_da_arvore, posicao);
        NoDaArvore *filho_direito =
            reconstruir_arvore_a_partir_da_pre_ordem(arvore_serializada, tamanho_da_arvore, posicao);
        if (filho_esquerdo == NULL || filho_direito == NULL) {
            liberar_arvore_de_huffman(filho_esquerdo);
            liberar_arvore_de_huffman(filho_direito);
            return NULL;
        }
        return criar_no(0, 0, filho_esquerdo, filho_direito);
    }

    if (byte_atual == CARACTERE_DE_ESCAPE) {

        if (*posicao >= tamanho_da_arvore) return NULL;
        byte_atual = arvore_serializada[(*posicao)++];
    }

    return criar_no(byte_atual, 0, NULL, NULL);
}

typedef struct {
    FILE *arquivo_de_saida;
    uint8_t byte_em_construcao;
    int bits_no_byte_atual;
} EscritorDeBits;

static void escrever_um_bit(EscritorDeBits *escritor, int valor_do_bit) {
    escritor->byte_em_construcao =
        (uint8_t)((escritor->byte_em_construcao << 1) | valor_do_bit);
    escritor->bits_no_byte_atual++;

    if (escritor->bits_no_byte_atual == 8) {
        fputc(escritor->byte_em_construcao, escritor->arquivo_de_saida);
        escritor->byte_em_construcao = 0;
        escritor->bits_no_byte_atual = 0;
    }
}

static void finalizar_escrita_de_bits(EscritorDeBits *escritor) {
    if (escritor->bits_no_byte_atual > 0) {
        escritor->byte_em_construcao <<= (8 - escritor->bits_no_byte_atual);
        fputc(escritor->byte_em_construcao, escritor->arquivo_de_saida);
        escritor->byte_em_construcao = 0;
        escritor->bits_no_byte_atual = 0;
    }
}

static int comprimir_arquivo(const char *caminho_do_original, const char *caminho_do_comprimido) {
    FILE *arquivo_original = fopen(caminho_do_original, "rb");
    if (arquivo_original == NULL) { perror("arquivo original"); return 1; }

    uint64_t frequencia_de_cada_byte[QUANTIDADE_DE_BYTES_POSSIVEIS] = {0};
    uint64_t tamanho_original = 0;
    uint8_t bloco_lido[TAMANHO_DO_BLOCO_DE_LEITURA];
    size_t bytes_lidos;

    while ((bytes_lidos = fread(bloco_lido, 1, sizeof bloco_lido, arquivo_original)) > 0) {
        for (size_t i = 0; i < bytes_lidos; i++) {
            frequencia_de_cada_byte[bloco_lido[i]]++;
        }
        tamanho_original += bytes_lidos;
    }

    NoDaArvore *raiz = construir_arvore_de_huffman(frequencia_de_cada_byte);
    static CodigoDoByte tabela_de_codigos[QUANTIDADE_DE_BYTES_POSSIVEIS];
    if (raiz != NULL) {
        uint8_t caminho_ate_aqui[QUANTIDADE_DE_BYTES_POSSIVEIS];
        gerar_codigos_percorrendo_arvore(raiz, tabela_de_codigos, caminho_ate_aqui, 0);

        if (no_eh_folha(raiz)) {
            tabela_de_codigos[raiz->valor_do_byte].bits[0] = 0;
            tabela_de_codigos[raiz->valor_do_byte].quantidade_de_bits = 1;
        }
    }

    uint64_t total_de_bits_dos_dados = 0;
    for (int valor = 0; valor < QUANTIDADE_DE_BYTES_POSSIVEIS; valor++) {
        total_de_bits_dos_dados += frequencia_de_cada_byte[valor] * (uint64_t)tabela_de_codigos[valor].quantidade_de_bits;
    }
    int bits_de_lixo = (int)((8 - (total_de_bits_dos_dados % 8)) % 8);

    uint8_t arvore_serializada[TAMANHO_MAXIMO_DA_ARVORE_EM_BYTES];
    int tamanho_da_arvore = 0;
    if (raiz != NULL) {
        tamanho_da_arvore = serializar_arvore_em_pre_ordem(raiz, arvore_serializada, 0);
    }

    FILE *arquivo_comprimido = fopen(caminho_do_comprimido, "wb");
    if (arquivo_comprimido == NULL) {
        perror("arquivo comprimido");
        liberar_arvore_de_huffman(raiz);
        fclose(arquivo_original);
        return 1;
    }
    uint8_t primeiro_byte_do_cabecalho = (uint8_t)((bits_de_lixo << (BITS_DO_CAMPO_DE_TAMANHO_DA_ARVORE - 8)) |
                                                   (tamanho_da_arvore >> 8));
    uint8_t segundo_byte_do_cabecalho = (uint8_t)(tamanho_da_arvore & 0xFF);
    fputc(primeiro_byte_do_cabecalho, arquivo_comprimido);
    fputc(segundo_byte_do_cabecalho, arquivo_comprimido);

    fwrite(arvore_serializada, 1, (size_t)tamanho_da_arvore, arquivo_comprimido);

    EscritorDeBits escritor = { arquivo_comprimido, 0, 0 };
    rewind(arquivo_original);
    while ((bytes_lidos = fread(bloco_lido, 1, sizeof bloco_lido, arquivo_original)) > 0) {
        for (size_t i = 0; i < bytes_lidos; i++) {
            const CodigoDoByte *codigo = &tabela_de_codigos[bloco_lido[i]];
            for (int posicao_do_bit = 0; posicao_do_bit < codigo->quantidade_de_bits; posicao_do_bit++) {
                escrever_um_bit(&escritor, codigo->bits[posicao_do_bit]);
            }
        }
    }
    finalizar_escrita_de_bits(&escritor);

    long tamanho_comprimido = ftell(arquivo_comprimido);
    printf("Original: %llu bytes | Comprimido: %ld bytes",
           (unsigned long long)tamanho_original, tamanho_comprimido);
    if (tamanho_original > 0) {
        printf(" (%.1f%%)", 100.0 * tamanho_comprimido / (double)tamanho_original);
    }
    printf("\n");

    liberar_arvore_de_huffman(raiz);
    fclose(arquivo_original);
    fclose(arquivo_comprimido);
    return 0;
}

static int descomprimir_arquivo(const char *caminho_do_comprimido, const char *caminho_do_recuperado) {
    FILE *arquivo_comprimido = fopen(caminho_do_comprimido, "rb");
    if (arquivo_comprimido == NULL) { perror("arquivo comprimido"); return 1; }

    int primeiro_byte_do_cabecalho = fgetc(arquivo_comprimido);
    int segundo_byte_do_cabecalho = fgetc(arquivo_comprimido);
    if (primeiro_byte_do_cabecalho == EOF || segundo_byte_do_cabecalho == EOF) {
        fprintf(stderr, "Cabecalho corrompido\n");
        fclose(arquivo_comprimido);
        return 1;
    }

    int bits_de_lixo = primeiro_byte_do_cabecalho >> (8 - BITS_DO_CAMPO_DE_LIXO);

    int tamanho_da_arvore = ((primeiro_byte_do_cabecalho & 0x1F) << 8) | segundo_byte_do_cabecalho;

    uint8_t arvore_serializada[TAMANHO_MAXIMO_DA_ARVORE_EM_BYTES];
    if (fread(arvore_serializada, 1, (size_t)tamanho_da_arvore, arquivo_comprimido) != (size_t)tamanho_da_arvore) {
        fprintf(stderr, "Arvore truncada ou corrompida\n");
        fclose(arquivo_comprimido);
        return 1;
    }

    NoDaArvore *raiz = NULL;
    if (tamanho_da_arvore > 0) {
        int posicao_na_arvore = 0;
        raiz = reconstruir_arvore_a_partir_da_pre_ordem(arvore_serializada, tamanho_da_arvore, &posicao_na_arvore);
        if (raiz == NULL || posicao_na_arvore != tamanho_da_arvore) {
            fprintf(stderr, "Arvore invalida\n");
            liberar_arvore_de_huffman(raiz);
            fclose(arquivo_comprimido);
            return 1;
        }
    }

    FILE *arquivo_recuperado = fopen(caminho_do_recuperado, "wb");
    if (arquivo_recuperado == NULL) {
        perror("arquivo recuperado");
        liberar_arvore_de_huffman(raiz);
        fclose(arquivo_comprimido);
        return 1;
    }

    uint64_t bytes_ja_escritos = 0;
    const NoDaArvore *no_atual = raiz;

    if (raiz != NULL) {

        int byte_atual = fgetc(arquivo_comprimido);
        while (byte_atual != EOF) {
            int proximo_byte = fgetc(arquivo_comprimido);
            int bits_validos = (proximo_byte == EOF) ? (8 - bits_de_lixo) : 8;

            for (int posicao_do_bit = 7; posicao_do_bit >= 8 - bits_validos; posicao_do_bit--) {
                int valor_do_bit = (byte_atual >> posicao_do_bit) & 1;

                if (no_eh_folha(raiz)) {

                    fputc(raiz->valor_do_byte, arquivo_recuperado);
                    bytes_ja_escritos++;
                    continue;
                }

                no_atual = valor_do_bit ? no_atual->filho_direito : no_atual->filho_esquerdo;

                if (no_eh_folha(no_atual)) {
                    fputc(no_atual->valor_do_byte, arquivo_recuperado);
                    bytes_ja_escritos++;
                    no_atual = raiz;
                }
            }
            byte_atual = proximo_byte;
        }
    }

    int houve_erro = (raiz != NULL && !no_eh_folha(raiz) && no_atual != raiz);
    if (houve_erro) {
        fprintf(stderr, "Aviso: arquivo truncado ou corrompido\n");
    } else {
        printf("Descomprimido: %llu bytes\n", (unsigned long long)bytes_ja_escritos);
    }

    liberar_arvore_de_huffman(raiz);
    fclose(arquivo_comprimido);
    fclose(arquivo_recuperado);
    return houve_erro;
}

int main(int quantidade_de_argumentos, char **argumentos) {
    int argumentos_validos = (quantidade_de_argumentos == 4) &&
                             (argumentos[1][0] == 'c' || argumentos[1][0] == 'd');
    if (!argumentos_validos) {
        fprintf(stderr, "Uso:\n"
                        "  %s c arquivo_original arquivo.huff   (comprimir)\n"
                        "  %s d arquivo.huff arquivo_recuperado (descomprimir)\n",
                argumentos[0], argumentos[0]);
        return 1;
    }

    char modo = argumentos[1][0];
    if (modo == 'c') return comprimir_arquivo(argumentos[2], argumentos[3]);
    return descomprimir_arquivo(argumentos[2], argumentos[3]);
}
