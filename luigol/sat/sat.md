# 🧩 Desafio: SAT Solver

**Disciplina:** Estruturas de Dados / Programação 2 (UFAL, Instituto de Computação)
**Professor:** Márcio Ribeiro
**Aluno:** Luigi Costa Caldas

> **Tarefa (slide do professor):** *Implement a SAT Solver using DIMACS format (.cnf)*

---

## 1. Contexto

Você tem uma fórmula booleana com várias variáveis e quer saber se **existe alguma combinação de Verdadeiro/Falso** que deixe a fórmula inteira verdadeira.

- Se existir, a fórmula é **satisfatível (SAT)**, e você deve mostrar uma atribuição que funcione.
- Se não existir, ela é **insatisfatível (UNSAT)**.

---

## 2. O formato da fórmula: CNF

A fórmula vem sempre em **Forma Normal Conjuntiva (CNF)**, um **E de OUs** (*AND of ORs*):

```
( x1 ∨ ¬x2 )  ∧  ( x2 ∨ x3 )  ∧  ( ¬x1 ∨ ¬x3 )
 └─cláusula─┘     └cláusula┘     └─cláusula──┘
```

| Termo | Significado |
|---|---|
| **Literal** | uma variável ou a negação dela (`x1`, `¬x2`) |
| **Cláusula** | um **OU** de literais; basta **um** literal verdadeiro para ela ser verdadeira |
| **Fórmula** | um **E** de cláusulas; **todas** precisam ser verdadeiras |

Nos slides, a fórmula também aparece como um **conjunto de cláusulas**:

```
(¬A ∨ ¬B) ∧ (A ∨ ¬B) ∧ (¬A ∨ ¬C)   ≡   { ¬A ∨ ¬B,  A ∨ ¬B,  ¬A ∨ ¬C }
```

Com `{A = true, B = false, C = false}`, a fórmula fica verdadeira, então ela é **SAT!**

---

## 3. A entrada: formato DIMACS (`.cnf`)

DIMACS é o formato padrão de arquivo texto usado por praticamente todos os SAT solvers do mundo.

```
c Isto é um comentário
c Pode haver várias linhas de comentário
p cnf 3 3
1 -2 0
2 3 0
-1 -3 0
```

| Linha | Significado |
|---|---|
| `c ...` | **comentário**, deve ser ignorado |
| `p cnf V C` | **cabeçalho**: `V` = número de variáveis, `C` = número de cláusulas |
| `1 -2 0` | uma **cláusula** |

Dentro de uma cláusula:
- As variáveis são numeradas de **1 até V**, então `3` significa `x3`.
- Um **número negativo** é a negação: `-2` significa `¬x2`.
- O **`0` marca o fim** da cláusula. Por isso não existe a variável 0.

O arquivo acima representa exatamente a fórmula do exemplo:

```
 1 -2 0   →  ( x1 ∨ ¬x2)
 2  3 0   →  ( x2 ∨  x3)
-1 -3 0   →  (¬x1 ∨ ¬x3)
```

---

## 4. A saída

O programa deve ler um arquivo DIMACS e imprimir:
- **`SAT`** seguido de uma atribuição válida;
- ou **`UNSAT`**.

**Formato usado no slide do professor** (`1` = verdadeiro, `0` = falso):

```
SAT!
1 = 1
2 = 0
3 = 0
```

> 💡 **Alternativa opcional (padrão das competições de SAT):**
> ```
> s SATISFIABLE
> v 1 -2 -3 0
> ```
> Positivo = Verdadeiro, negativo = Falso.

---

## 5. Conceito-chave do professor: Interpretação Parcial

Uma **interpretação parcial `I`** atribui valor a **apenas algumas** variáveis. Ao substituir esses valores em `F`, existem 3 resultados possíveis:

| Situação | Exemplo do slide | Resultado |
|---|---|---|
| `F` já é **verdadeira** | `F = {x ∨ y ∨ z, ¬y ∨ ¬z}`, `I = {x = true, z = false}` → `{true, true}` | `I ⇒ F` |
| `F` já é **falsa** | `F = {¬x ∨ y, ¬x ∨ z}`, `I = {x = true, z = false}` → `{y, false}` | `I ⇒ ¬F` |
| **Ainda depende** de outras variáveis | `F = {x ∨ y, ¬x ∨ ¬y ∨ z}`, `I = {x = true, z = false}` → `{¬y}` | depende de `y` |

> Basta **uma cláusula falsa** para a fórmula inteira ser falsa.
> A fórmula só é verdadeira quando **todas** as cláusulas forem verdadeiras.

---

## 6. Algoritmo de referência (slide do professor)

```
boolean sat(formula F, partial_interpretation I) {
  if (I ⇒ F)  return true;
  if (I ⇒ ¬F) return false;
  selecione xi não atribuído em I
  if sat(F, I ∪ {xi = true})  return true;
  if sat(F, I ∪ {xi = false}) return true;
  return false;
}
```

> 🌳 **"The algorithm can be based on a Binary Tree!"**
> Cada nível da árvore é uma variável e cada ramo é uma escolha (`T` ou `F`). Quando um ramo torna `F` falsa, ele é **podado** (✗) e o algoritmo volta atrás.

### Exemplo do slide: `{¬x1 ∨ ¬x2, x1 ∨ ¬x2, ¬x1 ∨ ¬x3}`

```
                 x1
              T /
              x2
          T /      \ F
          ✗        x3
                T /   \ F
                ✗      ✅ SAT!
```

| Passo | `I` | Resultado |
|---|---|---|
| 1 | `{x1=T}` | depende de x2 e x3 |
| 2 | `{x1=T, x2=T}` | **falsa** (✗), volta |
| 3 | `{x1=T, x2=F}` | depende de x3 |
| 4 | `{x1=T, x2=F, x3=T}` | **falsa** (✗), volta |
| 5 | `{x1=T, x2=F, x3=F}` | **verdadeira**, ✅ SAT! |

---

## 7. Casos de teste

**Teste 1 (SAT, exemplo do slide):**
```
c comments
p cnf 3 3
-1 -2 0
1 -2 0
-1 -3 0
```
Resposta do slide: `1 = 1, 2 = 0, 3 = 0`.

**Teste 2 (SAT):**
```
p cnf 3 3
1 -2 0
2 3 0
-1 -3 0
```

**Teste 3 (UNSAT, o mais simples possível):**
```
p cnf 1 2
1 0
-1 0
```

**Teste 4 (UNSAT, menos óbvio):**
```
p cnf 2 4
1 2 0
1 -2 0
-1 2 0
-1 -2 0
```

**Teste 5 (cláusula com um literal só):**
```
p cnf 3 3
1 2 3 0
-1 0
-2 0
```

> ⚠️ Pode haver **mais de uma resposta válida** para um caso SAT. Para conferir, não compare com uma resposta fixa: verifique se a sua atribuição satisfaz **todas** as cláusulas.

---

## 8. Níveis de desafio 🎯

| Nível | Meta |
|---|---|
| 1 | Ler e interpretar o arquivo DIMACS corretamente |
| 2 | Avaliar uma interpretação (parcial ou completa): verdadeira, falsa ou indefinida |
| 3 | Implementar o algoritmo do professor (backtracking em árvore binária) |
| 4 | Otimizar com **DPLL** (propagação unitária e literal puro) |

---

## 9. Extensão presente no slide: SMT Solver (LIA)

O último slide também propõe: *Implement a SMT Solver using the Linear Integer Arithmetic (LIA) Theory.*

O exemplo do slide é este:
```
2x + 1 ≤ 9   →  x ≤ 4
x − 2 ≥ 1    →  x ≥ 3
Interseção:  3 ≤ x ≤ 4   →   x ∈ {3, 4}   →  SAT!
```
Para verificar: `x = 3` dá `7 ≤ 9` e `1 ≥ 1`; `x = 4` dá `9 ≤ 9` e `2 ≥ 1`. Ambos verdadeiros.

> ❓ Confirmar com a equipe/professor se essa parte faz parte da entrega.