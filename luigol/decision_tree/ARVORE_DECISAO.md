# 🌳 Desafio: Árvore de Decisão com Entropia (ID3)

**Disciplina:** Estruturas de Dados / Programação 2 (ECOM008), UFAL, Instituto de Computação
**Professor:** Márcio Ribeiro
**Aluno:** Luigi Costa Caldas

> **Tarefa (documento do projeto, módulo 1):** implementar DOIS algoritmos ou estruturas de dados com CÓDIGO. Temas da equipe: **Árvores de Decisão com Entropia** e **Segment Tree**. Apresentação dos dois com, no máximo, **10 slides**.
>
> **Linguagem:** C.

---

## 1. Contexto

Você tem uma **tabela de exemplos já classificados** e quer que o programa **descubra sozinho as regras** por trás deles, na forma de uma árvore de perguntas. Depois, a árvore é usada para classificar **casos novos**.

```
"Vou jogar bola hoje?"

              [Tempo?]
           /     |      \
     Ensolarado Nublado  Chuvoso
         |        |        |
   [Umidade?]    SIM    [Vento?]
     /    \              /    \
  Alta   Normal       Forte  Fraco
   |       |            |      |
  NÃO     SIM          NÃO    SIM
```

| Termo | Significado |
|---|---|
| **Atributo** | uma coluna usada para decidir (Tempo, Umidade...) |
| **Classe** | a coluna que se quer prever (Jogar: Sim/Não) |
| **Nó interno** | uma pergunta sobre um atributo |
| **Ramo** | um valor possível daquele atributo |
| **Folha** | a resposta final (uma classe) |

> 💡 **Referência externa:** no Python, a biblioteca **scikit-learn** tem o `DecisionTreeClassifier`. Com `criterion="entropy"`, ela usa o mesmo critério deste projeto. Não será usada aqui, mas serve para conferir resultados.

---

## 2. O problema central: qual pergunta fazer primeiro?

### Entropia: o quanto um grupo está "bagunçado"

$$H(S) = -\sum_{i} p_i \log_2 p_i$$

onde $p_i$ é a proporção de cada classe no grupo $S$.

```
🟢🟢🟢🟢  (100% Sim)      →  H = 0      puro, nenhuma dúvida
🟢🟢🔴🔴  (50% / 50%)    →  H = 1      confusão máxima
🟢🟢🟢🔴  (75% / 25%)    →  H ≈ 0,811
```

> Por convenção, $0 \cdot \log_2 0 = 0$, ou seja, uma classe ausente não contribui.

### Ganho de informação: o quanto uma pergunta "arruma" o grupo

$$Ganho(S, A) = H(S) - \sum_{v \in valores(A)} \frac{|S_v|}{|S|} \, H(S_v)$$

Em palavras: a entropia **antes** da pergunta menos a entropia **média depois** dela, ponderada pelo tamanho de cada subgrupo $S_v$.

> **O atributo com maior ganho vira a pergunta do nó.**

---

## 3. O algoritmo ID3 (visão geral)

```
construir(exemplos, atributos disponíveis):

  se todos os exemplos têm a mesma classe
      → folha com essa classe

  se não sobrou atributo para perguntar
      → folha com a classe mais frequente (maioria)

  senão
      → escolhe o atributo de MAIOR ganho
      → cria um nó com ele
      → para cada valor desse atributo:
            filho = construir(exemplos com esse valor,
                              atributos sem o escolhido)
```

---

## 4. Entrada: arquivo CSV

- A **primeira linha** é o cabeçalho, com os nomes das colunas.
- As colunas são separadas por **vírgula** ou **ponto e vírgula** (o Excel em português salva o CSV com `;`).
- A **última coluna** é sempre a **classe**.
- Os valores são **categóricos** (texto), sem números contínuos.
- Nenhum campo pode ficar vazio: se faltar um valor, o programa avisa a linha e para.

---

## 5. Saída

O programa deve:
1. **Imprimir a árvore** construída, em qualquer formato legível, por exemplo com indentação;
2. **Classificar exemplos novos** que não estão na tabela.

---

## 6. Caso de teste: `jogar.csv`

É o exemplo clássico da literatura (Quinlan, 1986), com 14 dias. O arquivo está na pasta do projeto.

```csv
Tempo,Temperatura,Umidade,Vento,Jogar
Ensolarado,Quente,Alta,Fraco,Nao
Ensolarado,Quente,Alta,Forte,Nao
Nublado,Quente,Alta,Fraco,Sim
Chuvoso,Amena,Alta,Fraco,Sim
Chuvoso,Fria,Normal,Fraco,Sim
Chuvoso,Fria,Normal,Forte,Nao
Nublado,Fria,Normal,Forte,Sim
Ensolarado,Amena,Alta,Fraco,Nao
Ensolarado,Fria,Normal,Fraco,Sim
Chuvoso,Amena,Normal,Fraco,Sim
Ensolarado,Amena,Normal,Forte,Sim
Nublado,Amena,Alta,Forte,Sim
Nublado,Quente,Normal,Fraco,Sim
Chuvoso,Amena,Alta,Forte,Nao
```

> Sem acentos de propósito: em C, letras acentuadas ocupam mais de um byte e complicam a comparação de strings.

### Valores esperados para conferir

**Entropia do conjunto inteiro** (9 Sim, 5 Não): **H ≈ 0,940**

**Ganho de cada atributo na raiz:**

| Atributo | Ganho |
|---|---|
| **Tempo** | **≈ 0,247** ← vencedor |
| Umidade | ≈ 0,152 |
| Vento | ≈ 0,048 |
| Temperatura | ≈ 0,029 |

**A árvore final** deve ser exatamente a do desenho da seção 1.

**Classificações para testar:**

| Tempo | Temperatura | Umidade | Vento | Esperado |
|---|---|---|---|---|
| Ensolarado | Fria | Alta | Forte | Nao |
| Chuvoso | Quente | Normal | Fraco | Sim |
| Nublado | Fria | Alta | Forte | Sim |

---

## 7. 🤔 Para pensar antes de programar

1. **E se o CSV tivesse uma coluna `Dia` (1, 2, 3... 14)?** Calcule mentalmente o ganho dela. Por que o algoritmo a escolheria, e por que essa seria uma árvore inútil?
2. **Valores como texto:** comparar strings em C é caro e chato. Como você poderia transformar `"Ensolarado"`, `"Nublado"` e `"Chuvoso"` em números logo na leitura?
3. **Um valor sem exemplos:** ao dividir um subgrupo, um valor pode não ter nenhum exemplo. O que a folha desse ramo deve responder?

---

## 8. Níveis de desafio 🎯

| Nível | Meta |
|---|---|
| 1 | Ler o CSV e guardar a tabela em memória |
| 2 | Calcular a entropia de um conjunto de exemplos |
| 3 | Calcular o ganho de cada atributo, e bater os valores da seção 6 |
| 4 | Construir a árvore recursivamente |
| 5 | Imprimir a árvore e classificar exemplos novos |

---

## 9. Referência

- Quinlan, J. R. (1986). *Induction of Decision Trees*. Machine Learning, 1, 81–106.
