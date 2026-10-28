#include <stdio.h>
#include <stdlib.h> // Malloc e calloc

typedef struct clause {
    int n_literals;
    int *literals;
} clause;

typedef enum {
    FALSE      = -1,
    UNDEFINED =  0,
    TRUE       =  1
} value;

typedef struct formula {
    clause *clauses;
    int n_clauses;
    int n_var;
    value *I; // Partial interpretation
} formula;

value evaluate_literal(int li, value *I) {

    int var = abs(li);

    if (li < 0) {
        if (I[var] == TRUE) return FALSE;
        if (I[var] == FALSE) return TRUE;
    }

    return I[var];
}

value evaluate_clause(clause ci, value *I) {
    int has_undefined = 0;

    for (int i = 0; i < ci.n_literals; i++) {
        value partial_value = evaluate_literal(ci.literals[i], I);

        if (partial_value == TRUE) return TRUE;
        if (partial_value == UNDEFINED) has_undefined = 1;
    }

    if (has_undefined) return UNDEFINED;
    return FALSE;
}

value evaluate_formula(formula *F) {
    int has_undefined = 0;

    for (int i = 0; i < F->n_clauses; i++) {
        value partial_value = evaluate_clause(F->clauses[i], F->I);

        if (partial_value == FALSE) return FALSE;
        if (partial_value == UNDEFINED) has_undefined = 1;
    }

    if (has_undefined) return UNDEFINED;
    return TRUE;
}

int pick_unassigned_var(formula *F) {

    for (int i = 1; i <= F->n_var; i++)
    {
        if (F->I[i] == UNDEFINED)
        {
            return i;
        }
    }
    
    return 0;
}

value sat(formula *F) {

    value partial_value = evaluate_formula(F);
    if (partial_value == TRUE) return TRUE;
    if (partial_value == FALSE) return FALSE;

    int xi = pick_unassigned_var(F);

    F->I[xi] = TRUE;
    if (sat(F) == TRUE) return TRUE;
    F->I[xi] = FALSE;
    if (sat(F) == TRUE) return TRUE;
    F->I[xi] = UNDEFINED;

    return FALSE;
}

void read_file(FILE *file, formula *F) {
    int c;
    int curr_clause = 0;

    while ((c = fgetc(file)) != EOF) {

        if (c == 'c') {
            while (c != '\n' && c != EOF) c = fgetc(file);
        }
        else if (c == 'p') {
            fscanf(file, " cnf %d %d", &F->n_var, &F->n_clauses);
            F->clauses = calloc(F->n_clauses, sizeof(clause));
            F->I = calloc(F->n_var + 1, sizeof(value));
        }
        else if (c == '\n'|| c == '\r' || c == ' ') continue;
        else {
            int buffer;
            ungetc(c, file);

            F->clauses[curr_clause].literals = calloc(2 * F->n_var, sizeof(int)); // All clauses will have the same size

            while (fscanf(file, "%d", &buffer) == 1 && buffer != 0) {
                F->clauses[curr_clause].literals[F->clauses[curr_clause].n_literals] = buffer;
                F->clauses[curr_clause].n_literals++;
            }
            curr_clause++;
        }
    }

    printf("File was read successfully.\n");
    printf("The formula has %d variables and %d clauses.\n", F->n_var, F->n_clauses);
}

void free_formula(formula *F) {

    for (int i = 0; i < F->n_clauses; i++) {
        free(F->clauses[i].literals);
    }

    free(F->clauses);
    free(F->I);
    free(F);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: sat <file.cnf>\n");
        return 1;
    }

    FILE *DIMACS_file = fopen(argv[1], "r");
    if (DIMACS_file == NULL) {
        printf("Could not open %s\n", argv[1]);
        return 1;
    }

    formula *F = calloc(1, sizeof(formula));
    read_file(DIMACS_file, F);

    fclose(DIMACS_file);

    value S = sat(F);

    if (S == TRUE) {
        printf("SAT\n");
        printf("Solution:\n");
        char *value_name;
        for (int i = 1; i <= F->n_var; i++)
        {
            if(F->I[i] == 1) value_name = "TRUE";
            else if(F->I[i] == -1) value_name = "FALSE";
            else value_name = "UNDEFINED";

            printf("x%d: %s\n", i, value_name);
        }
    }
    else if (S == FALSE) printf("UNSAT\n");

    free_formula(F);

    return 0;
}