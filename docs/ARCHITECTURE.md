# System Architecture and Data Flow

## Overview

The **Container Port Berth & Crane Scheduling Suite** is a modular C application designed to manage vessel records, generate feasible berth schedules, allocate shared crane resources, and analyze terminal network operations.

The application separates fleet management, scheduling, resource planning, network analysis, and reporting into independent modules.

## 1. System Architecture

```text
User / Command-Line Interface
│
└── src/main.c
    │
    └── src/core/cli.c — Application Controller
        │
        ├── Fleet Management & Data Handling
        │   ├── io.c          → CSV input and validation
        │   ├── fleet.c       → Vessel CRUD, ID indexing, CSV export
        │   └── report.c      → Schedule reports and statistics
        │
        ├── Scheduling Engine
        │   ├── scheduler.c   → Berth and crane allocation
        │   ├── merge_sort.c  → Arrival/priority ordering
        │   └── port_verify_schedule()
        │                     → Constraint verification
        │
        ├── Resource Planning
        │   ├── interval_partition.c
        │   │                 → Fixed-window berth requirements
        │   └── knapsack.c    → Crane-hour budget optimization
        │
        ├── Terminal Network Analysis
        │   ├── network.c     → Graph representation and operations
        │   ├── dijkstra.c    → Shortest-path calculation
        │   ├── bellman_ford.c
        │   │                 → Shortest-path verification
        │   ├── kruskal.c     → Minimum spanning tree analysis
        │   └── max_flow.c    → Maximum network flow
        │
        └── Performance Evaluation
            └── benchmark.c   → Scenario comparisons
```

The command-line interface coordinates the application modules while keeping their responsibilities separate.

## 2. Core Data Structures

| Structure | Purpose |
|---|---|
| `Vessel` | Stores vessel information, including arrival, duration, crane requirements, priority, and deadline |
| `Assignment` | Stores scheduling results associated with a vessel's array index |
| `Port` | Maintains vessel records, assignments, fleet size, and resource capacity |
| `PortStats` | Stores aggregate scheduling statistics |
| `VesselIndexEntry` | Maps a vessel ID to its original array position for efficient searching |
| `PortNetwork` | Stores terminal node names, directed travel-time weights, and directed capacity information |

The `PortNetwork` representation supports up to 16 terminal nodes.

## 3. Scheduling Workflow

The scheduling engine processes vessel information through the following stages.

### Stage 1 — Data Input and Validation

The application loads vessel records from a CSV file or the interactive fleet editor.

Input validation checks vessel attributes and rejects duplicate IDs or invalid records.

### Stage 2 — Vessel Ordering

Merge sort creates an ordered list of vessel indices using either:

- Arrival-first ordering
- Priority-first ordering

The implementation sorts indices rather than copying complete vessel records.

### Stage 3 — Candidate Time Generation

For each vessel, the scheduler considers candidate start times based on:

- The vessel's arrival time
- Completion times of previously scheduled vessels

### Stage 4 — Resource Feasibility

Each candidate is evaluated against:

- Service duration and deadline
- Available berth capacity
- Simultaneous crane requirements
- Existing berth assignments and overlapping service intervals

### Stage 5 — Assignment

The scheduler commits the first feasible combination of berth, start time, and finish time.

If no feasible allocation is found, the vessel remains unscheduled.

### Stage 6 — Schedule Verification

The `port_verify_schedule()` function independently checks the completed timetable against the scheduling constraints.

This provides an additional validation step after schedule construction.

### Stage 7 — Performance Metrics

The application calculates aggregate measures including:

- Vessel waiting time
- Vessel turnaround time
- Berth utilization
- Crane utilization

### Stage 8 — Report Generation

The reporting module exports vessel-level scheduling results in CSV format and aggregate statistics in JSON format.

## 4. Data Consistency

Fleet modifications can invalidate previously generated scheduling results.

To prevent outdated assignments from being reused, the fleet management module clears existing assignments when a vessel is added, edited, or removed, or when port capacity changes.

When comparing arrival-first and priority-first strategies, the application creates independent copies of the original fleet data.

This ensures both scheduling strategies operate on the same initial conditions.

## 5. Resource Planning

The resource-planning module provides two independent capabilities.

**Interval Partitioning**

Estimates the number of berths required for fixed vessel service windows. This analysis does not account for flexible scheduling or shared crane constraints.

**0/1 Knapsack**

Selects vessel workloads under an aggregate crane-hour budget to maximize the selected container workload.

These tools support planning analysis but do not directly construct the berth timetable.

## 6. Terminal Network Analysis

The network module models terminal connectivity using directed travel-time and capacity matrices.

It provides:

- **Dijkstra:** Shortest paths with nonnegative weights
- **Bellman–Ford:** Independent shortest-path analysis
- **Kruskal:** Minimum spanning tree analysis using an undirected representation
- **Edmonds–Karp:** Maximum directed network flow

These operations are separate from the core vessel scheduling engine.

## 7. Testing and Validation

Automated tests are implemented in `tests/test_main.c`.

The test suite covers:

- CSV input and validation
- Expected results for predefined scheduling scenarios
- Independent scheduling constraint verification
- Vessel editing operations
- CSV export and import consistency
- Sorting, knapsack, and graph algorithm examples
- 100 generated vessel fleets tested with both scheduling strategies

The supplied reference build reports **581 assertions passed with zero failures**.

This result is taken from the existing project documentation and has not been independently rerun.

### Running Tests

Using the Windows build script:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
.\port_tests.exe
```

Using CMake:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

CMake runs the test executable with the project root as its working directory.

## 8. Design Principles

The architecture emphasizes:

- **Modularity:** Independent components for scheduling, fleet management, resource planning, and network analysis.
- **Data Integrity:** Validation of input records and invalidation of outdated schedules.
- **Reproducibility:** Predefined datasets and reference outputs for comparing scheduling strategies.
- **Verification:** Independent constraint checking and automated tests.
- **Extensibility:** Separation of scheduling logic from additional optimization and network analysis tools.

## 9. Related Documentation

- [Main README](../README.md) — Project overview, features, and execution instructions
- [Algorithm Complexities](ALGORITHM_COMPLEXITIES.md) — Algorithm analysis and computational trade-offs
- [Testing Documentation](TESTING.md) — Test coverage and validation
- [Worked Example](WORKED_EXAMPLE.md) — Example scheduling workflow