# Scheduling Example: Arrival-First vs Priority-First

## Overview

This example demonstrates how the Container Port Berth & Crane Scheduling Suite allocates vessels under berth and crane capacity constraints.

It compares two scheduling strategies:

- **Arrival-First Greedy Scheduling:** Processes vessels in order of arrival.
- **Priority-First Greedy Scheduling:** Processes vessels according to their assigned priority.

Both strategies attempt to find feasible berth assignments while respecting vessel arrival times, service durations, deadlines, and shared crane availability.

## 1. Initial Configuration

The port has the following resources:

| Resource | Available Capacity |
|---|---:|
| Berths | 2 |
| Cranes | 3 |
| Vessels | 3 |

### Vessel Data

| Vessel | Arrival | Duration | Cranes Required | Priority | Deadline |
|---|---:|---:|---:|---:|---:|
| A (ID 1) | 0 | 4 | 2 | 2 | 10 |
| B (ID 2) | 1 | 3 | 2 | 5 | 8 |
| C (ID 3) | 2 | 2 | 1 | 3 | 10 |

All times are expressed in integer hours.

Each vessel occupies one berth and retains its requested number of cranes throughout its service interval.

## 2. Arrival-First Greedy Scheduling

**Processing order:** A → B → C

### Step 1 — Schedule Vessel A

Vessel A arrives at hour 0 and requires:

- 4 hours of service
- 2 cranes
- 1 berth

The port has sufficient capacity.

**Assignment:** Berth 1, `[0, 4)`

### Step 2 — Schedule Vessel B

Vessel B arrives at hour 1 and requires 2 cranes.

Starting at hour 1 would exceed the available crane capacity:

`2 (A) + 2 (B) = 4 cranes > 3 available`

Vessel A finishes at hour 4, releasing its resources.

**Assignment:** Berth 1, `[4, 7)`

### Step 3 — Schedule Vessel C

Vessel C arrives at hour 2 and requires 1 crane for 2 hours.

Although Vessel B has already been assigned a later service interval, Vessel C can use an earlier available slot.

During `[2, 4)`:

`2 (A) + 1 (C) = 3 cranes`

This satisfies the crane capacity constraint.

**Assignment:** Berth 2, `[2, 4)`

### Final Arrival-First Schedule

| Vessel | Berth | Start | Finish | Waiting Time |
|---|---:|---:|---:|---:|
| A | 1 | 0 | 4 | 0 |
| B | 1 | 4 | 7 | 3 |
| C | 2 | 2 | 4 | 0 |

### Timeline

```text
Time       0   1   2   3   4   5   6   7
           |---|---|---|---|---|---|---|

Berth 1    [       A       ][     B     ]
Berth 2            [   C   ]

A:         [0,4)  — 2 cranes
B:                         [4,7) — 2 cranes
C:                 [2,4)  — 1 crane
```

### Performance

**Vessels served:** 3 of 3

Waiting times:

- A: `0 - 0 = 0`
- B: `4 - 1 = 3`
- C: `2 - 2 = 0`

**Average waiting time:**

`(0 + 3 + 0) / 3 = 1.00 hour`

## 3. Priority-First Greedy Scheduling

**Processing order:** B → C → A

Higher priority values are processed first.

### Step 1 — Schedule Vessel B

Vessel B has the highest priority and arrives at hour 1.

**Assignment:** Berth 1, `[1, 4)`

It uses 2 cranes.

### Step 2 — Schedule Vessel C

Vessel C arrives at hour 2 and requires 1 crane.

During `[2, 4)`:

`2 (B) + 1 (C) = 3 cranes`

Both vessels can operate simultaneously.

**Assignment:** Berth 2, `[2, 4)`

### Step 3 — Schedule Vessel A

Vessel A arrives at hour 0 and requires 2 cranes for 4 hours.

Starting at hour 0 would overlap the already assigned vessels and exceed crane capacity.

The next feasible candidate is hour 4, when B and C have completed service.

**Assignment:** Berth 1, `[4, 8)`

### Final Priority-First Schedule

| Vessel | Berth | Start | Finish | Waiting Time |
|---|---:|---:|---:|---:|
| B | 1 | 1 | 4 | 0 |
| C | 2 | 2 | 4 | 0 |
| A | 1 | 4 | 8 | 4 |

### Timeline

```text
Time       0   1   2   3   4   5   6   7   8
           |---|---|---|---|---|---|---|---|

Berth 1        [     B     ][       A       ]
Berth 2            [   C   ]

B:             [1,4) — 2 cranes
C:                 [2,4) — 1 crane
A:                         [4,8) — 2 cranes
```

### Performance

**Vessels served:** 3 of 3

Waiting times:

- B: `1 - 1 = 0`
- C: `2 - 2 = 0`
- A: `4 - 0 = 4`

**Average waiting time:**

`(0 + 0 + 4) / 3 = 1.33 hours`

## 4. Scheduling Comparison

| Metric | Arrival-First | Priority-First |
|---|---:|---:|
| Vessels served | 3 | 3 |
| Unscheduled vessels | 0 | 0 |
| Average waiting time | 1.00 hour | 1.33 hours |
| Final vessel completion | Hour 7 | Hour 8 |
| Berth capacity respected | Yes | Yes |
| Crane capacity respected | Yes | Yes |
| Deadlines respected | Yes | Yes |

In this example, arrival-first scheduling produces a lower average waiting time and an earlier final completion time.

However, this result does not establish that arrival-first scheduling is universally better. Performance depends on vessel characteristics, resource availability, and scheduling order.

## 5. Half-Open Service Intervals

The application represents service intervals using the notation:

`[start, finish)`

The starting time is included, while the finishing time is excluded.

For example:

- Vessel A: `[0, 4)`
- Vessel B: `[4, 7)`

These intervals do not overlap.

Therefore, a berth or crane released at hour 4 can immediately be reassigned to another vessel starting at hour 4.

The scheduling engine applies consistent boundary handling when checking berth occupancy and simultaneous crane demand.

## 6. Key Observations

### Scheduling Order Affects Results

Different vessel processing orders can produce different berth assignments, service start times, and waiting times.

### Shared Resources Create Dependencies

Even when a berth is available, a vessel may be delayed because insufficient cranes are available.

### Earlier Gaps Can Be Utilized

The scheduler can assign a vessel to an earlier feasible time slot even when another vessel has already been scheduled for a later interval.

### Feasibility Does Not Guarantee Optimality

Both greedy strategies produce valid schedules in this example.

Neither strategy guarantees a globally optimal timetable across all possible vessel combinations.

## 7. Conclusion

This example illustrates how vessel arrival times, priorities, service durations, and shared crane requirements interact during berth scheduling.

Both strategies successfully schedule all three vessels, but the arrival-first approach achieves a lower average waiting time for this particular dataset.

The comparison demonstrates the importance of evaluating multiple scheduling strategies rather than assuming that a single greedy ordering will always produce the best result.

For further implementation details, see:

- [System Architecture](ARCHITECTURE.md)
- [Algorithm Complexities](ALGORITHM_COMPLEXITIES.md)
- [Testing and Validation](TESTING.md)