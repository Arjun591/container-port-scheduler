#include "port.h"
#include "algorithms.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/* Events describe changes in the number of occupied cranes. */
typedef struct {
    int time;
    int crane_change;
} CraneEvent;

static int compare_events(const void *left, const void *right) {
    const CraneEvent *a = left, *b = right;
    return (a->time > b->time) - (a->time < b->time);
}
static int compare_times(const void *left, const void *right) {
    int a = *(const int *)left, b = *(const int *)right;
    return (a > b) - (a < b);
}

/* Half-open intervals: ships may exchange a berth at the SAME hour. */
static int intervals_overlap(int a_start, int a_finish, int b_start, int b_finish) {
    return a_start < b_finish && b_start < a_finish;
}

/* One berth cannot serve two ships at overlapping times. */
static int berth_available(const Port *port, int berth, int start, int finish) {
    for (size_t i = 0; i < port->count; i++) {
        const Assignment *assigned = &port->assignments[i];
        if (assigned->scheduled && assigned->berth == berth &&
            intervals_overlap(start, finish, assigned->start, assigned->finish))
            return 0;
    }
    return 1;
}

/* Sweep the overlap events of ALL already accepted ships.
   At each boundary, the total active crane use plus the candidate's
   simultaneous request must not exceed the port-wide crane count. */
static int cranes_available(const Port *port, int requested, int start, int finish) {
    if (requested > port->crane_count) return 0;
    CraneEvent events[2 * PORT_MAX_VESSELS];
    int event_count = 0;
    for (size_t i = 0; i < port->count; i++) {
        const Assignment *a = &port->assignments[i];
        if (!a->scheduled || !intervals_overlap(start, finish, a->start, a->finish))
            continue;
        int overlap_start = a->start > start ? a->start : start;
        int overlap_finish = a->finish < finish ? a->finish : finish;
        events[event_count++] = (CraneEvent){overlap_start, port->vessels[i].cranes};
        events[event_count++] = (CraneEvent){overlap_finish, -port->vessels[i].cranes};
    }
    qsort(events, (size_t)event_count, sizeof(events[0]), compare_events);
    int active = 0;
    for (int i = 0; i < event_count; ) {
        int hour = events[i].time, changes_at_hour = 0;
        /* Group simultaneous completions/starts to respect [start, finish). */
        while (i < event_count && events[i].time == hour)
            changes_at_hour += events[i++].crane_change;
        active += changes_at_hour;
        if (active + requested > port->crane_count) return 0;
    }
    return 1;
}

/* Greedy earliest-feasible placement after ordering ships with Merge Sort.
   This deliberately does NOT claim a globally optimal solution. */
int port_schedule(Port *port, PortStrategy strategy) {
    if (!port || (strategy != PORT_ARRIVAL_FIRST && strategy != PORT_PRIORITY_FIRST))
        return 0;

    /* Every run starts from a fresh, empty schedule. */
    for (size_t i = 0; i < port->count; i++)
        port->assignments[i] = (Assignment){0, -1, 0, 0};

    int order[PORT_MAX_VESSELS];
    if (!merge_sort_vessel_order(port, order, strategy)) return 0;

    for (size_t rank = 0; rank < port->count; rank++) {
        int vessel_index = order[rank];
        const Vessel *ship = &port->vessels[vessel_index];

        /* Candidate starts: arrival time or an existing vessel completion.
           Between these boundary times no resource becomes newly available,
           so searching every individual hour would be unnecessary. */
        int candidates[PORT_MAX_VESSELS + 1];
        int count = 0;
        candidates[count++] = ship->arrival;
        for (size_t i = 0; i < port->count; i++) {
            const Assignment *a = &port->assignments[i];
            if (a->scheduled && a->finish >= ship->arrival)
                candidates[count++] = a->finish;
        }
        qsort(candidates, (size_t)count, sizeof(candidates[0]), compare_times);

        for (int i = 0; i < count; i++) {
            if (i > 0 && candidates[i] == candidates[i - 1]) continue;
            int start = candidates[i];
            long long finish64 = (long long)start + ship->duration;
            if (finish64 > INT_MAX ||
                (ship->deadline != 0 && finish64 > ship->deadline))
                continue;
            int finish = (int)finish64;
            if (!cranes_available(port, ship->cranes, start, finish)) continue;

            /* Choose the first berth without an overlapping assignment. */
            for (int berth = 0; berth < port->berth_count; berth++) {
                if (berth_available(port, berth, start, finish)) {
                    port->assignments[vessel_index] =
                        (Assignment){1, berth, start, finish};
                    break;
                }
            }
            if (port->assignments[vessel_index].scheduled) break;
        }
        /* If no time and berth work, the ship stays unscheduled. */
    }
    return 1;
}

/* Independent validator checks physical capacity and deadline constraints.
   It does NOT assume the greedy schedule is optimal. */
int port_verify_schedule(const Port *port, char *err, size_t errsz) {
    if (!port) return 0;
    CraneEvent events[2 * PORT_MAX_VESSELS];
    int event_count = 0;
    for (size_t i = 0; i < port->count; i++) {
        const Vessel *v = &port->vessels[i];
        const Assignment *a = &port->assignments[i];
        if (!a->scheduled) continue;
        if (a->berth < 0 || a->berth >= port->berth_count ||
            a->start < v->arrival ||
            (long long)a->finish - a->start != v->duration ||
            (v->deadline && a->finish > v->deadline)) {
            snprintf(err, errsz, "Invalid time or berth for ship ID %d", v->id);
            return 0;
        }
        for (size_t j = 0; j < i; j++) {
            const Assignment *other = &port->assignments[j];
            if (other->scheduled && other->berth == a->berth &&
                intervals_overlap(a->start,a->finish,other->start,other->finish)) {
                snprintf(err,errsz,"Berth overlap: IDs %d and %d",v->id,port->vessels[j].id);
                return 0;
            }
        }
        events[event_count++] = (CraneEvent){a->start, v->cranes};
        events[event_count++] = (CraneEvent){a->finish, -v->cranes};
    }
    qsort(events, (size_t)event_count, sizeof(events[0]), compare_events);
    int active = 0;
    for (int i = 0; i < event_count; ) {
        int hour = events[i].time, delta = 0;
        while (i < event_count && events[i].time == hour)
            delta += events[i++].crane_change;
        active += delta;
        if (active < 0 || active > port->crane_count) {
            snprintf(err,errsz,"Crane constraint violated around hour %d",hour);
            return 0;
        }
    }
    return 1;
}
