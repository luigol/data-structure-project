#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE 1024

typedef struct node {
    int attribute;
    char **values;
    struct node **children;
    char *class;
} node;

typedef struct dataset {
    int n_rows;
    int n_cols;
    char ***sheet; // sheet[row][col] = text of the cell. Row 0 is the header
} dataset;

void read_csv(FILE *file, dataset *ds) {

    char line[MAX_LINE];
    int n_rows = 0;
    while (fgets(line, MAX_LINE, file) != NULL) n_rows++;

    rewind(file);

    ds->n_rows = n_rows;
    ds->sheet = malloc(ds->n_rows * sizeof(char **));

    for (int row = 0; row < ds->n_rows; row++) {
        fgets(line, MAX_LINE, file);
        line[strcspn(line, "\r\n")] = '\0'; // Strcspn return the position of "\r\n".

        if (row == 0) {

            int n_cols = 1;
            for (int i = 0; line[i] != '\0'; i++) {
                if (line[i] == ',') n_cols++;
            }
            ds->n_cols = n_cols;
        }
        ds->sheet[row] = malloc(ds->n_cols * sizeof(char *));

        char *token = strtok(line, ",");
        for (int col = 0; col < ds->n_cols; col++) {
            ds->sheet[row][col] = strdup(token); // Copy: line is reused
            token = strtok(NULL, ",");
        }
    }
}

void free_dataset(dataset *ds) {
    for (int row = 0; row < ds->n_rows; row++) {
        for (int col = 0; col < ds->n_cols; col++) {
            free(ds->sheet[row][col]);
        }
        free(ds->sheet[row]);
    }
    free(ds->sheet);
}

// Fill the array with the different values rows
char ** distinct_values(dataset *ds, int *set, int n_set, int attribute) {

    char **values = malloc((n_set + 1) * (sizeof(char *)));

    int n_values = 0;
    for (int i = 0; i < n_set; i++) {

        char *value = ds->sheet[set[i]][attribute];

        int seen = 0;
        for (int j = 0; j < i; j++) {
            if ((strcmp(value, ds->sheet[set[j]][attribute])) == 0) seen = 1;
        }

        if (seen) continue;

        values[n_values] = value;
        n_values++;
    }

    values[n_values] = NULL;

    return values;
}

// Count the rows of the set with value in the column col
int count_value(dataset *ds, int *set, int n_set, int col, char *value) {

    int n_value = 0;
    for (int i = 0; i < n_set; i++) {
        if (strcmp(value, ds->sheet[set[i]][col]) == 0) n_value++;
    }

    return n_value;
}

double entropy(dataset *ds, int *set, int n_set) {

    int class_col = ds->n_cols - 1;
    char **classes = distinct_values(ds, set, n_set, class_col);
    double entropy = 0;

    for (int i = 0; classes[i] != NULL; i++) {

        int n_class = count_value(ds, set, n_set, class_col, classes[i]);
        double p = (double) n_class / n_set;
        entropy = entropy - p * log2(p);
    }

    free(classes);

    return entropy;
}

// Select rows with value v from the column attribute
int * get_subset(dataset *ds, int *set, int n_set, int attribute, char *value) {

    int *subset = malloc(n_set * (sizeof(int)));
    int n_subset = 0;
    for (int i = 0; i < n_set; i++) {

        char *row = ds->sheet[set[i]][attribute];
        if (strcmp(value, row) == 0) {
            subset[n_subset] = set[i];
            n_subset++;
        }
    }

    return subset;
}

double information_gain(dataset *ds, int *set, int n_set, int attribute) {

    char **values = distinct_values(ds, set, n_set, attribute);
    double gain = entropy(ds, set, n_set);

    for (int i = 0; values[i] != NULL; i++) {

        int *subset = get_subset(ds, set, n_set, attribute, values[i]);
        int n_subset = count_value(ds, set, n_set, attribute, values[i]);

        gain = gain - ((double) n_subset/n_set) * entropy(ds, subset, n_subset);

        free(subset);
    }

    free(values);

    return gain;
}

char * majority_class(dataset *ds, int *set, int n_set) {

    int class_col = ds->n_cols - 1;
    char **classes = distinct_values(ds, set, n_set, class_col);
    char *majority = classes[0];
    int n_majority = 0;

    for (int i = 0; classes[i] != NULL; i++) {

        int n_class = count_value(ds, set, n_set, class_col, classes[i]);
        if (n_class > n_majority) {
            majority = classes[i];
            n_majority = n_class;
        }
    }

    free(classes);

    return majority;
}

node * create_new_node(int attribute, char *class, char **values) {

    node *new_node = calloc(1, sizeof(node));
    new_node->attribute = attribute;
    new_node->class = class;
    new_node->values = values;

    return new_node;
}

int best_attribute(dataset *ds, int *set, int n_set, int *used) {

    int best = -1;
    double best_gain = -1;
    for (int i = 0; i < ds->n_cols - 1; i++) {
        if (used[i] == 1) continue;

        double curr_gain = information_gain(ds, set, n_set, i);
        if (best_gain < curr_gain) {
            best = i;
            best_gain = curr_gain;
        }
    }
    
    return best;
}

node * build_tree(dataset *ds, int *set, int n_set, int *used) {

    double set_entropy = entropy(ds, set, n_set);

    // Case A: Pure set
    if (set_entropy == 0) return create_new_node(-1, majority_class(ds, set, n_set), NULL);

    // Find out the attribute with highest information gain
    int best = best_attribute(ds, set, n_set, used);

    // Case B: No attribute left
    if (best == -1) return create_new_node(-1, majority_class(ds, set, n_set), NULL);

    // Build tree
    char **values = distinct_values(ds, set, n_set, best);
    node *new_node = create_new_node(best, NULL, values);

    int n_children = 0;
    while (values[n_children] != NULL) n_children++;
    new_node->children = malloc(n_children * sizeof(node *));

    used[best] = 1;
    for (int i = 0; values[i] != NULL; i++) {
        int *subset = get_subset(ds, set, n_set, best, values[i]);
        int n_subset = count_value(ds, set, n_set, best, values[i]);
        new_node->children[i] = build_tree(ds, subset, n_subset, used);
        free(subset);
    }
    used[best] = 0;

    return new_node;
}

void print_tree(dataset *ds, node *curr_node, int depth) {
    if (curr_node == NULL) return;

    if (curr_node->class != NULL) {
        for (int t = 0; t < depth; t++) printf("\t");
        printf("%s\n", curr_node->class);
        return;
    }

    for (int t = 0; t < depth; t++) printf("\t"); 
    printf("%s\n", ds->sheet[0][curr_node->attribute]);

    depth++;

    for (int i = 0; curr_node->values[i] != NULL; i++) {
        for (int t = 0; t < depth; t++) printf("\t");
        printf("%s\n", curr_node->values[i]);
        print_tree(ds, curr_node->children[i], depth + 1);
    }
}

void free_tree(node *curr_node) {
    
    if (curr_node->attribute = -1) {
        free(curr_node);
        return;
    }

    for (int i = 0; curr_node->values[i] != NULL; i++) {
        free_tree(curr_node->children[i]);
    }
}

char ** create_row(dataset *ds, char *line) {

    char **row = malloc(ds->n_cols * sizeof(char *));

    char *token = strtok(line, ",");
    for (int i = 0; token != NULL && i < ds->n_cols - 1; i++) {
        row[i] = token;
        token = strtok(NULL, ",");
    }
    
    return row;
}

char * classify(char **row, node *curr_node) {

    while (curr_node->attribute != -1) {
        int invalid = 1;
        for (int i = 0; curr_node->children[i] != NULL; i++) {
            if (strcmp(row[curr_node->attribute], curr_node->values[i]) == 0) {
                curr_node = curr_node->children[i];
                invalid = 0;
                break;
            }
        }

        if (invalid) {
            return NULL;
        }
    }

    return curr_node->class;
}

double accuracy (dataset *ds, node *tree, int *set, int n_set) {

    int valid_rows = 0;

    for (int i = 0; i < n_set; i++) {
        if ()
    }
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
    read_csv(file, &ds);
    fclose(file);

    int *used = calloc(ds.n_cols - 1, sizeof(int));

    int n_root_set = ds.n_rows - 1;
    int *root_set = malloc(n_root_set * sizeof(int));
    for (int i = 0; i < n_root_set; i++) {
        root_set[i] = i + 1;
    }

    node *root = build_tree(&ds, root_set, n_root_set, used);

    printf("Tree was built.\n");

    print_tree(&ds, root, 0);

    printf("%llf", accuracy(&ds, root, root_set, n_root_set));

    int line[MAX_LINE];
    fgets(line, MAX_LINE, stdin);

    char **new_row = (&ds, line);

    classify(new_row, root);

    free_tree(root);
    free_dataset(&ds);

    return 0;
}
