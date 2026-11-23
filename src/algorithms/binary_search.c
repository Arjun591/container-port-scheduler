#include "algorithms.h"
#include <stdlib.h>
static int compare_id(const void *left, const void *right) {
    const VesselIndexEntry *a = left, *b = right;
    return (a->id > b->id) - (a->id < b->id);
}
void port_build_index(const Port *p, VesselIndex *idx) {
    idx->count = p->count;
    for (size_t i = 0; i < p->count; i++)
        idx->entries[i] = (VesselIndexEntry){p->vessels[i].id, i};
    qsort(idx->entries, idx->count, sizeof(idx->entries[0]), compare_id);
}
int port_lookup_id(const Port *p, const VesselIndex *idx, int id) {
    (void)p;
    int low = 0, high = (int)idx->count - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int current = idx->entries[mid].id;
        if (current == id) return (int)idx->entries[mid].array_index;
        if (current < id) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}
