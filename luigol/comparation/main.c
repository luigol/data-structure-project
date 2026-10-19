#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10000 // Número total de valores
#define MAX_NUMBER 100000 // Maior número
#define DRAWS 100 // Número de sorteios

// List

typedef struct list_node {
    int value;
    struct list_node *next;
} list_node;

list_node * list_add(list_node *node, int value)
{
    list_node *new_node = (list_node*)(malloc(sizeof(list_node)));
    new_node->value = value;
    new_node->next = node;
    return new_node;
}

int list_search(list_node *node, int value, int counter)
{
    if (node == NULL) return -1;

    counter++;

    if (value == node->value) return counter;

    return list_search(node->next, value, counter);
}

void list_free(list_node *node)
{
    while (node != NULL)
    {
        list_node *next = node->next;
        free(node);
        node = next;
    }
}

// Binary Search Tree

typedef struct bst_node {
    int value;
    struct bst_node *left;
    struct bst_node *right;
} bst_node;

bst_node * bst_add(bst_node *node, int value)
{
    if (node == NULL)
    {
        bst_node *new_node = (bst_node*)(malloc(sizeof(bst_node)));
        new_node->value = value;
        new_node->left = NULL;
        new_node->right = NULL;
        return new_node;
    }

    if (value < node->value)
    {
        node->left = bst_add(node->left, value);
    }
    else if (value > node->value) 
    {
        node->right = bst_add(node->right, value);
    }

    return node;
}

int bst_search(bst_node *node, int value, int counter)
{
    if (node == NULL) return -1;

    counter++;

    if (value == node->value) return counter;

    if (value < node->value)
    {
        return bst_search(node->left, value, counter);
    }
    else if (value > node->value)
    {
        return bst_search(node->right, value, counter);
    }
    
}

void bst_free(bst_node *node)
{
    if (node == NULL) return;
    bst_free(node->left);
    bst_free(node->right);
    free(node);
}


// Main

int main()
{

    list_node *head = NULL;
    bst_node *root = NULL;

    int values[N];

    srand(time(NULL));

    for (int i = 0; i < N; i++) {
        values[i] = rand() % MAX_NUMBER;
        head = list_add(head, values[i]);
        root = bst_add(root, values[i]);
    }

    FILE *file = fopen("results.csv", "w");
    fprintf(file, "drawn_number,list_comparisons,bst_comparisons\n");

    for (int i = 0; i < DRAWS; i++) {
        int random = values[rand() % N];
        int list_counter = list_search(head, random, 0);
        int bst_counter = bst_search(root, random, 0);
        printf("Numero sorteado: %d | Lista: %d comparacoes | BST: %d comparacoes\n", random, list_counter, bst_counter);
        fprintf(file, "%d,%d,%d\n", random, list_counter, bst_counter);
    }

    fclose(file);

    list_free(head);
    bst_free(root);

}