#include "algorithms.h"
#include <limits.h>
/* Edmonds-Karp: BFS chooses a shortest augmenting path in residual graph. */
int edmonds_karp(int n, const int *capacity, int source, int sink) {
    if (n < 2 || n > 16 || !capacity || source < 0 || sink < 0 ||
        source >= n || sink >= n || source == sink) return -1;
    int residual[16][16] = {{0}};
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) {
        if (capacity[i * 16 + j] < 0) return -1;
        residual[i][j] = capacity[i * 16 + j];
    }
    int total = 0;
    for (;;) {
        int parent[16], queue[16], head = 0, tail = 0;
        unsigned char seen[16] = {0};
        for (int i = 0; i < n; i++) parent[i] = -1;
        queue[tail++] = source;
        seen[source] = 1;
        while (head < tail && !seen[sink]) {
            int u = queue[head++];
            for (int v = 0; v < n; v++) if (!seen[v] && residual[u][v] > 0) {
                seen[v] = 1;
                parent[v] = u;
                queue[tail++] = v;
            }
        }
        if (!seen[sink]) break;
        int augment = INT_MAX;
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            if (residual[u][v] < augment) augment = residual[u][v];
        }
        if (augment > INT_MAX - total) return -1;
        total += augment;
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            residual[u][v] -= augment;
            residual[v][u] += augment;
        }
    }
    return total;
}
