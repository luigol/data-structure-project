#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10000 // Number of generated numbers
#define  MAX_NUMBER 100000 // Biggest generated number
#define DRAWS 100 // Number of draws

typedef struct node {
    int number;
    struct node *next;
    struct node *left;
    struct node *right;
} node;

node *list_insert(node *head, int number) {
    
    node *new_node = calloc(1, sizeof(node));
    new_node->number = number;
    new_node->next = head;
    return new_node;
}

node *bst_insert(node *curr_node, int number) {

    if (curr_node == NULL) {
        node *new_node = calloc(1, sizeof(node));
        new_node->number = number;
        return new_node;
    }

    if (number > curr_node->number) {
        curr_node->right = bst_insert(curr_node->right, number);
    }
    else if (number < curr_node->number) {
        curr_node->left = bst_insert(curr_node->left, number);
    }

    return curr_node;
}

int list_search(node *curr_node, int random, int counter) {
    
    if (curr_node == NULL) return -1;
    if (curr_node->number == random) return counter + 1;
    return list_search(curr_node->next, random, counter + 1);
}

int bst_search(node *curr_node, int random, int counter) {

    if (curr_node == NULL) return -1;
    if (curr_node->number == random) return counter + 1;
    if (random > curr_node->number) return bst_search(curr_node->right, random, counter + 1);
    if (random < curr_node->number) return bst_search(curr_node->left, random, counter + 1);
}

void list_free(node *curr_node)
{
    while (curr_node != NULL)
    {
        node *next = curr_node->next;
        free(curr_node);
        curr_node = next;
    }
}

void bst_free(node *curr_node)
{
    if (curr_node == NULL) return;
    bst_free(curr_node->left);
    bst_free(curr_node->right);
    free(curr_node);
}

int main() {

    // 1) Generate numbers and set them into an array
    srand(time(NULL)); // Seed = time

    int arr[N];

    for (int i = 0; i < N; i++) {
        arr[i] = rand() % MAX_NUMBER;
    }

    // 2) Insertion into a binary search tree and a linked list at the same time

    node *head = NULL;
    node *root = NULL;

    for (int i = 0; i < N; i++) {
        root = bst_insert(root, arr[i]);
        head = list_insert(head, arr[i]);
    }

    // 3) Draw array numbers and search them in the data structures

    FILE *file = fopen("results.csv", "w");
    fprintf(file, "drawn_number,list_comparisons,bst_comparisons\n");

    printf("Loading...\n");
    for (int i = 0; i < DRAWS; i++) {
        int random = arr[rand() % N];
        int list_counter = list_search(head, random, 0);
        int bst_counter = bst_search(root, random, 0);
        fprintf(file, "%d,%d,%d\n", random, list_counter, bst_counter);
    }

    fclose(file);

    printf("End.\n");

    list_free(head);
    bst_free(root);

    // Rscript.exe plot.R

    return 0;
}