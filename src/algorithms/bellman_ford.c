#include "algorithms.h"
/* General relax-all-edges algorithm. -1 is reserved for absent edges. */
int bellman_ford(int n, const int *weights, int source, int *dist) {
    if (!weights || !dist || n < 1 || n > 16 || source < 0 || source >= n) return 0;
    for (int i = 0; i < n; i++) dist[i] = ALG_INF;
    dist[source] = 0;
    for (int pass = 1; pass < n; pass++) {
        int changed = 0;
        for (int u = 0; u < n; u++) if (dist[u] != ALG_INF)
            for (int v = 0; v < n; v++) if (weights[u * n + v] != -1 &&
                (long long)dist[u] + weights[u * n + v] < dist[v]) {
                dist[v] = dist[u] + weights[u * n + v];
                changed = 1;
            }
        if (!changed) break;
    }
    for (int u = 0; u < n; u++) if (dist[u] != ALG_INF)
        for (int v = 0; v < n; v++) if (weights[u * n + v] != -1 &&
            (long long)dist[u] + weights[u * n + v] < dist[v]) return -1;
    return 1;
}
