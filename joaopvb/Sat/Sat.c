#include <stdio.h>
#include <stdlib.h>

typedef struct Clause {
    int size;
    int *lits;
} Clause;

typedef enum {
    VAL_FALSE = -1,
    VAL_UNDEF =  0,
    VAL_TRUE  =  1
} Truth;

typedef struct Formula {
    int num_vars;
    int num_clauses;
    Clause *clauses;
    Truth *assignment;
} Formula;

Truth literal_truth(int lit, Truth *assignment) {
    Truth t = assignment[abs(lit)];

    if (lit < 0) {
        if (t == VAL_TRUE) return VAL_FALSE;
        if (t == VAL_FALSE) return VAL_TRUE;
    }

    return t;
}

Truth clause_truth(Clause c, Truth *assignment) {
    int seen_undef = 0;

    for (int k = 0; k < c.size; k++) {
        Truth t = literal_truth(c.lits[k], assignment);

        if (t == VAL_TRUE) return VAL_TRUE;
        if (t == VAL_UNDEF) seen_undef = 1;
    }

    return seen_undef ? VAL_UNDEF : VAL_FALSE;
}

Truth formula_truth(Formula *f) {
    int seen_undef = 0;

    for (int k = 0; k < f->num_clauses; k++) {
        Truth t = clause_truth(f->clauses[k], f->assignment);

        if (t == VAL_FALSE) return VAL_FALSE;
        if (t == VAL_UNDEF) seen_undef = 1;
    }

    return seen_undef ? VAL_UNDEF : VAL_TRUE;
}

int first_free_var(Formula *f) {
    for (int v = 1; v <= f->num_vars; v++) {
        if (f->assignment[v] == VAL_UNDEF) return v;
    }

    return 0;
}

Truth solve(Formula *f) {
    Truth state = formula_truth(f);
    if (state != VAL_UNDEF) return state;

    int var = first_free_var(f);

    f->assignment[var] = VAL_TRUE;
    if (solve(f) == VAL_TRUE) return VAL_TRUE;

    f->assignment[var] = VAL_FALSE;
    if (solve(f) == VAL_TRUE) return VAL_TRUE;

    f->assignment[var] = VAL_UNDEF;
    return VAL_FALSE;
}

void load_dimacs(FILE *fp, Formula *f) {
    int ch;
    int next_clause = 0;

    while ((ch = fgetc(fp)) != EOF) {

        if (ch == ' ' || ch == '\n' || ch == '\r') {
            continue;
        }
        else if (ch == 'c') {
            while (ch != '\n' && ch != EOF) ch = fgetc(fp);
        }
        else if (ch == 'p') {
            fscanf(fp, " cnf %d %d", &f->num_vars, &f->num_clauses);
            f->clauses = calloc(f->num_clauses, sizeof(Clause));
            f->assignment = calloc(f->num_vars + 1, sizeof(Truth));
        }
        else {
            int lit;
            Clause *cur = &f->clauses[next_clause];

            ungetc(ch, fp);
            cur->lits = calloc(2 * f->num_vars, sizeof(int));

            while (fscanf(fp, "%d", &lit) == 1 && lit != 0) {
                cur->lits[cur->size++] = lit;
            }
            next_clause++;
        }
    }

    printf("File was read successfully.\n");
    printf("The formula has %d variables and %d clauses.\n", f->num_vars, f->num_clauses);
}

void destroy_formula(Formula *f) {
    for (int k = 0; k < f->num_clauses; k++) {
        free(f->clauses[k].lits);
    }

    free(f->clauses);
    free(f->assignment);
    free(f);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: sat <file.cnf>\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Could not open %s\n", argv[1]);
        return 1;
    }

    Formula *f = calloc(1, sizeof(Formula));
    load_dimacs(fp, f);
    fclose(fp);

    Truth result = solve(f);

    if (result == VAL_TRUE) {
        printf("SAT\n");
        printf("Solution:\n");
        for (int v = 1; v <= f->num_vars; v++) {
            const char *name;
            switch (f->assignment[v]) {
                case VAL_TRUE:  name = "TRUE";  break;
                case VAL_FALSE: name = "FALSE"; break;
                default:        name = "UNDEFINED"; break;
            }
            printf("x%d: %s\n", v, name);
        }
    }
    else if (result == VAL_FALSE) {
        printf("UNSAT\n");
    }

    destroy_formula(f);

    return 0;
}
