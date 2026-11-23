#ifndef NETWORK_H
#define NETWORK_H
#define NETWORK_MAX_NODES 16
#define NETWORK_NAME_LEN 32
typedef struct {
    int n;
    char names[NETWORK_MAX_NODES][NETWORK_NAME_LEN];
    int minutes[NETWORK_MAX_NODES * NETWORK_MAX_NODES]; /* -1 = absent */
    int capacity[16][16];
} PortNetwork;
int network_load_csv(const char *path, PortNetwork *net, char *err, int errsz);
int network_print_analysis(const PortNetwork *net, const char *source, const char *target);
#endif
