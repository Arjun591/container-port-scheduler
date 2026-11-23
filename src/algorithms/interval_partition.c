#include "algorithms.h"
/* Interval partitioning for FIXED immediate-start windows only.
   Lower bound on berths if every vessel MUST start at its arrival time.
   A berth is reusable when a prior vessel finishes exactly as another starts. */
int interval_partition_port(const Port *p, int *berth_for_ship) {
    if (!p || !berth_for_ship) return -1;
    int order[PORT_MAX_VESSELS], ends[PORT_MAX_VESSELS] = {0};
    if (!merge_sort_vessel_order(p, order, PORT_ARRIVAL_FIRST)) return -1;
    int used = 0;
    for (size_t i = 0; i < p->count; i++) {
        int v = order[i];
        int b = 0;
        while (b < used && ends[b] > p->vessels[v].arrival) b++;
        if (b == used) used++;
        ends[b] = p->vessels[v].arrival + p->vessels[v].duration;
        berth_for_ship[v] = b;
    }
    return used;
}
