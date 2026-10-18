#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE   1024
#define SEPARATORS ",;"   // Excel in Portuguese saves the CSV with ';'
#define SHOWN_ROWS 10     // Rows of the encoded table printed on the screen

typedef struct node {
    int attribute;            // Column this node asks about (-1 if leaf)
    int value;                // Class answered by a leaf
    struct node **children;   // One child per value of the attribute (NULL if leaf)
} node;

/**
 * Dataset read from a CSV file, with every value encoded as an integer.
 * Each column has its own dictionary: dict[col][code] is the original
 * text of the value. The last column is the class.
 */
typedef struct dataset {
    int n_cols;          // Number of columns (attributes + class)
    int n_rows;          // Number of examples (header not included)
    char **col_names;    // col_names[col] = column name from the header
    char ***dict;        // dict[col][code] = original text of the value
    int *n_values;       // n_values[col] = distinct values in the column
    int **data;          // data[row][col] = encoded value
} dataset;

// Removes the trailing newline ("\n" or "\r\n") from a line read by fgets.
void strip_newline(char *line) {
    line[strcspn(line, "\r\n")] = '\0';
}

// Counts how many columns a CSV line has (number of separators + 1).
int count_columns(char *line) {
    int n_cols = 1;

    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == ',' || line[i] == ';') n_cols++;
    }

    return n_cols;
}

// Counts the examples of the CSV file: every non-blank line after the header.
// At the end, the file is rewound to the beginning.
int count_rows(FILE *file) {
    char line[MAX_LINE];
    int n_rows = 0;

    fgets(line, MAX_LINE, file); // Skip the header

    while (fgets(line, MAX_LINE, file) != NULL) {
        strip_newline(line);
        if (line[0] != '\0') n_rows++;
    }

    rewind(file);
    return n_rows;
}

// Returns the numeric code of a value in a column, or -1 if it was never seen.
int find_code(dataset *ds, int col, char *value) {
    for (int code = 0; code < ds->n_values[col]; code++) {
        if (strcmp(ds->dict[col][code], value) == 0) return code;
    }

    return -1;
}

// Returns the numeric code of a value in a column.
// If the value was never seen in that column, it receives the next free code.
int get_code(dataset *ds, int col, char *value) {

    // Already in the dictionary?
    int code = find_code(ds, col, value);
    if (code != -1) return code;

    // New value: store a copy in the next free position of the dictionary
    int new_code = ds->n_values[col];
    ds->dict[col][new_code] = strdup(value);
    ds->n_values[col]++;

    return new_code;
}

// Reads the whole CSV file: header first, then every row encoded.
// Returns 0 if the file is not a valid table (the error is printed).
int read_csv(FILE *file, dataset *ds) {
    char line[MAX_LINE];
    char *token;

    // First pass: how many examples?
    ds->n_rows = count_rows(file);
    if (ds->n_rows == 0) {
        printf("ERROR: The file has no examples.\n");
        return 0;
    }

    // Header: column names
    fgets(line, MAX_LINE, file);
    strip_newline(line);

    ds->n_cols = count_columns(line);
    if (ds->n_cols < 2) {
        printf("ERROR: The table needs at least 2 columns (attributes and class).\n");
        return 0;
    }

    ds->col_names = calloc(ds->n_cols, sizeof(char *));
    ds->dict      = calloc(ds->n_cols, sizeof(char **));
    ds->n_values  = calloc(ds->n_cols, sizeof(int));

    token = strtok(line, SEPARATORS); // Cuts the line at the first separator
    for (int col = 0; col < ds->n_cols; col++) {
        if (token == NULL) {
            printf("ERROR: The header has an empty column name.\n");
            return 0;
        }

        ds->col_names[col] = strdup(token);

        // A column can never have more distinct values than rows
        ds->dict[col] = calloc(ds->n_rows, sizeof(char *));

        token = strtok(NULL, SEPARATORS); // It continues from where it had left off
    }

    // Second pass: one example per line
    ds->data = calloc(ds->n_rows, sizeof(int *));
    int row = 0;
    int line_number = 1; // The header was line 1

    while (fgets(line, MAX_LINE, file) != NULL) {
        line_number++;
        strip_newline(line);
        if (line[0] == '\0') continue; // Skip blank lines

        ds->data[row] = calloc(ds->n_cols, sizeof(int));

        token = strtok(line, SEPARATORS);
        for (int col = 0; col < ds->n_cols; col++) {
            if (token == NULL) {
                printf("ERROR: Line %d has a missing value.\n", line_number);
                return 0;
            }

            ds->data[row][col] = get_code(ds, col, token);
            token = strtok(NULL, SEPARATORS);
        }

        row++;
    }

    return 1;
}

// Prints every column dictionary and the first rows of the encoded table.
void print_dataset(dataset *ds) {
    printf("Columns: %d | Rows: %d\n\n", ds->n_cols, ds->n_rows);

    printf("Dictionaries:\n");
    for (int col = 0; col < ds->n_cols; col++) {
        printf("  %-12s", ds->col_names[col]);
        for (int code = 0; code < ds->n_values[col]; code++) {
            printf(" %d=%s", code, ds->dict[col][code]);
        }
        printf("\n");
    }

    printf("\nEncoded data:\n");
    for (int row = 0; row < ds->n_rows && row < SHOWN_ROWS; row++) {
        printf("  ");
        for (int col = 0; col < ds->n_cols; col++) {
            printf("%d ", ds->data[row][col]);
        }
        printf("\n");
    }

    if (ds->n_rows > SHOWN_ROWS) {
        printf("  ... (%d more rows)\n", ds->n_rows - SHOWN_ROWS);
    }
}

// Frees all the memory allocated by read_csv.
void free_dataset(dataset *ds) {
    for (int col = 0; col < ds->n_cols; col++) {
        for (int code = 0; code < ds->n_values[col]; code++) {
            free(ds->dict[col][code]);
        }
        free(ds->dict[col]);
        free(ds->col_names[col]);
    }
    free(ds->dict);
    free(ds->col_names);
    free(ds->n_values);

    for (int row = 0; row < ds->n_rows; row++) {
        free(ds->data[row]);
    }
    free(ds->data);
}

/**
 * From here on, a "group" of examples is an array with the indexes of its
 * rows in ds->data (ex.: rows = {0, 3, 7}, n_rows = 3).
 * Splitting a group copies only indexes, never the table.
 */

// Fills count[class] with how many examples of each class the group has.
void count_classes(dataset *ds, int *rows, int n_rows, int *count) {
    int class_col = ds->n_cols - 1; // The class is the last column

    for (int class = 0; class < ds->n_values[class_col]; class++) {
        count[class] = 0;
    }

    for (int i = 0; i < n_rows; i++) {
        int class = ds->data[rows[i]][class_col];
        count[class]++;
    }
}

// H(S) = -sum(p * log2(p)), where p is the proportion of each class in the group.
double entropy(dataset *ds, int *rows, int n_rows) {
    int n_classes = ds->n_values[ds->n_cols - 1];
    int count[n_classes];
    count_classes(ds, rows, n_rows, count);

    double h = 0.0;
    for (int class = 0; class < n_classes; class++) {
        if (count[class] == 0) continue; // By convention, 0 * log2(0) = 0

        double p = (double) count[class] / n_rows;
        h -= p * log2(p);
    }

    return h;
}

// Most frequent class of the group (in a tie, the lowest code).
int majority_class(dataset *ds, int *rows, int n_rows) {
    int n_classes = ds->n_values[ds->n_cols - 1];
    int count[n_classes];
    count_classes(ds, rows, n_rows, count);

    int majority = 0;
    for (int class = 1; class < n_classes; class++) {
        if (count[class] > count[majority]) majority = class;
    }

    return majority;
}

int same_class(dataset *ds, int *rows, int n_rows) {
    int class_col = ds->n_cols - 1;
    int first = ds->data[rows[0]][class_col];

    for (int i = 1; i < n_rows; i++) {
        if (ds->data[rows[i]][class_col] != first) return 0;
    }

    return 1;
}

// Copies to `subset` only the rows where `attribute` == `value`. Returns |Sv|.
int filter_rows(dataset *ds, int *rows, int n_rows, int attribute, int value, int *subset) {
    int n_subset = 0;

    for (int i = 0; i < n_rows; i++) {
        if (ds->data[rows[i]][attribute] == value) {
            subset[n_subset] = rows[i];
            n_subset++;
        }
    }

    return n_subset;
}

// Gain(S, A) = H(S) - sum(|Sv| / |S| * H(Sv)), with one Sv per value v of A.
double information_gain(dataset *ds, int *rows, int n_rows, int attribute) {
    int *subset = malloc(n_rows * sizeof(int));
    double gain = entropy(ds, rows, n_rows);

    for (int value = 0; value < ds->n_values[attribute]; value++) {
        int n_subset = filter_rows(ds, rows, n_rows, attribute, value, subset);
        double weight = (double) n_subset / n_rows;

        gain -= weight * entropy(ds, subset, n_subset);
    }

    free(subset);
    return gain;
}

// Attribute not used yet with the highest gain (-1 if every one was used).
int best_attribute(dataset *ds, int *rows, int n_rows, int *used) {
    int best = -1;
    double best_gain = -1.0;

    for (int a = 0; a < ds->n_cols - 1; a++) {
        if (used[a]) continue;

        double gain = information_gain(ds, rows, n_rows, a);
        if (gain > best_gain) {
            best_gain = gain;
            best = a;
        }
    }

    return best;
}

// Leaf: attribute = -1 and value = class. Question: attribute = column and value = -1.
node * create_node(int attribute, int value) {
    node *new_node = calloc(1, sizeof(node)); // children starts as NULL
    new_node->attribute = attribute;
    new_node->value = value;
    return new_node;
}

// ID3. used[a] = 1 if the attribute a was already asked on the path from the root.
node * build_tree(dataset *ds, int *rows, int n_rows, int *used) {
    int majority = majority_class(ds, rows, n_rows);

    // Case 1: every example has the same class -> leaf with that class
    if (same_class(ds, rows, n_rows)) return create_node(-1, majority);

    // Case 2: no attribute left to ask about -> leaf with the majority class
    int best = best_attribute(ds, rows, n_rows, used);
    if (best == -1) return create_node(-1, majority);

    // Case 3: question about the attribute with the highest gain,
    // with one child for each of its values
    node *tree = create_node(best, -1);
    tree->children = calloc(ds->n_values[best], sizeof(node *));

    used[best] = 1; // The children can't ask about it again
    int *subset = malloc(n_rows * sizeof(int));

    for (int value = 0; value < ds->n_values[best]; value++) {
        int n_subset = filter_rows(ds, rows, n_rows, best, value, subset);

        if (n_subset == 0) {
            // No example with this value: answer the majority of the parent
            tree->children[value] = create_node(-1, majority);
        } else {
            tree->children[value] = build_tree(ds, subset, n_subset, used);
        }
    }

    free(subset);
    used[best] = 0; // Backtracking: available again for the other branches
    return tree;
}

// Prints the tree with indentation: one level deeper for each question.
void print_tree(dataset *ds, node *tree, int depth) {

    if (tree->attribute == -1) { // Leaf
        printf("%s\n", ds->dict[ds->n_cols - 1][tree->value]);
        return;
    }

    printf("[%s?]\n", ds->col_names[tree->attribute]);

    for (int value = 0; value < ds->n_values[tree->attribute]; value++) {
        for (int i = 0; i <= depth; i++) printf("    ");
        printf("%s -> ", ds->dict[tree->attribute][value]);

        print_tree(ds, tree->children[value], depth + 1);
    }
}

// Walks down from the root following the example's answers until a leaf.
int classify(node *tree, int *example) {
    while (tree->attribute != -1) {
        int answer = example[tree->attribute];
        tree = tree->children[answer];
    }

    return tree->value;
}

// Turns a typed example (ex.: "Ensolarado,Fria,Alta,Forte") into codes.
// Returns 0 if some value is missing or never appeared in the table.
int encode_example(dataset *ds, char *line, int *example) {
    char *token = strtok(line, SEPARATORS);

    for (int col = 0; col < ds->n_cols - 1; col++) {
        if (token == NULL) return 0;

        example[col] = find_code(ds, col, token);
        if (example[col] == -1) return 0;

        token = strtok(NULL, SEPARATORS);
    }

    return 1;
}

void free_tree(dataset *ds, node *tree) {

    // Post-order: free the children before the parent
    if (tree->attribute != -1) {
        for (int value = 0; value < ds->n_values[tree->attribute]; value++) {
            free_tree(ds, tree->children[value]);
        }
        free(tree->children);
    }

    free(tree);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s <file.csv>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("ERROR: File was not found.\n");
        return 1;
    }

    dataset ds;
    int valid = read_csv(file, &ds);
    fclose(file);

    if (valid == 0) return 1;

    print_dataset(&ds);

    // At the root, the group has every example and no attribute was used yet
    int *rows = calloc(ds.n_rows, sizeof(int));
    for (int i = 0; i < ds.n_rows; i++) {
        rows[i] = i;
    }

    int n_attributes = ds.n_cols - 1;
    int *used = calloc(n_attributes, sizeof(int));

    // Entropy and gains at the root, to check the expected values
    printf("\nEntropy of the whole set: %.3f\n", entropy(&ds, rows, ds.n_rows));
    printf("Gain of each attribute at the root:\n");
    for (int a = 0; a < n_attributes; a++) {
        printf("  %-12s %.3f\n", ds.col_names[a], information_gain(&ds, rows, ds.n_rows, a));
    }

    node *root = build_tree(&ds, rows, ds.n_rows, used);

    printf("\nDecision tree:\n");
    print_tree(&ds, root, 0);

    // Classifies new examples typed by the user, one per line
    char line[MAX_LINE];
    int example[n_attributes];

    printf("\nType new examples in this order (empty line to quit):\n  ");
    for (int a = 0; a < n_attributes; a++) {
        if (a > 0) printf(",");
        printf("%s", ds.col_names[a]);
    }
    printf("\n");

    while (fgets(line, MAX_LINE, stdin) != NULL) {
        strip_newline(line);
        if (line[0] == '\0') break;

        if (encode_example(&ds, line, example)) {
            int class = classify(root, example);
            printf("  -> %s\n", ds.dict[ds.n_cols - 1][class]);
        } else {
            printf("  Invalid example: missing or unknown value.\n");
        }
    }

    free_tree(&ds, root);
    free(used);
    free(rows);
    free_dataset(&ds);
    return 0;
}
