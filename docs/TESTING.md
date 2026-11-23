# Testing, Validation and Known Limitations

## Overview

The Container Port Berth & Crane Scheduling Suite includes an automated test suite to validate vessel management, scheduling constraints, algorithm correctness, input handling, and terminal network operations.

Testing covers predefined datasets, algorithm-specific examples, and repeatably randomized vessel workloads.

## 1. Running the Tests

### Windows — PowerShell

From the project root, build the application:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
```

Run the test executable:

```powershell
.\port_tests.exe
```

### Linux

Build and run the project:

```bash
sh scripts/run_all.sh
```

Run the tests:

```bash
./port_tests
```

### CMake / CTest

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 2. Test Coverage

### 2.1 Dataset Loading and Input Validation

The test suite checks:

- Successful loading of predefined scenario files.
- Rejection of missing input files.
- Validation of vessel records and scheduling inputs.
- Loading of terminal network data.

### 2.2 Scheduling Validation

Both arrival-first and priority-first scheduling strategies are tested against four predefined datasets:

- Normal
- Capacity shock
- Stress
- Adversarial

The tests compare scheduled-vessel counts against expected reference results.

The `port_verify_schedule()` function independently validates scheduling constraints, including:

- Berth assignments and overlapping service intervals.
- Vessel arrival times and service durations.
- Deadline compliance.
- Simultaneous crane capacity.

### 2.3 Fleet Management

The suite tests vessel management operations, including:

- Adding vessels.
- Editing existing records.
- Deleting vessels.
- Rejecting invalid modifications.
- Saving vessel data to CSV and loading it again.

These tests help verify that fleet changes are handled consistently.

### 2.4 Sorting and Resource Planning

Algorithm-specific tests cover:

**Merge Sort**
- Correct ordering of vessel indices.

**Interval Partitioning**
- Reuse of available berths for fixed service intervals.

**0/1 Knapsack**
- Correct optimization objective.
- Reconstruction of the selected vessel subset.

### 2.5 Terminal Network Algorithms

The test suite includes known examples for:

| Algorithm | Validation |
|---|---|
| Dijkstra | Shortest-path results |
| Bellman–Ford | Comparison with Dijkstra and a negative-edge example |
| Kruskal | Minimum spanning tree calculation |
| Edmonds–Karp | Maximum-flow calculation |

Terminal network loading is also tested.

### 2.6 Randomized Scheduling Tests

The test suite generates **100 repeatably randomized valid vessel fleets**.

Both greedy scheduling strategies are evaluated against these generated workloads.

The resulting schedules are checked for compliance with the application's scheduling constraints.

Repeatable randomization supports consistent regression testing.

## 3. Validation Results

The supplied project documentation reports the following results:

| Validation | Reported Result |
|---|---|
| Automated assertions | 581 passed, 0 failed |
| GCC C11 warning-as-error build | Passed |
| CMake / CTest | Passed |
| UndefinedBehaviorSanitizer (UBSan) | Passed |

These results are taken from the project's recorded reference build and have not been independently reproduced during this documentation update.

Successful testing increases confidence in the implementation, but it does not establish the absence of all defects or prove that the scheduling heuristics produce globally optimal solutions.

## 4. Known Limitations

### 4.1 Time Representation

Scheduling uses integer hours rather than real calendar dates and timestamps.

Time zones and calendar-specific scheduling are not supported.

### 4.2 Berth Characteristics

Berths are treated as identical resources.

The model does not account for berth length, water depth, or specialized cargo-handling restrictions.

### 4.3 Crane Allocation

Cranes are modeled as identical shared capacity units.

The system does not individually identify cranes or model crane movement, maintenance, downtime, or energy consumption.

### 4.4 Vessel Service Duration

Vessel service durations and container totals are provided as inputs.

The application does not predict service duration using crane productivity or historical operational data.

### 4.5 Non-Preemptive Scheduling

Once a vessel begins service, its operation cannot be paused and resumed later.

### 4.6 Heuristic Scheduling

The arrival-first and priority-first strategies generate feasible schedules according to different vessel orderings.

Neither strategy guarantees a globally optimal solution for vessel throughput, waiting time, or another overall scheduling objective.

### 4.7 Local Assignment Decisions

A vessel may remain unscheduled when it cannot be inserted into the existing timetable.

A different combination of earlier assignments might produce a better overall result, but the greedy scheduling engine does not perform global reassignment or exhaustive optimization.

### 4.8 Input and Resource Limits

| Parameter | Maximum Supported Value |
|---|---:|
| Vessels | 256 |
| Berths | 16 |
| Cranes | 64 |
| Terminal network nodes | 16 |
| Knapsack crane-hour budget | 5,000 |

### 4.9 CSV Format

The application uses a simplified comma-separated format.

Quoted commas in vessel names and escaped CSV fields are not supported.

### 4.10 Independent Planning Modules

The following features operate independently of the core scheduling engine:

- 0/1 knapsack resource selection.
- Fixed-window interval partitioning.
- Terminal network routing and flow analysis.

Their results should not be interpreted as automatically optimized berth timetables.

### 4.11 Terminal Network Configuration

The interactive network analysis feature uses:

`data/terminal_network.csv`

To change the network analyzed by menu option 13, the corresponding dataset must be updated.

### 4.12 Benchmark Timing

The benchmark module records CPU execution time.

These measurements are not equivalent to statistically controlled operating-system-level elapsed-time benchmarks.

Results may vary with hardware, compiler settings, and runtime conditions.

## 5. Testing Scope

The automated suite focuses on correctness, input validation, scheduling feasibility, and reproducible algorithm behavior.

It does not constitute a formal proof of correctness, a complete security assessment, or validation against real-world port operating conditions.

## 6. Related Documentation

- [Project README](../README.md) — Project overview and execution instructions
- [System Architecture](ARCHITECTURE.md) — Application components and data flow
- [Algorithm Complexities](ALGORITHM_COMPLEXITIES.md) — Computational complexity analysis
- [Worked Example](WORKED_EXAMPLE.md) — Example scheduling workflow