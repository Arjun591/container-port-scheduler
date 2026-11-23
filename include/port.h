#ifndef PORT_H
#define PORT_H
#include <stddef.h>
#define PORT_MAX_VESSELS 256
#define PORT_NAME_LEN 64
#define PORT_ERR_LEN 256

typedef struct {
    int id;
    char name[PORT_NAME_LEN];
    int arrival;      /* earliest allowed start, integer hours */
    int duration;     /* berth occupancy, integer hours */
    int cranes;       /* simultaneous quay cranes requested */
    int priority;     /* 1..10, higher value gets preference */
    int containers;   /* report-only workload */
    int deadline;     /* 0 = no deadline, else latest completion */
} Vessel;

typedef struct {
    int scheduled;    /* 1 if feasible assignment exists */
    int berth;        /* 0-based berth, -1 for unassigned */
    int start;
    int finish;
} Assignment;

typedef struct {
    Vessel vessels[PORT_MAX_VESSELS];
    Assignment assignments[PORT_MAX_VESSELS];
    size_t count;
    int berth_count;
    int crane_count;
} Port;

typedef enum { PORT_ARRIVAL_FIRST=1, PORT_PRIORITY_FIRST=2 } PortStrategy;

typedef struct {
    size_t total;
    size_t scheduled;
    size_t unscheduled;
    long long containers_served;
    int makespan;
    double avg_wait;
    double avg_turnaround;
    double berth_utilization;
    double crane_utilization;
} PortStats;

typedef struct { int id; size_t array_index; } VesselIndexEntry;
typedef struct { VesselIndexEntry entries[PORT_MAX_VESSELS]; size_t count; } VesselIndex;

void port_init(Port *p);
int port_load_csv(const char *path, Port *p, char *err, size_t errsz);
int port_validate_input(const Port *p, char *err, size_t errsz);
int port_schedule(Port *p, PortStrategy strategy);
int port_verify_schedule(const Port *p, char *err, size_t errsz);
PortStats port_stats(const Port *p);
void port_print_report(const Port *p, const PortStats *stats, const char *strategy);
int port_export_csv(const Port *p, const char *path);
int port_export_json(const Port *p, const PortStats *s, const char *strategy, const char *path);
/* Editable fleet operations; all mutations invalidate existing schedules. */
int port_find_vessel(const Port *p, int id);
int port_add_vessel(Port *p, Vessel v, char *err, size_t errsz);
int port_edit_vessel(Port *p, int id, Vessel v, char *err, size_t errsz);
int port_delete_vessel(Port *p, int id);
int port_set_capacity(Port *p, int berths, int cranes, char *err, size_t errsz);
void port_list_vessels(const Port *p);
int port_save_csv(const Port *p, const char *path);

#endif
