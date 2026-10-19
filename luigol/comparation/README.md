# 📊 Desafio: Lista vs Árvore de Busca Binária

**Disciplina:** Estruturas de Dados / Programação 2 (ECOM008), UFAL, Instituto de Computação
**Professor:** Márcio Ribeiro
**Aluno:** Luigi Costa Caldas

> **Tarefa (documento do projeto, módulo 4):** armazenar muitos dados em duas estruturas, fazer vários sorteios e **contar o número de comparações** que cada uma precisa para achar o número sorteado. Coletar essas comparações e gerar um gráfico em **R ou Matlab**. Os dados devem ser gerados e plotados **NA HORA** da apresentação!

---

## 1. O problema

As duas estruturas recebem **os mesmos dados**. A cada sorteio, buscamos o número nas duas e contamos quantas comparações cada uma fez. Os resultados vão para um CSV e viram um gráfico em R.

O enunciado obriga a escolher um destes pares:

| Opção do enunciado | Escolhida |
|---|---|
| Lista vs Árvore de Busca Binária | ✅ |
| Árvore desbalanceada vs AVL | |
| Fila de prioridade sem heap vs com heap | |

A diferença esperada é clara: a lista percorre os elementos um por um, enquanto a BST descarta metade dos candidatos a cada passo.

| Estrutura | Custo da busca | Com 10.000 elementos |
|---|---|---|
| Lista encadeada | **O(n)** | até 10.000 comparações (≈ 5.000 em média) |
| BST balanceada | **O(log n)** | ≈ 14 comparações (`log₂ 10.000 ≈ 13,3`) |

- **O(n):** o número de comparações cresce junto com o tamanho da estrutura. Com 1.000 elementos, no pior caso são 1.000 comparações.
- **O(log n):** cada comparação elimina metade dos candidatos restantes. Com 1.000 elementos são ~10 comparações (`log₂ 1.000 ≈ 10`); com 1.000.000, apenas ~20.

> ⚠️ O(log n) só vale se a árvore estiver **balanceada**. Com valores inseridos em ordem aleatória, a BST fica "quase" balanceada: a busca custa em média ~1,39 × log₂ n, por isso aparecem valores entre 6 e 27 no gráfico, e não sempre 14. Se os valores chegassem em **ordem crescente**, a BST viraria uma lista e a busca voltaria a ser O(n). É isso que a opção "Árvore desbalanceada vs AVL" explora.

---

## 2. As estruturas

### Lista encadeada: `list_node`

Cada nó guarda um valor e um ponteiro para o próximo.

```c
typedef struct list_node {
    int value;              // Valor armazenado
    struct list_node *next; // Ponteiro para o próximo nó
} list_node;
```

O `list_add` insere **no início** da lista (custo O(1)), então a lista fica na **ordem inversa** da inserção. Inserindo `34, 23, 47, 10, 55`:

```
head → [55] → [10] → [47] → [23] → [34] → NULL
```

A busca começa sempre no `head` e avança um nó por vez até achar o valor ou chegar ao `NULL`.

### Árvore de Busca Binária: `bst_node`

Cada nó guarda um valor e dois ponteiros: filho esquerdo (valores **menores**) e filho direito (valores **maiores**).

```c
typedef struct bst_node {
    int value;              // Valor armazenado
    struct bst_node *left;  // Ponteiro para o filho esquerdo
    struct bst_node *right; // Ponteiro para o filho direito
} bst_node;
```

Inserindo os mesmos `34, 23, 47, 10, 55`:

```
        34
       /  \
     23    47
    /        \
  10          55
```

A busca compara o valor com o nó atual e desce pela esquerda ou pela direita, descartando a outra subárvore inteira.

### Funções

| Função | O que faz |
|---|---|
| `list_add` | cria um nó e o coloca no início da lista |
| `list_search` | percorre a lista contando comparações |
| `list_free` | libera todos os nós da lista |
| `bst_add` | desce pela árvore e insere o valor numa folha |
| `bst_search` | desce pela árvore contando comparações |
| `bst_free` | libera a árvore em pós-ordem (filhos antes do pai) |

> 💡 **Valores repetidos:** o `bst_add` ignora um valor que já existe (não é `<` nem `>`), enquanto a lista guarda todos. Na busca, a lista para na primeira ocorrência que encontrar.

---

## 3. Contagem de comparações

Cada função de busca recebe um contador e o incrementa a cada nó visitado. O retorno é o total de comparações até achar o número, ou `-1` se ele não existir.

```c
int list_search(list_node *node, int value, int counter);
int bst_search(bst_node *node, int value, int counter);
```

As duas são chamadas com `counter = 0` e incrementam **antes** de comparar, então o primeiro nó visitado já conta como `1`.

Usando as estruturas do exemplo acima:

| Procurando | Caminho na lista | Lista | Caminho na BST | BST |
|---|---|---|---|---|
| `55` | `55` | 1 | `34 → 47 → 55` | 3 |
| `10` | `55 → 10` | 2 | `34 → 23 → 10` | 3 |
| `34` | `55 → 10 → 47 → 23 → 34` | 5 | `34` | 1 |

> A lista pode ganhar quando o número está perto do `head`. Com 10.000 elementos isso quase nunca acontece: em média ele está no meio.

> 💡 O número sorteado sai do próprio vetor `values[]` (`values[rand() % N]`), então ele **sempre existe** nas duas estruturas e a busca nunca devolve `-1`.

---

## 4. Aleatoriedade

Computadores são determinísticos: dado o mesmo ponto de partida, sempre produzem o mesmo resultado. A solução são **números pseudoaleatórios**, gerados por uma fórmula matemática cuja sequência *parece* aleatória, mas depende de um valor inicial chamado **semente (seed)**.

### `srand`: define a semente

```c
srand(time(NULL));
```

- `time(NULL)` retorna o horário atual em segundos desde 01/01/1970.
- Como o horário muda a cada segundo, a semente é diferente a cada execução.
- É chamado **uma única vez** no início do programa, como embaralhar o baralho antes de jogar.
- `srand(42)` (semente fixa) geraria sempre a mesma sequência: útil para testes, ruim para a apresentação.

### `rand`: sorteia o próximo número

```c
int numero = rand() % MAX_NUMBER;
```

- Retorna um inteiro entre `0` e `RAND_MAX`, uma constante de `<stdlib.h>` que vale **`32767` no Windows** e `2147483647` no Linux (glibc).
- `% MAX_NUMBER` pega o resto da divisão, limitando o resultado a `0` até `MAX_NUMBER - 1`.
- Cada chamada avança para o próximo número da sequência.

### Por que `srand` fora do loop e `rand` dentro?

```c
srand(time(NULL)); // uma vez só: define a sequência

for (int i = 0; i < N; i++) {
    int num = rand() % MAX_NUMBER; // avança a sequência a cada iteração
}
```

Se `srand` estivesse dentro do loop, a semente seria reiniciada a cada iteração. Como `time(NULL)` só muda a cada segundo, num loop rápido a semente seria a mesma várias vezes seguidas, gerando **o mesmo número repetido**.

### Como `rand()` calcula o número: Linear Congruential Generator (LCG)

Os segundos entram **só uma vez**, via `srand`, como ponto de partida. A partir daí, cada `rand()` aplica uma fórmula ao estado anterior. No Windows (MinGW/MSVC) ela é:

```
estado = (estado × 214013 + 2531011) % 2³²
rand() = (estado >> 16) & 0x7FFF
```

Exemplo com `srand(1)`, conferido rodando o compilador desta máquina:

| Chamada | Novo estado | `>> 16` | `& 0x7FFF` = `rand()` |
|---|---|---|---|
| 1ª | `1 × 214013 + 2531011 = 2.745.024` | 41 | **41** |
| 2ª | `2.745.024 × 214013 + 2531011 = 587.473.352.323`, `% 2³²` = `3.357.800.067` | 51.235 | **18467** |
| 3ª | `% 2³²` = `415.139.642` | 6.334 | **6334** |

| Parte | Por quê |
|---|---|
| `% 2³²` | o estado cabe sempre em um inteiro de 32 bits (o estouro "dá a volta") |
| `>> 16` | os bits mais baixos de um LCG são pouco aleatórios (o último bit só alterna 0, 1, 0, 1...), então são descartados |
| `& 0x7FFF` | guarda só 15 bits, por isso o maior valor possível é `2¹⁵ − 1 = 32767` |

As constantes (`214013`, `2531011`) são escolhidas para a sequência demorar o máximo possível para se repetir e espalhar bem os valores. Outras implementações usam outras constantes: o livro *Numerical Recipes* sugere `1664525` e `1013904223`, e o `rand()` do Linux (glibc) nem usa LCG por padrão.

> ⚠️ **Consequência no Windows:** como `rand()` nunca passa de `32767`, o `rand() % MAX_NUMBER` com `MAX_NUMBER = 100000` também não passa. No `results.csv` atual, o maior número sorteado é `32727`. Com 10.000 valores em apenas 32.768 possíveis, vários se repetem, e a BST acaba com menos nós que a lista.

**Fontes:**
- Knuth, D. E. *The Art of Computer Programming, Vol. 2: Seminumerical Algorithms*. Addison-Wesley, 1997.
- Press, W. H. et al. *Numerical Recipes in C: The Art of Scientific Computing*. Cambridge University Press, 1992.

---

## 5. Constantes

| Constante | Valor | Descrição |
|---|---|---|
| `N` | 10000 | número de elementos inseridos nas estruturas |
| `MAX_NUMBER` | 100000 | limite dos valores gerados (no Windows, na prática, 32767) |
| `DRAWS` | 100 | número de sorteios |

---

## 6. Requisitos

| Ferramenta | Uso |
|---|---|
| Compilador C (GCC) | compilar e executar o `main.c` |
| R | rodar o `plot.R` e gerar o gráfico |

**GCC:** no Windows, instale via [MinGW](https://www.mingw-w64.org/). Para verificar:
```powershell
gcc --version
```

**R:** baixe em [cran.r-project.org](https://cran.r-project.org/) (Windows > base). Para verificar:
```powershell
Rscript.exe --version
```

---

## 7. Como rodar

**1. Compile e execute o programa C**, que gera o `results.csv`:

```powershell
gcc main.c -o main
./main.exe
```

**2. Rode o script R**, que gera o gráfico:

```powershell
Rscript.exe plot.R
```

O gráfico é salvo como `Rplots.pdf` na mesma pasta.

> ⚠️ **Na apresentação**, tudo deve rodar **na hora**. Em uma linha só:
> ```powershell
> gcc main.c -o main; ./main.exe; Rscript.exe plot.R
> ```

---

## 8. A saída: `results.csv`

Durante a execução, cada sorteio aparece no terminal:

```
Numero sorteado: 11682 | Lista: 9433 comparacoes | BST: 17 comparacoes
```

E é gravado no `results.csv`:

```
drawn_number,list_comparisons,bst_comparisons
11682,9433,17
6434,4191,17
11097,8607,18
...
```

| Coluna | Significado |
|---|---|
| `drawn_number` | número sorteado |
| `list_comparisons` | comparações da lista para achá-lo |
| `bst_comparisons` | comparações da BST para achá-lo |

---

## 9. O gráfico: `plot.R`

```r
dados <- read.csv("results.csv")

plot(1:nrow(dados), dados$list_comparisons,
     type="l", col="red",
     xlab="Sorteio", ylab="Comparações",
     main="Lista vs BST")

lines(1:nrow(dados), dados$bst_comparisons, col="blue")

legend("topright",
       legend=c("Lista", "BST"),
       col=c("red", "blue"), lty=1)
```

Para quem nunca viu R: a linguagem foi feita para análise de dados e gráficos. Ela trabalha com **tabelas** (chamadas de *data frames*) e tem funções prontas para desenhar.

### `read.csv`: lê os dados

```r
dados <- read.csv("results.csv")
```

- `read.csv(...)` **lê um arquivo CSV** e o transforma em uma tabela na memória.
- `<-` é o operador de atribuição do R (equivale ao `=` de outras linguagens).
- `dados` passa a ser a tabela com as colunas `drawn_number`, `list_comparisons` e `bst_comparisons`.
- Uma coluna é acessada pelo nome com `$`: `dados$list_comparisons` devolve todos os valores dela.

### `plot`: desenha a primeira linha (a Lista)

```r
plot(1:nrow(dados), dados$list_comparisons,
     type="l", col="red",
     xlab="Sorteio", ylab="Comparações",
     main="Lista vs BST")
```

| Argumento | Significado |
|---|---|
| `1:nrow(dados)` | eixo X: `nrow()` conta as linhas (sorteios) e `1:n` gera `1, 2, 3, ..., n` |
| `dados$list_comparisons` | eixo Y: comparações da lista em cada sorteio |
| `type="l"` | liga os pontos com uma **linha** (`l` de *line*); sem isso seriam pontos soltos |
| `col="red"` | linha vermelha |
| `xlab`, `ylab`, `main` | rótulos dos eixos e título |

### `lines`: adiciona a segunda linha (a BST)

```r
lines(1:nrow(dados), dados$bst_comparisons, col="blue")
```

- `plot()` cria um gráfico **do zero**; `lines()` **desenha por cima** de um que já existe.
- Por isso a lista usa `plot()` e a BST usa `lines()`: as duas aparecem na mesma figura.

### `legend`: explica as cores

```r
legend("topright",
       legend=c("Lista", "BST"),
       col=c("red", "blue"), lty=1)
```

| Argumento | Significado |
|---|---|
| `"topright"` | posiciona a legenda no canto superior direito |
| `c("Lista", "BST")` | `c()` (de *combine*) junta valores em um vetor: os textos da legenda |
| `col=c("red", "blue")` | associa cada texto a uma cor, na mesma ordem |
| `lty=1` | (*line type*) desenha uma linha contínua ao lado de cada rótulo |

**Resumo:** ler o CSV → desenhar a linha da lista → sobrepor a linha da BST → adicionar a legenda.

### Resultado esperado

Números do `results.csv` atual (mudam a cada execução, por causa do `srand(time(NULL))`):

| | Lista | BST |
|---|---|---|
| Média | 4.637 | 17 |
| Mínimo | 110 | 6 |
| Máximo | 9.914 | 27 |

> ✅ A linha vermelha (lista) fica alta e pula entre quase 0 e `N`, dependendo de onde o número está na lista. A linha azul (BST) fica colada no chão, perto de `log₂ n`. Essa é a diferença entre **O(n)** e **O(log n)** vista no gráfico.

---

## 10. Níveis de desafio 🎯

| Nível | Meta |
|---|---|
| 1 | Implementar a lista e a BST com inserção e busca |
| 2 | Contar as comparações de cada busca |
| 3 | Fazer vários sorteios e gravar os resultados em CSV |
| 4 | Gerar o gráfico em R, **na hora** da apresentação |
