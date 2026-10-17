# Argumentos de linha de comando em C (`argc` e `argv`)

```c
int main(int argc, char *argv[])
```

| Nome   | Vem de            | Significa                              |
| ------ | ----------------- | -------------------------------------- |
| `argc` | *argument count*  | quantos argumentos foram passados      |
| `argv` | *argument vector* | um vetor de strings com os argumentos  |

## Exemplo

Se você rodar no terminal:

```bash
./sat test.cnf
```

Os valores ficam assim:

```text
argc = 2

argv[0] → "./sat"       ← o próprio nome do programa (sempre vem!)
argv[1] → "test.cnf"    ← o seu argumento
```

> ⚠️ O `argv[0]` é sempre o nome do programa, então o primeiro argumento de verdade é o `argv[1]`, e `argc` é no mínimo `1`.

## Abrindo o arquivo

```c
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.cnf>\n", argv[0]);
        return 1;
    }

    FILE *arq = fopen(argv[1], "r");   // "r" = read (leitura)
    if (arq == NULL) {
        printf("Erro ao abrir %s\n", argv[1]);
        return 1;
    }

    // ... aqui entra a sua leitura do DIMACS ...

    fclose(arq);
    return 0;
}
```

Os dois `if` servem de proteção:

- **O usuário esqueceu de passar o arquivo?** O programa avisa como usar em vez de acessar um `argv[1]` que não existe, o que daria *segfault*.
- **O arquivo não existe ou o nome está errado?** O `fopen` devolve `NULL`, e o programa avisa em vez de quebrar.

### ⚠️ Sobre "mesma pasta"

O caminho `"test.cnf"` é relativo à pasta **de onde você roda o comando** no terminal, não à pasta onde está o executável. Se você estiver em outra pasta, precisa passar o caminho:

```bash
./sat testes/test.cnf
```

> A leitura do conteúdo (ignorar o `c`, ler o `p cnf`, ler as cláusulas) fica com você.

---

## `char *argv[]` é um ponteiro de palavras?

> *"Tipo ponteiro de arrays de char, é isso?"*

Quase. A ideia está certa, mas a ordem é inversa: `char *argv[]` é um **array de ponteiros para `char`**, e cada ponteiro aponta para uma palavra.

```text
argv
 ┌─────┐
 │  ●──┼──→ "./sat\0"
 ├─────┤
 │  ●──┼──→ "test.cnf\0"
 ├─────┤
 │NULL │   ← o C garante que argv[argc] == NULL
 └─────┘
```

### Como ler a declaração

Em C, o `[]` tem prioridade maior que o `*`, então a leitura começa pelo nome e vai para a direita antes de ir para a esquerda:

```text
       argv        → argv é...
       argv[]      → ...um array...
      *argv[]      → ...de ponteiros...
 char *argv[]      → ...para char.
```

Um "ponteiro para array de `char`" seria escrito de outra forma, com parênteses:

```c
char (*p)[10];   // p é UM ponteiro para UM array de 10 chars
```

### Detalhe extra

Como parâmetro de função, um array "decai" para ponteiro. Por isso estas duas assinaturas são idênticas para o compilador:

```c
int main(int argc, char *argv[])
int main(int argc, char **argv)   // ponteiro para ponteiro de char
```

Você vai encontrar as duas formas por aí. A primeira deixa mais claro que se trata de uma lista de strings.
