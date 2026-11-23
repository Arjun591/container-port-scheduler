#include "algorithms.h"
#include <stdlib.h>
typedef struct { int u, v, cost; } Edge;
static int compare_edges(const void *a, const void *b) {
    const Edge *x = a, *y = b;
    return (x->cost > y->cost) - (x->cost < y->cost);
}
static int root(int *parent, int v) {
    if (parent[v] != v) parent[v] = root(parent, parent[v]);
    return parent[v];
}
/* Kruskal + union-find, O(E log E). */
int kruskal_mst(int n, int m, const int *u, const int *v, const int *weights, int *total) {
    if (n < 1 || n > 16 || m < 0 || !u || !v || !weights || !total) return 0;
    Edge *edges = malloc((size_t)(m ? m : 1) * sizeof(*edges));
    if (!edges) return 0;
    for (int i = 0; i < m; i++) {
        if (u[i] < 0 || v[i] < 0 || u[i] >= n || v[i] >= n || weights[i] < 0) {
            free(edges); return 0;
        }
        edges[i] = (Edge){u[i], v[i], weights[i]};
    }
    qsort(edges, (size_t)m, sizeof(*edges), compare_edges);
    int parent[16], rank[16] = {0};
    for (int i = 0; i < n; i++) parent[i] = i;
    *total = 0;
    int used = 0;
    for (int i = 0; i < m && used < n - 1; i++) {
        int a = root(parent, edges[i].u), b = root(parent, edges[i].v);
        if (a == b) continue;
        if (rank[a] < rank[b]) { int t = a; a = b; b = t; }
        parent[b] = a;
        if (rank[a] == rank[b]) rank[a]++;
        *total += edges[i].cost;
        used++;
    }
    free(edges);
    return used == n - 1;
}
