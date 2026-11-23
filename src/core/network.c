#include "network.h"
#include "algorithms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int find_or_add(PortNetwork *g, const char *name) {
    for (int i = 0; i < g->n; i++) if (strcmp(g->names[i], name) == 0) return i;
    if (g->n == NETWORK_MAX_NODES) return -1;
    int id = g->n++;
    snprintf(g->names[id], NETWORK_NAME_LEN, "%s", name);
    return id;
}
int network_load_csv(const char *path, PortNetwork *g, char *err, int errsz) {
    FILE *f = fopen(path, "r");
    if (!f) { snprintf(err, (size_t)errsz, "Cannot open network: %s", path); return 0; }
    memset(g, 0, sizeof(*g));
    for (int i = 0; i < NETWORK_MAX_NODES * NETWORK_MAX_NODES; i++) g->minutes[i] = -1;
    for (int i = 0; i < NETWORK_MAX_NODES; i++) g->minutes[i * 16 + i] = 0;
    char line[256];
    int line_no = 0, header = 0;
    while (fgets(line, sizeof(line), f)) {
        line_no++;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (!header) { header = 1; continue; }
        char a[NETWORK_NAME_LEN], b[NETWORK_NAME_LEN], extra;
        int minutes, cap;
        if (sscanf(line, " %31[^,],%31[^,],%d,%d %c", a, b, &minutes, &cap, &extra) != 4 ||
            minutes < 0 || minutes >= ALG_INF / 16 || cap < 0) {
            snprintf(err, (size_t)errsz, "Invalid network CSV line %d", line_no);
            fclose(f); return 0;
        }
        int u = find_or_add(g, a), v = find_or_add(g, b);
        if (u < 0 || v < 0) {
            snprintf(err, (size_t)errsz, "Network exceeds 16 nodes"); fclose(f); return 0;
        }
        int index = u * 16 + v;
        if (g->minutes[index] < 0 || minutes < g->minutes[index]) g->minutes[index] = minutes;
        if (cap > g->capacity[u][v]) g->capacity[u][v] = cap;
    }
    fclose(f);
    if (g->n < 2) { snprintf(err,(size_t)errsz,"Network requires 2+ nodes"); return 0; }
    return 1;
}
int network_print_analysis(const PortNetwork *g, const char *source, const char *target) {
    int s = -1, t = -1;
    for (int i = 0; i < g->n; i++) {
        if (strcmp(g->names[i], source) == 0) s = i;
        if (strcmp(g->names[i], target) == 0) t = i;
    }
    if (s < 0 || t < 0 || s == t) {
        fprintf(stderr, "Choose two different existing node names.\n"); return 0;
    }
    int graph[256], dijkstra_dist[16], bellman_dist[16];
    for (int i = 0; i < g->n; i++) for (int j = 0; j < g->n; j++)
        graph[i * g->n + j] = g->minutes[i * 16 + j];
    if (!dijkstra(g->n, graph, s, dijkstra_dist) ||
        bellman_ford(g->n, graph, s, bellman_dist) != 1) return 0;
    for (int i = 0; i < g->n; i++) if (dijkstra_dist[i] != bellman_dist[i]) {
        fprintf(stderr, "Shortest path methods disagree!\n"); return 0;
    }
    printf("\nTerminal network: %d nodes\n",g->n);
    printf("Shortest %s -> %s: ", source, target);
    if (dijkstra_dist[t] == ALG_INF) puts("unreachable");
    else printf("%d minutes (Dijkstra = Bellman-Ford)\n",dijkstra_dist[t]);
    /* Build undirected view for minimum spanning tree. */
    int from[120], to[120], cost[120], m = 0;
    for (int i = 0; i < g->n; i++) for (int j = i + 1; j < g->n; j++) {
        int a = g->minutes[i * 16 + j], b = g->minutes[j * 16 + i];
        if (a < 0 && b < 0) continue;
        from[m] = i; to[m] = j;
        cost[m++] = (a < 0) ? b : (b < 0 || a < b ? a : b);
    }
    int tree_cost = 0;
    if (kruskal_mst(g->n, m, from, to, cost, &tree_cost))
        printf("Kruskal MST: %d (sum of undirected minimum-link minute weights)\n",tree_cost);
    else puts("Kruskal MST: disconnected network");
    printf("Edmonds-Karp max flow %s -> %s: %d capacity units/interval\n",
           source, target, edmonds_karp(g->n, &g->capacity[0][0], s, t));
    puts("Note: MST is an undirected infrastructure proxy; network routes/flow are directed.");
    return 1;
}
