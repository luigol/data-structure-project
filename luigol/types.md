# 🔢 Tipos inteiros em C

## Tabela geral

| Tipo | Tamanho | Faixa | Formato `printf` | Uso típico |
|---|---|---|---|---|
| `char` | 1 byte | **-128 a 127** ou **0 a 255**, depende do compilador | `%c` (letra) / `%d` (número) | texto |
| `signed char` | 1 byte | -128 a 127, sempre | `%hhd` | inteiro minúsculo com sinal (raro) |
| `unsigned char` | 1 byte | 0 a 255, sempre | `%hhu` | **bytes crus** (arquivos binários) |
| `short` | 2 bytes | -32.768 a 32.767 | `%hd` | economizar memória em vetores grandes |
| `unsigned short` | 2 bytes | 0 a 65.535 | `%hu` | idem, sem negativos |
| `int` | 4 bytes (em geral) | ≈ -2,1 bi a 2,1 bi | `%d` | números em geral, e o **retorno do `fgetc`** |
| `unsigned int` | 4 bytes (em geral) | 0 a ≈ 4,3 bi | `%u` | contagens e manipulação de bits |
| `long` | **4 ou 8 bytes** ⚠️ | depende do sistema | `%ld` | evitar: o tamanho muda entre Windows e Linux |
| `unsigned long` | **4 ou 8 bytes** ⚠️ | depende do sistema | `%lu` | idem |
| `long long` | 8 bytes | ≈ -9,2 × 10¹⁸ a 9,2 × 10¹⁸ | `%lld` | números muito grandes |
| `unsigned long long` | 8 bytes | 0 a ≈ 1,8 × 10¹⁹ | `%llu` | contagens enormes (ex.: frequências no Huffman) |

### Valores exatos dos limites

| Tipo | Mínimo | Máximo |
|---|---|---|
| `int` | -2.147.483.648 | 2.147.483.647 |
| `unsigned int` | 0 | 4.294.967.295 |
| `long long` | -9.223.372.036.854.775.808 | 9.223.372.036.854.775.807 |
| `unsigned long long` | 0 | 18.446.744.073.709.551.615 |

> 💡 A regra por trás: com **n bits**, um tipo `unsigned` vai de **0 a 2ⁿ − 1**, e um tipo com sinal vai de **−2ⁿ⁻¹ a 2ⁿ⁻¹ − 1**. O `unsigned` não "cria" valores: troca os negativos por positivos, dobrando o alcance para cima.

---

## ⚠️ Pegadinhas

### 1. `char` puro: com ou sem sinal?
Cada compilador decide. No PC comum (x86 + GCC), `char` tem **sinal**. Em ARM, costuma **não ter**. Para bytes de arquivo, use sempre **`unsigned char`**.

### 2. `long` muda de tamanho

| Sistema | `long` |
|---|---|
| Windows 64 bits | 4 bytes |
| Linux / macOS 64 bits | 8 bytes |

Se precisar de 8 bytes garantidos, use **`long long`**.

### 3. Estouro (*overflow*)

```c
unsigned char x = 255;
x = x + 1;          // x vira 0 — "dá a volta"

int y = 2147483647;
y = y + 1;          // comportamento indefinido em C (tipos com sinal)
```

- **`unsigned`:** o estouro "dá a volta" (volta a 0), um comportamento definido pela linguagem.
- **Com sinal:** o estouro é **comportamento indefinido**, ou seja, qualquer coisa pode acontecer.

---

## Caractere é número

```c
char c = 'A';   // é o mesmo que char c = 65;
```

| Escrita | O que é | Valor | Binário |
|---|---|---|---|
| `1` | o **número** um | 1 | `00000001` |
| `'1'` | o **caractere** "um" | 49 | `00110001` |
| `"1"` | uma **string** (2 bytes) | `'1'` + `'\0'` | `00110001 00000000` |

---

## O caso do `fgetc`

O `fgetc` devolve **257 respostas possíveis**: os bytes de 0 a 255 **e** o `EOF` (-1). Um tipo de 1 byte só tem 256 valores.

| Tipo recebendo o `fgetc` | Resultado |
|---|---|
| `char` (com sinal) | o byte 255 vira -1, que é igual ao `EOF`, e a leitura **para cedo** (todo JPEG começa com o byte 255!) |
| `char` (sem sinal) / `unsigned char` | o `EOF` vira 255, nunca é detectado, e o **laço fica infinito** |
| **`int`** | ✅ correto |

> **Regra prática:** `int` para receber o `fgetc`, `unsigned char` para guardar bytes.

---

## Descobrindo o tamanho na sua máquina

```c
printf("%zu\n", sizeof(long));   // %zu é o formato do resultado de sizeof
```

Os limites exatos estão em `<limits.h>`: `INT_MAX`, `UINT_MAX`, `LLONG_MAX`, `ULLONG_MAX` etc.
