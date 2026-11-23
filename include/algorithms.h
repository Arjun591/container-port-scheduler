#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include "port.h"
#define ALG_INF 1000000000

/* 3: Merge sort is used to order vessels for both real scheduling strategies. */
int merge_sort_vessel_order(const Port *p, int *order, PortStrategy strategy);
/* 4: Binary search, over a sorted vessel-ID index. */
void port_build_index(const Port *p, VesselIndex *idx);
int port_lookup_id(const Port *p, const VesselIndex *idx, int id);
/* 5: Minimum number of berths for fixed [arrival, arrival+duration) windows. */
int interval_partition_port(const Port *p, int *berth_for_ship);
/* 6: Exact 0/1 knapsack on total crane-hour budget, not a berth timetable. */
int knapsack_crane_hours(const Port *p, int budget, unsigned char *selected, long long *best_containers);
/* 7,8: Directed terminal shortest paths, adjacency matrix representation. */
int dijkstra(int n, const int *weights, int source, int *dist);
int bellman_ford(int n, const int *weights, int source, int *dist);
/* 9: Minimum spanning tree for UNDIRECTED infrastructure. */
int kruskal_mst(int n, int m, const int *u, const int *v, const int *weights, int *total);
/* 10: Edmonds-Karp maximum flow on a directed capacitated network. */
int edmonds_karp(int n, const int *capacity, int source, int sink);
#endif
