# 🗜️ Desafio: Codificação de Huffman

**Disciplina:** Estruturas de Dados / Programação 2 (ECOM008), UFAL, Instituto de Computação
**Professor:** Márcio Ribeiro
**Aluno:** Luigi Costa Caldas

> **Tarefa (documento do projeto, módulo 2):** compactação e descompactação de **QUAISQUER** tipos de arquivos. **TODAS** as estruturas têm que estar implementadas com `void*`.

---

## 1. O problema

Em ASCII, todo caractere ocupa **8 bits**, seja ele um `e`, que aparece o tempo todo, ou um `z`, que quase não aparece. Isso desperdiça espaço.

| Caractere | Decimal | Hexadecimal | Binário |
|---|---|---|---|
| a | 97 | 61 | `01100001` |
| b | 98 | 62 | `01100010` |
| ... | ... | ... | ... |
| z | 122 | 7A | `01111010` |

A ideia de Huffman tem duas partes:
- usar **códigos de tamanhos diferentes** para caracteres diferentes;
- dar **códigos curtos para o que aparece muito** e longos para o que aparece pouco.

```
"test" em ASCII:    01110100 01100101 01110011 01110100   (32 bits)
"test" com Huffman: 00 01 111 00  →  000111100            (9 bits)   → 71% menor
```

| Caractere | Código |
|---|---|
| e | `01` |
| o | `100` |
| s | `111` |
| t | `00` |

### ⚠️ Regra de ouro: nenhum código pode ser prefixo de outro

| Caractere | Código |
|---|---|
| e | `00` |
| t | `001` |

Ao ler `001111100`, o começo é `e` ou `t`? Não dá para saber.

A árvore de Huffman **garante** que isso nunca acontece, porque os caracteres ficam só nas **folhas**.

---

## 2. Como a árvore é construída

Exemplo do slide: `AAAAAABBBBBCCCCDDDEEF`

### Passo 1: frequências em uma lista ordenada (fila de prioridade)

```
F:1 → E:2 → D:3 → C:4 → B:5 → A:6
```

### Passo 2: juntar os dois menores, repetidamente

Tire os **dois nós de menor frequência**, crie um nó-pai `*` cuja frequência é a **soma** dos filhos, e devolva o pai à lista **na posição correta**. Repita até sobrar **um nó só**, que é a raiz.

| Etapa | Junta | Novo nó | Lista resultante |
|---|---|---|---|
| 1 | F:1 + E:2 | `*:3` | `*:3 → D:3 → C:4 → B:5 → A:6` |
| 2 | *:3 + D:3 | `*:6` | `C:4 → B:5 → *:6 → A:6` |
| 3 | C:4 + B:5 | `*:9` | `*:6 → A:6 → *:9` |
| 4 | *:6 + A:6 | `*:12` | `*:9 → *:12` |
| 5 | *:9 + *:12 | `*:21` | `*:21` ← **raiz** |

### Passo 3: códigos (esquerda = `0`, direita = `1`)

```
               (21)
            0 /    \ 1
           (9)      (12)
         0/  \1    0/   \1
         C    B   (6)    A
                0/  \1
               (3)   D
             0/  \1
             F    E
```

| Caractere | Frequência | Código |
|---|---|---|
| A | 6 | `11` |
| B | 5 | `01` |
| C | 4 | `00` |
| D | 3 | `101` |
| E | 2 | `1001` |
| F | 1 | `1000` |

---

## 3. O arquivo compactado (`filename.huff`)

O arquivo compactado precisa carregar **a árvore** junto com os dados, senão é impossível descompactar. O formato **obrigatório** é este:

```
┌──────────┬───────────────┬─────────────────┬──────────────────────┐
│ 3 bits   │ 13 bits       │ árvore          │ dados compactados    │
│ lixo     │ tamanho da    │ (pré-ordem)     │ (+ lixo no fim)      │
│          │ árvore        │                 │                      │
└──────────┴───────────────┴─────────────────┴──────────────────────┘
 └──── 2 bytes de cabeçalho ───┘
```

### Exemplo do slide

```
10100000 00001011  **CB***FEDA  11111111 11110101 01010100 00000010 11011011 00110011 000_____
```

| Trecho | Valor | Significado |
|---|---|---|
| `101` | 5 | **lixo**: os 5 últimos bits do último byte não são dados |
| `0000000001011` | 11 | **tamanho da árvore**: 11 bytes |
| `**CB***FEDA` | 11 bytes | **árvore** em pré-ordem |
| `11111111 ... 000_____` | 51 bits | **dados** compactados, mais 5 bits de lixo |

> 💡 Conferindo: `6×2 + 5×2 + 4×2 + 3×3 + 2×4 + 1×4 = 51 bits`. Isso dá 7 bytes (56 bits), logo `56 − 51 = 5` bits de lixo.

| Campo | Detalhe |
|---|---|
| **Lixo (3 bits)** | quantos bits do **último byte** não são dados. Os dados raramente fecham um múltiplo de 8 |
| **Tamanho da árvore (13 bits)** | quantos bytes a árvore ocupa no arquivo |
| **Árvore** | gravada em **pré-ordem**: nó interno vira `*`, folha vira o próprio byte |
| **Dados** | os códigos de cada byte do arquivo original, colados um atrás do outro |

### ⚠️ O caractere de escape `\`

Se o arquivo original tiver um `*` de verdade, como diferenciá-lo de um nó interno na árvore gravada?

A regra é: **folhas que sejam `*` ou `\` são gravadas com um `\` antes**, ou seja, `\*` e `\\`. Esse `\` extra **conta** no tamanho da árvore.

---

## 4. Ferramentas de bits (slides "Helpful functions")

Você vai ler e escrever **bit a bit**, mas o arquivo só aceita **bytes**.

| Operação | Efeito |
|---|---|
| `x << n` | desloca `x` **n** bits para a esquerda |
| `x >> n` | desloca `x` **n** bits para a direita |

```
x = 11000111
x << 2  →  00011100
x >> 2  →  00110001
```

> Os slides consideram valores **sem sinal**: trabalhe sempre com `unsigned char`.

### O bit `i` está ligado?

```c
int is_bit_i_set(unsigned char c, int i)
{
    unsigned char mask = 1 << i;
    return mask & c;
}
```

### Ligar o bit `i`

```c
unsigned char set_bit(unsigned char c, int i)
{
    unsigned char mask = 1 << i;
    return mask | c;
}
```

---

## 5. Regras obrigatórias (slide "Mandatory Rules")

| Regra | Implicação |
|---|---|
| Seguir o cabeçalho | 3 bits de lixo + 13 bits de tamanho, nessa ordem |
| Árvore com `*` em pré-ordem | serialização exatamente como no slide |
| `\` como caractere de escape | para `*` e `\` que aparecem como folha |
| TADs documentados | comentários estilo JavaDoc em cada estrutura e função |
| **`void*` em todas as estruturas** | a fila de prioridade e a árvore guardam `void*`, não `unsigned char` |
| **Huffman para qualquer formato** | texto, imagem, PDF, executável: leia e escreva em modo **binário** (`"rb"` / `"wb"`) |

---

## 6. Casos de teste

| Arquivo | O que verifica |
|---|---|
| `AAAAAABBBBBCCCCDDDEEF` | o resultado deve bater com o slide: lixo 5, árvore `**CB***FEDA` de 11 bytes |
| texto contendo `*` e `\` | o caractere de escape |
| imagem (`.png`, `.jpg`) ou `.pdf` | arquivos binários |
| arquivo com **um único byte repetido** (`aaaa`) | 🤔 que código uma árvore com uma folha só gera? |
| arquivo **vazio** | 🤔 o que o programa deve fazer? |

> ✅ **Critério de sucesso:** compactar e depois descompactar deve gerar um arquivo **byte a byte idêntico** ao original. No Linux, `cmp original descompactado` não pode imprimir nada.
>
> ⚠️ A árvore pode sair diferente da do slide quando há **empates** de frequência, dependendo de onde o nó novo entra na lista. O que importa é a descompactação recuperar o original.

---

## 7. Níveis de desafio 🎯

| Nível | Meta |
|---|---|
| 1 | Contar a frequência dos 256 bytes possíveis de um arquivo |
| 2 | Fila de prioridade com `void*` e construção da árvore |
| 3 | Gerar a tabela de códigos de cada byte |
| 4 | Escrever o `.huff`: cabeçalho, árvore e dados bit a bit |
| 5 | Descompactar: ler o cabeçalho, reconstruir a árvore e decodificar |

---

## 8. Ferramentas sugeridas pelo professor

- **Eclipse + CUnit:** testes unitários.
- **Hex Editor:** inspecionar o `.huff` byte a byte. No VS Code, a extensão *Hex Editor* faz o mesmo.
- **GitHub:** uso obrigatório no projeto.
- **Referências:** *Introduction to Algorithms* (Cormen), cap. 16; *Estrutura de Dados e Algoritmos em C++* (Drozdek), cap. 11.