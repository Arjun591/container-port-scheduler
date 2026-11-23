#include "algorithms.h"
#include <stdlib.h>
/* Return true when index a must appear before index b. */
static int goes_first(const Port *p, int a, int b, PortStrategy strategy) {
    const Vessel *x = &p->vessels[a], *y = &p->vessels[b];
    if (strategy == PORT_PRIORITY_FIRST && x->priority != y->priority)
        return x->priority > y->priority;
    if (x->arrival != y->arrival) return x->arrival < y->arrival;
    if (x->duration != y->duration) return x->duration < y->duration;
    return x->id <= y->id;
}
static void sort_range(const Port *p, int *a, int *tmp, int lo, int hi, PortStrategy s) {
    if (hi - lo <= 1) return;
    int mid = lo + (hi - lo) / 2;
    sort_range(p, a, tmp, lo, mid, s);
    sort_range(p, a, tmp, mid, hi, s);
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi)
        tmp[k++] = goes_first(p, a[i], a[j], s) ? a[i++] : a[j++];
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi) tmp[k++] = a[j++];
    for (i = lo; i < hi; i++) a[i] = tmp[i];
}
int merge_sort_vessel_order(const Port *p, int *order, PortStrategy strategy) {
    if (!p || !order || (strategy != PORT_ARRIVAL_FIRST && strategy != PORT_PRIORITY_FIRST))
        return 0;
    int *tmp = malloc((p->count ? p->count : 1) * sizeof(int));
    if (!tmp) return 0;
    for (size_t i = 0; i < p->count; i++) order[i] = (int)i;
    sort_range(p, order, tmp, 0, (int)p->count, strategy);
    free(tmp);
    return 1;
}
