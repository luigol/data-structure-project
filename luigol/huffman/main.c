#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Malloc and calloc

typedef struct node {
    void *item;
    unsigned long long frequency;
    struct node *next;
    struct node *left;
    struct node *right;
} node;

node * create_new_node(void *item, unsigned long long frequency)
{
    node *new_node = calloc(1, sizeof(node));
    new_node->item = item;
    new_node->frequency = frequency;
    return new_node;
}

node * queue_insertion(node *head, node *new_node) {

    if (head == NULL || new_node->frequency <= head->frequency) {
        new_node->next = head;
        return new_node;
    }

    node *prev_node = head;
    node *curr_node = head->next;

    while (curr_node != NULL && curr_node->frequency < new_node->frequency) {
        prev_node = curr_node;
        curr_node = curr_node->next;
    }

    new_node->next = curr_node;
    prev_node->next = new_node;

    return head;
}

node * create_internal_node(node *left, node *right) {
    node *new_node = create_new_node(NULL, left->frequency + right->frequency);
    new_node->left = left;
    new_node->right = right;
    return new_node;
}

void fill_codes_table(node *root, char *code_table[256], char *path, int depth) {

    if (root->left == NULL) {              // Leaf
        path[depth] = '\0';
        code_table[*(unsigned char *)root->item] = strdup(path); // String duplicate
        return;
    }

    // Backtracking
    path[depth] = '0';
    fill_codes_table(root->left, code_table, path, depth + 1);

    path[depth] = '1';
    fill_codes_table(root->right, code_table, path, depth + 1);
}

int calculate_tree_size(node *root) {

    // Recursividade
    if (root->left == NULL)
    {
        unsigned char byte = *(unsigned char *)root->item;
        if (byte == '*' || byte == '\\') {
            return 2;
        }
        else {
            return 1;
        }
    }
    else {
        return 1 + calculate_tree_size(root->left) + calculate_tree_size(root->right);
    }
}

node * read_tree(FILE *file) {

    int byte = fgetc(file);

    if (byte == '*') {
        node *new_node = create_new_node(NULL, 0);
        new_node->left = read_tree(file);
        new_node->right = read_tree(file);
        return new_node;
    }

    if (byte == '\\') byte = fgetc(file);

    unsigned char *byte_ptr = malloc(sizeof(unsigned char));
    *byte_ptr = byte;
    node *new_node = create_new_node(byte_ptr, 0);
    return new_node;
}

void write_tree(node *curr_node, FILE *output) {

    if (curr_node->left == NULL) {

        unsigned char character = *(unsigned char *)curr_node->item;
        if (character == '*' || character == '\\') fputc('\\', output);
        fputc(character, output);
        return;
    }
    
    fputc('*', output);
    write_tree(curr_node->left, output);
    write_tree(curr_node->right, output);

    return;
}

void free_tree(node *root) {

    if (root == NULL) return;

    // Post-order: free the children before the parent
    free_tree(root->left);
    free_tree(root->right);

    free(root->item); // Leaf: the malloc'd byte. Internal node: NULL, and free(NULL) does nothing
    free(root);
}

void free_codes_table(char *codes_table[256]) {

    for (int i = 0; i < 256; i++) {
        free(codes_table[i]); // Strings from strdup. NULL entries are safe
    }
}

void decode(FILE *file, FILE *output, node *root, int trash) {

    long long curr_pos = ftell(file); // Current cursor position
    fseek(file, 0, SEEK_END); // Move the cursor to the final
    long long end_pos = ftell(file);
    long long total_bits = ((end_pos - curr_pos) * 8) - trash; // Bytes * 8 - trash
    fseek(file, curr_pos, SEEK_SET);

    // Only one distinct byte: the root is a leaf and every bit ("0") is one byte
    if (root->left == NULL) {
        for (long long i = 0; i < total_bits; i++) {
            fputc(*(unsigned char *)root->item, output);
        }
        return;
    }

    long long bits_count = 0;
    node *curr_node = root;
    int byte;

    while ((byte = fgetc(file)) != EOF) {

        for (int i = 0; i < 8; i++) {

            if (bits_count == total_bits) break;

            int bit = (byte >> (7 - i)) & 1;

            if (bit == 0) curr_node = curr_node->left;
            if (bit == 1) curr_node = curr_node->right;

            bits_count++;

            if (curr_node->left == NULL && curr_node->right == NULL) {

                fputc(*(unsigned char*)curr_node->item, output);
                curr_node = root;
            }
        }

        if (bits_count == total_bits) break;
    }
}

void encode(FILE *input, FILE *output, char *codes_table[256]) {

    unsigned char buffer = 0;
    int bits_count = 0;
    int byte;
    int bit;

    while ((byte = fgetc(input)) != EOF) {

        for (int i = 0; codes_table[byte][i] != '\0'; i++) {

            if (codes_table[byte][i] == '1') bit = 1;
            else bit = 0;

            buffer = buffer | bit << (7 - bits_count);

            bits_count++;

            if (bits_count == 8) {
                fputc(buffer, output);
                bits_count = 0;
                buffer = 0;
            }
        }
    }

    if (bits_count != 0) {
        fputc(buffer, output);
    }

    return;
}

void decompress(char *file_path) {

    // 1) Open file
    // The file must end with ".huff"
    int file_path_size = strlen(file_path);
    if (file_path_size <= 5 || strcmp(file_path + file_path_size - 5, ".huff") != 0) {
        printf("ERROR: File must have the .huff extension.\n");
        return;
    }

    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        printf("ERROR: File was not found.\n");
        return;
    }

    // 2) Read file and rebuild the Huffman tree
    unsigned char byte0 = fgetc(file);
    fgetc(file); // Byte 1 (tree size): skipped, the pre-order tree ends by itself

    int trash = byte0 >> 5;

    // Rebuild the tree
    node *root = read_tree(file);

    // 3) Remove ".huff" extension and create output path
    char output_path[512];
    int extension_pos = file_path_size - 5;
    strcpy(output_path, file_path);
    output_path[extension_pos] = '\0';

    FILE *output = fopen(output_path, "wb"); // wb = write binary

    if (output == NULL) {
        printf("ERROR: Could not create the output file.\n");
        fclose(file);
        free_tree(root);
        return;
    }

    // 4) Decode data
    decode(file, output, root, trash);

    // 5) Close files and free memory
    fclose(file);
    fclose(output);

    free_tree(root);

    return;
}

void compress(char *file_path) {

    // 1) Read file
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        printf("ERROR: File was not found.\n");
        return;
    }

    // 2) Frequency and Priority Queue
    unsigned long long freq[256] = {0};

    // On the file: Hi\n
    // At the read binary mode: 01001000  01101001  00001010
    // How the code will read: 72  105  10
    int byte;
    while ((byte = fgetc(file)) != EOF) {
        freq[byte]++;
    }

    // Priority queue
    node *head = NULL;
    for (int i = 0; i < 256; i++)
    {
        if (freq[i] > 0) {
            unsigned char *byte_ptr = malloc(sizeof(unsigned char));
            *byte_ptr = i;
            node *new_node = create_new_node(byte_ptr, freq[i]);
            head = queue_insertion(head, new_node);
        }
    }

    // 3) Huffman tree
    // Queue to Binary Tree
    while (head != NULL && head->next != NULL)
    {
        node *left = head;
        node *right = head->next;
        head = head->next->next;
        node *new_node = create_internal_node(left, right);
        head = queue_insertion(head, new_node);
    }

    node *root = head;
    // Empty file
    if (root == NULL) {
        printf("ERROR: File is empty.\n");
        fclose(file);
        return;
    }

    // 4) Code table
    char *codes_table[256] = {NULL};
    char path[256];

    if (root->left == NULL) {
        // Only one distinct byte: the root is a leaf
        codes_table[*(unsigned char *)root->item] = strdup("0");
    }
    else {
        // Backtracking
        fill_codes_table(root, codes_table, path, 0);
    }

    // 5) Formation
    // Number of padding bits in last byte
    unsigned long long total_bits = 0;
    for (int i = 0; i < 256; i++)
    {
        if (codes_table[i] != NULL)
            total_bits += freq[i] * strlen(codes_table[i]);
    }
    int trash = (8 - (total_bits % 8)) % 8;

    // Tree size: number of bytes the pre-order tree takes in the file (escapes included)
    int tree_size = calculate_tree_size(root);

    // Byte 0 and Byte 1
    unsigned char byte0 = (trash << 5) | (tree_size >> 8);
    unsigned char byte1 = tree_size & 0xFF; // 0x means hexadecimal and FF is 11111111. Also 0xFF = 0b11111111

    // 6) Build the compressed file
    // Create file.huff
    char output_path[512];
    sprintf(output_path, "%s.huff", file_path);

    rewind(file);
    FILE *output = fopen(output_path, "wb"); // wb = write binary

    if (output == NULL) {
        printf("ERROR: Could not create the output file.\n");
        fclose(file);
        free_codes_table(codes_table);
        free_tree(root);
        return;
    }

    // Write Byte 0 and Byte 1
    fputc(byte0, output);
    fputc(byte1, output);

    write_tree(root, output);

    encode(file, output, codes_table);

    // 7) Close files and free memory
    fclose(file);
    fclose(output);

    free_codes_table(codes_table);
    free_tree(root);

    return;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Compress: ./huffman.exe -c <./file.*>\n");
        printf("Decompress: ./huffman.exe -d <./file.*>\n");
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0) // strcmp (string compare) returns 0 for two identical strings
    {
        compress(argv[2]);
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        decompress(argv[2]);
    }

    return 0;
}
