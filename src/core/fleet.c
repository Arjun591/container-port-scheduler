#include "port.h"
#include "algorithms.h"
#include <stdio.h>
#include <string.h>
static void clear_assignments(Port *p) {
    for (size_t i = 0; i < p->count; i++)
        p->assignments[i] = (Assignment){0, -1, 0, 0};
}
int port_find_vessel(const Port *p, int id) {
    VesselIndex idx;
    port_build_index(p, &idx);
    return port_lookup_id(p, &idx, id);
}
int port_add_vessel(Port *p, Vessel v, char *err, size_t errsz) {
    if (p->count >= PORT_MAX_VESSELS) {
        snprintf(err, errsz, "Maximum %d ships reached", PORT_MAX_VESSELS); return 0;
    }
    Port tmp = *p;
    tmp.vessels[tmp.count++] = v;
    if (!port_validate_input(&tmp, err, errsz)) return 0;
    p->vessels[p->count++] = v;
    clear_assignments(p);
    return 1;
}
int port_edit_vessel(Port *p, int id, Vessel new_v, char *err, size_t errsz) {
    int index = port_find_vessel(p, id);
    if (index < 0) { snprintf(err, errsz, "Ship ID %d not found", id); return 0; }
    Port tmp = *p;
    tmp.vessels[index] = new_v;
    if (!port_validate_input(&tmp, err, errsz)) return 0;
    p->vessels[index] = new_v;
    clear_assignments(p);
    return 1;
}
int port_delete_vessel(Port *p, int id) {
    int index = port_find_vessel(p, id);
    if (index < 0) return 0;
    for (size_t i = (size_t)index + 1; i < p->count; i++)
        p->vessels[i - 1] = p->vessels[i];
    p->count--;
    clear_assignments(p);
    return 1;
}
int port_set_capacity(Port *p, int berths, int cranes, char *err, size_t errsz) {
    Port tmp = *p;
    tmp.berth_count = berths;
    tmp.crane_count = cranes;
    if (!port_validate_input(&tmp, err, errsz)) return 0;
    p->berth_count = berths;
    p->crane_count = cranes;
    clear_assignments(p);
    return 1;
}
void port_list_vessels(const Port *p) {
    printf("\nShips: %zu | Berths: %d | Cranes: %d\n",p->count,p->berth_count,p->crane_count);
    puts("ID     Name                Arr  Dur  Cr  Pri  Containers Deadline");
    for (size_t i = 0; i < p->count; i++) {
        const Vessel *v = &p->vessels[i];
        printf("%-6d %-19.19s %-4d %-4d %-3d %-4d %-10d %d\n",
           v->id,v->name,v->arrival,v->duration,v->cranes,v->priority,v->containers,v->deadline);
    }
}
int port_save_csv(const Port *p, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "#berths=%d\n#cranes=%d\n", p->berth_count, p->crane_count);
    fputs("id,name,arrival,duration,cranes,priority,containers,deadline\n", f);
    for (size_t i = 0; i < p->count; i++) {
        const Vessel *v = &p->vessels[i];
        fprintf(f, "%d,%s,%d,%d,%d,%d,%d,%d\n",v->id,v->name,v->arrival,v->duration,
                v->cranes,v->priority,v->containers,v->deadline);
    }
    return fclose(f) == 0;
}
