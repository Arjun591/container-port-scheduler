#include "algorithms.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/* 0/1 dynamic programming over crane-hours; O(n * budget) time and space.
   This is strategic vessel selection, NOT time-window resource feasibility. */
int knapsack_crane_hours(const Port *p, int budget, unsigned char *selected, long long *best_containers) {
    if (!p || !selected || !best_containers || budget < 0 || budget > 5000) return 0;
    const size_t width = (size_t)budget + 1;
    const size_t rows = p->count + 1;
    if (rows > SIZE_MAX / width || rows * width > SIZE_MAX / sizeof(long long)) return 0;
    long long *dp = calloc(rows * width, sizeof(long long));
    if (!dp) return 0;
    memset(selected, 0, p->count);
    for (size_t i = 1; i < rows; i++) {
        const Vessel *v = &p->vessels[i - 1];
        long long weight = (long long)v->duration * v->cranes;
        for (int b = 0; b <= budget; b++) {
            long long best = dp[(i - 1) * width + (size_t)b];
            if (weight <= b) {
                long long take = dp[(i - 1) * width + (size_t)(b - weight)] + v->containers;
                if (take > best) best = take;
            }
            dp[i * width + (size_t)b] = best;
        }
    }
    *best_containers = dp[p->count * width + (size_t)budget];
    int left = budget;
    for (size_t i = p->count; i > 0; i--) {
        const Vessel *v = &p->vessels[i - 1];
        long long weight = (long long)v->duration * v->cranes;
        if (weight <= left && dp[i * width + (size_t)left] >
            dp[(i - 1) * width + (size_t)left]) {
            selected[i - 1] = 1;
            left -= (int)weight;
        }
    }
    free(dp);
    return 1;
}
