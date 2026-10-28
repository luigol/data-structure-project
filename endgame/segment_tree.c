#include <stdio.h>

#define N 8 // Number of elements in the array

int arr[N] = {5, 8, 6, 3, 2, 7, 2, 6};

// Segment tree stored as an array, like a heap:
// root = tree[1], children of tree[i] = tree[2 * i] and tree[2 * i + 1]
// tree[0] is not used
int sum_tree[4 * N];
int max_tree[4 * N];

// Each node covers the range [start, end] of arr and stores its sum.
void sum_build(int node, int start, int end) {

    // Pre-order: first the node, then its children
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += arr[i];
    }
    sum_tree[node] = sum;

    if (start == end) return; // Leaf: a single element

    int mid = (start + end) / 2;
    sum_build(2 * node, start, mid);         // Left half
    sum_build(2 * node + 1, mid + 1, end);   // Right half
}

// Sum of arr[left..right]. [start, end] is the range of the current node.
int sum_query(int node, int start, int end, int left, int right) {

    // Case 1: the node is completely outside the query
    if (right < start || end < left) return 0;

    // Case 2: the node is completely inside the query
    if (left <= start && end <= right) return sum_tree[node];

    // Case 3: partial overlap -> ask both children
    int mid = (start + end) / 2;
    int left_sum = sum_query(2 * node, start, mid, left, right);
    int right_sum = sum_query(2 * node + 1, mid + 1, end, left, right);

    return left_sum + right_sum;
}

// arr[index] = value, fixing every sum on the path from the root to the leaf.
void sum_update(int node, int start, int end, int index, int value) {

    // Pre-order: fix the node before going down.
    // arr[index] still has the old value, so the sum changes by (value - arr[index])
    sum_tree[node] += value - arr[index];

    if (start == end) { // Leaf: start == end == index
        arr[index] = value;
        return;
    }

    // Go down only on the side that contains the index
    int mid = (start + end) / 2;
    if (index <= mid) sum_update(2 * node, start, mid, index, value);
    else sum_update(2 * node + 1, mid + 1, end, index, value);
}



void print_array() {
    for (int i = 0; i < N; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

// Prints the tree with indentation: one level deeper for each child.
void print_tree(int node, int start, int end, int depth) {

    for (int i = 0; i < depth; i++) printf("    ");
    printf("[%d..%d] = %d   (tree[%d])\n", start, end, sum_tree[node], node);

    if (start == end) return; // Leaf

    int mid = (start + end) / 2;
    print_tree(2 * node, start, mid, depth + 1);
    print_tree(2 * node + 1, mid + 1, end, depth + 1);
}

int main() {

    // 1) Build the tree from the array
    sum_build(1, 0, N - 1);

    printf("Array: ");
    print_array();

    printf("\nSegment tree:\n");
    print_tree(1, 0, N - 1, 0);

    // 2) Range sum queries
    printf("\nQueries:\n");
    printf("  sum[2..5] = %d\n", sum_query(1, 0, N - 1, 2, 5));
    printf("  sum[0..7] = %d\n", sum_query(1, 0, N - 1, 0, 7));
    printf("  sum[1..3] = %d\n", sum_query(1, 0, N - 1, 1, 3));
    printf("  sum[4..4] = %d\n", sum_query(1, 0, N - 1, 4, 4));

    // 3) Change one position and query again
    printf("\nUpdate: arr[3] = 10\n");
    sum_update(1, 0, N - 1, 3, 10);

    printf("\nArray: ");
    print_array();

    printf("\nSegment tree:\n");
    print_tree(1, 0, N - 1, 0);

    printf("\nQueries:\n");
    printf("  sum[2..5] = %d\n", sum_query(1, 0, N - 1, 2, 5));
    printf("  sum[0..7] = %d\n", sum_query(1, 0, N - 1, 0, 7));

    return 0;
}
