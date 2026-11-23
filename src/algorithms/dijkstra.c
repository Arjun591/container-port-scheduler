#include "algorithms.h"
/* Adjacency matrix, -1 means no edge; weights must be nonnegative. */
int dijkstra(int n, const int *weights, int source, int *dist) {
    if (!weights || !dist || n < 1 || n > 16 || source < 0 || source >= n) return 0;
    unsigned char used[16] = {0};
    for (int i = 0; i < n * n; i++) if (weights[i] < -1) return 0;
    for (int i = 0; i < n; i++) dist[i] = ALG_INF;
    dist[source] = 0;
    for (int k = 0; k < n; k++) {
        int u = -1;
        for (int i = 0; i < n; i++)
            if (!used[i] && (u < 0 || dist[i] < dist[u])) u = i;
        if (u < 0 || dist[u] == ALG_INF) break;
        used[u] = 1;
        for (int v = 0; v < n; v++)
            if (weights[u * n + v] >= 0 &&
                (long long)dist[u] + weights[u * n + v] < dist[v])
                dist[v] = dist[u] + weights[u * n + v];
    }
    return 1;
}
