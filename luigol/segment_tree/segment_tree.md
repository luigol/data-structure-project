# 🌲 Desafio: Segment Tree (Árvore de Segmentos)

**Disciplina:** Estruturas de Dados / Programação 2 (ECOM008), UFAL, Instituto de Computação
**Professor:** Márcio Ribeiro
**Aluno:** Luigi Costa Caldas

> **Tarefa (documento do projeto, módulo 1):** implementar DOIS algoritmos ou estruturas de dados com CÓDIGO. Temas da equipe: **Árvores de Decisão com Entropia** e **Segment Tree**. Apresentação dos dois com, no máximo, **10 slides**.
>
> **Linguagem:** C.

---

## 1. O problema

Você tem um array e precisa fazer duas operações, **muitas vezes**:
- **Consulta:** qual é a soma de `arr[left..right]`?
- **Atualização:** trocar o valor de `arr[i]`.

```
índice:  0  1  2  3  4  5  6  7
arr:     5  8  6  3  2  7  2  6

sum[2..5] = 6 + 3 + 2 + 7 = 18
```

| Abordagem | Consulta | Atualização |
|---|---|---|
| Somar na hora (laço de `left` até `right`) | O(n) | O(1) |
| Somas de prefixo (`prefix[i]` = soma de `arr[0..i]`) | O(1) | O(n): todos os prefixos depois de `i` mudam |
| **Segment Tree** | **O(log n)** | **O(log n)** |

As duas primeiras são rápidas em uma operação e lentas na outra. A segment tree é rápida **nas duas**.

> 💡 Com 1.000.000 de elementos, O(n) são até 1.000.000 de passos; O(log n) são cerca de 20 (`log₂ 1.000.000 ≈ 20`).

---

## 2. A ideia

Cada nó da árvore guarda a **soma de um pedaço (segmento)** do array:
- a **raiz** cobre o array inteiro, `[0..7]`;
- cada nó **divide o seu pedaço ao meio**: a metade esquerda vai para o filho esquerdo e a direita para o filho direito;
- as **folhas** cobrem um único elemento.

```
                      [0..7] = 39
                /                     \
       [0..3] = 22                    [4..7] = 17
         /      \                       /      \
 [0..1] = 13   [2..3] = 9       [4..5] = 9    [6..7] = 8
    /   \        /    \           /    \        /    \
   5     8      6      3         2      7      2      6
```

> O valor de um nó interno é sempre **filho esquerdo + filho direito**. Ex.: `22 = 13 + 9`.

A vantagem: qualquer intervalo do array pode ser montado juntando **poucos** desses pedaços prontos, em vez de somar elemento por elemento.

---

## 3. Guardando a árvore em um array

Não precisa de `struct`, ponteiros nem `malloc`: a árvore fica em um array `tree[]`, do mesmo jeito que um **heap**.

| Quem | Índice |
|---|---|
| raiz | `1` |
| filho esquerdo de `i` | `2 * i` |
| filho direito de `i` | `2 * i + 1` |
| pai de `i` | `i / 2` |

```
                  tree[1]
              /             \
       tree[2]               tree[3]
       /     \               /     \
  tree[4]   tree[5]     tree[6]   tree[7]
   /   \     /   \       /   \     /   \
  8     9   10   11     12   13   14   15
```

> `tree[0]` não é usado. Começar do `1` deixa a conta dos filhos mais simples.

Para o nosso array, o `tree[]` fica assim:

| índice | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| segmento | 0..7 | 0..3 | 4..7 | 0..1 | 2..3 | 4..5 | 6..7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| `tree[i]` | 39 | 22 | 17 | 13 | 9 | 9 | 8 | 5 | 8 | 6 | 3 | 2 | 7 | 2 | 6 |

> 💡 Assim como no Huffman, é uma árvore binária. A diferença é que aqui todo nó interno tem **exatamente 2 filhos**, e a posição de cada um é **calculada** (`2 * i`), não guardada em um ponteiro.

### ⚠️ Por que `tree[4 * N]`?

Com `N = 8` (uma potência de 2), a árvore é perfeita e usa os índices `1` a `15`, então `2 * N` posições bastariam. Mas, quando `N` **não** é potência de 2, a árvore fica "torta" e alguns índices vão mais longe:

| `N` | maior índice usado | `2 * N` basta? |
|---|---|---|
| 6 | 13 | ❌ vetor de 12 posições |
| 8 | 15 | ✅ |
| 10 | 25 | ❌ vetor de 20 posições |
| 12 | 29 | ❌ vetor de 24 posições |

A altura da árvore é `⌈log₂ N⌉`, então o maior índice fica sempre abaixo de `2^(⌈log₂ N⌉ + 1)`, que nunca passa de `4 * N`. Por isso `4 * N` é o tamanho usado na prática.

---

## 4. As funções

Todas recebem o nó atual (`node`) e o pedaço que ele cobre (`[start, end]`). A chamada sempre começa pela raiz: `node = 1`, `start = 0`, `end = N - 1`.

| Função | O que faz | Custo |
|---|---|---|
| `build` | monta a árvore a partir do `arr` | O(n log n) |
| `query` | soma de `arr[left..right]` | O(log n) |
| `update` | faz `arr[index] = value` e corrige as somas | O(log n) |
| `print_tree` | imprime a árvore com indentação | O(n) |

> 💡 `arr` e `tree` são **globais** de propósito: assim as funções recebem só índices, e o foco fica na lógica da árvore.

### `build`: montar a árvore

```c
void build(int node, int start, int end) {

    // Pre-order: first the node, then its children
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += arr[i];
    }
    tree[node] = sum;

    if (start == end) return; // Leaf: a single element

    int mid = (start + end) / 2;
    build(2 * node, start, mid);         // Left half
    build(2 * node + 1, mid + 1, end);   // Right half
}
```

- **Primeiro o nó:** soma os elementos do seu pedaço, `arr[start..end]`, e guarda o resultado em `tree[node]`.
- **Depois os filhos:** se o pedaço tiver mais de um elemento, divide ao meio e monta cada metade. É um percurso em **pré-ordem**, como o `write_tree` do Huffman: o nó antes dos filhos.

**Custo:** em cada nível da árvore, os pedaços somam juntos os `n` elementos uma vez. Ex.: no 2º nível, `[0..3]` e `[4..7]` somam 4 + 4 = 8 elementos. Com `log₂ n + 1` níveis, o custo é **O(n log n)**. Como o `build` roda **uma única vez**, isso não afeta as consultas e as atualizações, que continuam O(log n).

### `query`: soma de um intervalo

Comparando o pedaço do nó `[start, end]` com a consulta `[left, right]`, só existem **3 casos**:

| Caso | Condição | Resposta |
|---|---|---|
| 1. Totalmente **fora** | `right < start` ou `end < left` | `0`: não contribui |
| 2. Totalmente **dentro** | `left <= start` e `end <= right` | `tree[node]`, sem descer mais |
| 3. **Parcial** | qualquer outra | pergunta aos dois filhos e soma |

```c
int query(int node, int start, int end, int left, int right) {

    // Case 1: the node is completely outside the query
    if (right < start || end < left) return 0;

    // Case 2: the node is completely inside the query
    if (left <= start && end <= right) return tree[node];

    // Case 3: partial overlap -> ask both children
    int mid = (start + end) / 2;
    int left_sum = query(2 * node, start, mid, left, right);
    int right_sum = query(2 * node + 1, mid + 1, end, left, right);

    return left_sum + right_sum;
}
```

#### Exemplo: `sum[2..5]`

```
                     [0..7] parcial
               /                         \
      [0..3] parcial                 [4..7] parcial
       /          \                   /          \
[0..1] fora   [2..3] dentro   [4..5] dentro   [6..7] fora
    → 0           → 9             → 9             → 0
```

| Passo | Nó | Caso | Retorna |
|---|---|---|---|
| 1 | `[0..7]` | parcial | `9 + 9 = 18` |
| 2 | `[0..3]` | parcial | `0 + 9 = 9` |
| 3 | `[0..1]` | fora (`1 < 2`) | `0` |
| 4 | `[2..3]` | dentro | `9` |
| 5 | `[4..7]` | parcial | `9 + 0 = 9` |
| 6 | `[4..5]` | dentro | `9` |
| 7 | `[6..7]` | fora (`5 < 6`) | `0` |

Resultado: **18**, igual a `6 + 3 + 2 + 7`. A árvore usou os **2 pedaços prontos** `[2..3]` e `[4..5]` em vez de somar os 4 elementos um por um.

> 💡 **Por que O(log n)?** Os nós "fora" e "dentro" param na hora; só os parciais descem. Em cada nível existem no máximo **2 nós parciais** (um em cada borda da consulta), então são visitados no máximo **4 nós por nível**, e a árvore tem `log₂ n` níveis.

### `update`: trocar um valor

```c
void update(int node, int start, int end, int index, int value) {

    // Pre-order: fix the node before going down.
    // arr[index] still has the old value, so the sum changes by (value - arr[index])
    tree[node] += value - arr[index];

    if (start == end) { // Leaf: start == end == index
        arr[index] = value;
        return;
    }

    // Go down only on the side that contains the index
    int mid = (start + end) / 2;
    if (index <= mid) update(2 * node, start, mid, index, value);
    else update(2 * node + 1, mid + 1, end, index, value);
}
```

Também em **pré-ordem**: cada nó do caminho é corrigido **antes** de descer. Se `arr[index]` passa de `3` para `10`, toda soma que inclui essa posição aumenta exatamente `10 − 3 = +7`. A função soma essa diferença no nó atual, desce **só pelo lado que contém o `index`** e, ao chegar na folha, troca o valor no `arr`.

> ⚠️ O `arr[index]` só é trocado **na folha**, no fim da descida. Até lá, ele guarda o valor antigo, e é isso que permite calcular a diferença `value - arr[index]` em cada nó do caminho.

#### Exemplo: `arr[3] = 10` (antes era `3`, diferença `+7`)

```
[0..7]  39 → 46                    ← 1º
  └── [0..3]  22 → 29              ← 2º
        └── [2..3]  9 → 16         ← 3º
              └── [3..3]  3 → 10   ← 4º (folha)
```

| Ordem | Nó | Antes | Depois |
|---|---|---|---|
| 1º | `[0..7]` (`tree[1]`) | 39 | 39 + 7 = **46** |
| 2º | `[0..3]` (`tree[2]`) | 22 | 22 + 7 = **29** |
| 3º | `[2..3]` (`tree[5]`) | 9 | 9 + 7 = **16** |
| 4º | `[3..3]` (`tree[11]`) | 3 | 3 + 7 = **10** |

Só **4 nós** mudam, um por nível (`log₂ 8 + 1 = 4`). O lado direito inteiro, `[4..7]`, nem é visitado.

> ⚠️ Com **somas de prefixo**, a mesma troca obrigaria a recalcular `prefix[3]` até `prefix[7]`. Com 1.000.000 de elementos, seriam até 1.000.000 de contas; aqui são ~20.

---

## 5. Como rodar

```powershell
gcc main.c -o main
./main.exe
```

> O array está declarado no próprio `main.c`. Para testar outro, troque o `arr` e o `N` no topo do arquivo.

---

## 6. Saída esperada

```
Array: 5 8 6 3 2 7 2 6

Segment tree:
[0..7] = 39   (tree[1])
    [0..3] = 22   (tree[2])
        [0..1] = 13   (tree[4])
            [0..0] = 5   (tree[8])
            [1..1] = 8   (tree[9])
        [2..3] = 9   (tree[5])
            [2..2] = 6   (tree[10])
            [3..3] = 3   (tree[11])
    [4..7] = 17   (tree[3])
        [4..5] = 9   (tree[6])
            [4..4] = 2   (tree[12])
            [5..5] = 7   (tree[13])
        [6..7] = 8   (tree[7])
            [6..6] = 2   (tree[14])
            [7..7] = 6   (tree[15])

Queries:
  sum[2..5] = 18
  sum[0..7] = 39
  sum[1..3] = 17
  sum[4..4] = 2

Update: arr[3] = 10

Array: 5 8 6 10 2 7 2 6

Segment tree:
[0..7] = 46   (tree[1])
    [0..3] = 29   (tree[2])
        [0..1] = 13   (tree[4])
            [0..0] = 5   (tree[8])
            [1..1] = 8   (tree[9])
        [2..3] = 16   (tree[5])
            [2..2] = 6   (tree[10])
            [3..3] = 10   (tree[11])
    [4..7] = 17   (tree[3])
        [4..5] = 9   (tree[6])
            [4..4] = 2   (tree[12])
            [5..5] = 7   (tree[13])
        [6..7] = 8   (tree[7])
            [6..6] = 2   (tree[14])
            [7..7] = 6   (tree[15])

Queries:
  sum[2..5] = 25
  sum[0..7] = 46
```

> ✅ Conferindo na mão: `sum[2..5]` depois da troca é `6 + 10 + 2 + 7 = 25`, e o total subiu de `39` para `46` (`+7`, a diferença entre `10` e `3`).

---

## 7. 🤔 Para pensar

1. **Mínimo em vez de soma:** o que muda para a árvore responder o **menor** valor de um intervalo (*Range Minimum Query*)? Dica: troque o `+` por um `min` e pense no que o Caso 1 (fora) deve devolver no lugar do `0`.
2. **`N` que não é potência de 2:** desenhe a árvore para `arr = {1, 2, 3, 4, 5, 6}`. Por que o índice `13` é usado, mas o `10` e o `11` não?
3. **Atualizar um intervalo inteiro:** e se fosse preciso somar `+5` em todo `arr[2..6]` de uma vez? Chamar o `update` posição por posição custa O(k log n). A técnica que faz isso em O(log n) se chama **lazy propagation** (propagação preguiçosa).

---

## 8. Níveis de desafio 🎯

| Nível | Meta |
|---|---|
| 1 | Montar a árvore (`build`) e imprimi-la |
| 2 | Consultar a soma de um intervalo (`query`) com os 3 casos |
| 3 | Atualizar uma posição (`update`) e consultar de novo |
| 4 | Adaptar para mínimo ou máximo de um intervalo |
| 5 | Atualizar intervalos inteiros com **lazy propagation** |

---

## 9. Referências

- Laaksonen, A. *Competitive Programmer's Handbook*, 2018, cap. 9 (*Range queries*).
- cp-algorithms: [Segment Tree](https://cp-algorithms.com/data_structures/segment_tree.html).
