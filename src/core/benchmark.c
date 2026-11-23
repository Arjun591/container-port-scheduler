#include "port.h"
#include <stdio.h>
#include <time.h>
static const char *scenarios[] = {
    "data/normal.csv", "data/capacity_shock.csv", "data/stress.csv"
};
int port_benchmark(void) {
    puts("\n=== VERIFIED SCHEDULER BENCHMARK ===");
    puts("Dataset             Strategy    Served   AvgWait   Makespan  CPU time/run");
    for (int sc = 0; sc < 3; sc++) {
        Port base;
        char err[PORT_ERR_LEN];
        if (!port_load_csv(scenarios[sc], &base, err, sizeof(err))) {
            fprintf(stderr,"Benchmark input: %s\n",err); return 0;
        }
        for (int alg = 1; alg <= 2; alg++) {
            Port p = base;
            const int repeats = (base.count > 100) ? 15 : 80;
            clock_t before = clock();
            for (int rep = 0; rep < repeats; rep++)
                if (!port_schedule(&p, (PortStrategy)alg)) return 0;
            clock_t after = clock();
            if (!port_verify_schedule(&p, err, sizeof(err))) {
                fprintf(stderr,"Invalid benchmark schedule: %s\n",err); return 0;
            }
            PortStats s = port_stats(&p);
            double millis = 1000.0 * (after - before) / (CLOCKS_PER_SEC * repeats);
            printf("%-19.19s %-11s %3zu/%-3zu %8.2f %10d %12.3f ms\n",
                sc == 0 ? "Normal" : sc == 1 ? "Capacity shock" : "Stress (180)",
                alg == 1 ? "Arrival" : "Priority", s.scheduled,s.total,s.avg_wait,
                s.makespan,millis);
        }
    }
    puts("CPU times depend on the device; these are NOT end-to-end wall-clock timings.");
    return 1;
}
