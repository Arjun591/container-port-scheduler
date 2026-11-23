# System architecture and data flow

```text
User / PowerShell
  └─ src/main.c : main(argc,argv)
      └─ src/core/cli.c : port_cli()
           ├─ fleet editor + input/output
           │    ├─ io.c -> Port (Vessel array)
           │    ├─ fleet.c -> CRUD, ID index, save CSV
           │    └─ report.c -> schedules / statistics / JSON
           ├─ scheduler.c -> first feasible berth/timeslot
           │    ├─ merge_sort.c -> arrival / priority order
           │    └─ port_verify_schedule() -> physical constraint checks
           ├─ interval_partition.c -> fixed-window berth planning
           ├─ knapsack.c -> independent budget optimization
           ├─ network.c -> terminal graph features
           │    ├─ dijkstra.c and bellman_ford.c -> routing
           │    ├─ kruskal.c -> undirected network proxy
           │    └─ max_flow.c -> directed max throughput
           └─ benchmark.c -> scenario comparisons
```

### Structures

`Vessel`: raw input data. `Assignment`: output belonging to the same array index. `Port`: owns both fixed arrays, ship count and capacity counts. `PortStats`: computed aggregates. `VesselIndexEntry`: sorted search pair `(ID, original array index)`. `PortNetwork`: maximum 16 node names, directed travel-time matrix and directed capacity matrix.

### Scheduling life cycle

1. Read a CSV, assign fields, validate every row, reject duplicates.
2. Determine arrival-order or priority-order using Merge Sort on integer indices, not copies of the records.
3. For each ship in that order, create candidate starts from its arrival plus all already assigned completion hours.
4. For each candidate, check duration, deadline, aggregate simultaneous cranes, then each berth for overlap.
5. Commit the first feasible `(berth, start, finish)` or leave it unscheduled.
6. Independently re-check the entire schedule (`port_verify_schedule`).
7. Aggregate wait, turnaround, berth utilization and crane utilization.
8. Export CSV timetable + JSON metric summaries (option 11).

### Data consistency

When the user adds, edits or deletes a vessel or changes capacity, `fleet.c` clears old assignments to avoid accidentally showing a schedule for an outdated fleet. Menu option 11 makes independent copies before comparing strategies so the two outputs are based on the same original data.

### Testing

`tests/test_main.c` tests input, expected scenario counts, independent constraint checking, editing operations, a CSV round trip, known sorting/knapsack/graph examples and 100 generated fleets with both schedulers. The reference build passes **581 assertions** with zero failures. `ctest` runs that test executable with working directory set to the project root.
