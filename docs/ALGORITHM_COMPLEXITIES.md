# Algorithm Analysis and Complexity

This document describes the algorithms implemented in the Container Port Berth & Crane Scheduling Suite, including their applications, computational complexity, memory requirements, and limitations.

The application combines scheduling heuristics, sorting and searching techniques, dynamic programming, and graph algorithms to support vessel allocation, resource planning, and terminal network analysis.

## 1. Complexity Overview

**Notation:**

- `n` — Number of vessels
- `B` — Number of available berths
- `C` — Knapsack crane-hour budget
- `V` — Number of terminal network nodes
- `E` — Number of network edges

| Algorithm | Application | Time Complexity | Extra Space |
|---|---|---|---|
| Arrival-First Greedy Scheduling | Vessel timetable generation | O(n³(log n + B)) conservative bound | O(n) |
| Priority-First Greedy Scheduling | Priority-based vessel allocation | O(n³(log n + B)) conservative bound | O(n) |
| Merge Sort | Ordering vessel indices | O(n log n) | O(n) |
| Binary Search | Vessel lookup using sorted IDs | O(log n) per lookup | O(1) per lookup |
| Interval Partitioning | Fixed-interval berth requirement estimation | O(n²) | O(n) |
| 0/1 Knapsack (DP) | Crane-hour resource planning | O(nC) | O(nC) |
| Dijkstra | Shortest paths in nonnegative directed networks | O(V²) | O(V) |
| Bellman–Ford | Shortest-path analysis and verification | O(V³) | O(V) |
| Kruskal | Minimum spanning tree using an undirected network representation | O(E log E) | O(E + V) |
| Edmonds–Karp | Maximum flow in a directed terminal network | O(V³E) conservative implementation bound | O(V²) |

These estimates describe the implemented approaches, including their data structures. They may differ from the complexity of alternative implementations of the same algorithms.

For binary search, the O(1) extra-space figure excludes the O(n) sorted index and its construction.

## 2. Scheduling Algorithms

### 2.1 Arrival-First Greedy Scheduling

**Implementation:** `src/core/scheduler.c`

Vessels are considered according to their arrival times.

The scheduler attempts to allocate each vessel to a feasible berth while respecting arrival times, berth availability, crane capacity, and deadlines.

**Complexity:** O(n³(log n + B)) — conservative upper bound.

**Space:** O(n) auxiliary working memory.

**Limitation:** The algorithm does not guarantee globally optimal vessel throughput or waiting time.

### 2.2 Priority-First Greedy Scheduling

**Implementation:** `src/core/scheduler.c`

Vessels are considered according to their scheduling priority.

The algorithm uses the same feasibility checks as the arrival-first scheduler but processes vessels in a different order.

**Complexity:** O(n³(log n + B)) — conservative upper bound.

**Space:** O(n) auxiliary working memory.

**Limitation:** Prioritizing vessels does not necessarily maximize the total number of vessels served.

## 3. Scheduling Complexity Derivation

The scheduling process consists of several operations.

**Step 1 — Ordering vessels**

Merge sort orders `n` vessel indices.

Time complexity: O(n log n).

**Step 2 — Candidate start times**

For each vessel, the scheduler examines at most `n + 1` candidate start times, including its arrival and previously scheduled completion times.

**Step 3 — Crane feasibility**

For each candidate, the algorithm may inspect up to `n` accepted vessels, construct up to `2n` capacity events, sort those events, and evaluate the resulting capacity timeline.

Time complexity per candidate: O(n log n).

**Step 4 — Berth feasibility**

The scheduler may examine up to `B` berths and compare the candidate service interval against up to `n` existing assignments.

Time complexity per candidate: O(Bn).

**Step 5 — Combined complexity**

For each vessel:

`O(n × (n log n + Bn))`

Across `n` vessels:

**O(n³(log n + B))**

This is a conservative upper bound. Actual execution time depends on the number of accepted vessels, candidate start times, and overlapping service intervals.

The capacity-event sweep requires O(n) temporary memory.

## 4. Sorting and Searching

### 4.1 Merge Sort

**Implementation:** `src/algorithms/merge_sort.c`

Merge sort orders vessel indices for the scheduling strategies.

It follows a divide-and-conquer approach by recursively dividing the input and merging sorted partitions.

**Recurrence:**

`T(n) = 2T(n/2) + O(n)`

**Time Complexity:** O(n log n) in best, average, and worst cases.

**Space Complexity:** O(n).

### 4.2 Binary Search

**Implementation:** `src/algorithms/binary_search.c`

Binary search locates vessel records using a sorted index of vessel IDs.

**Time Complexity:** O(log n) per lookup.

**Extra Space:** O(1) for iterative lookup.

Building the sorted index typically requires O(n log n) time and O(n) storage.

Binary search requires the index to remain sorted.

## 5. Resource Planning Algorithms

### 5.1 Interval Partitioning

**Implementation:** `src/algorithms/interval_partition.c`

Interval partitioning estimates the minimum number of berths required when vessel service intervals have fixed start and end times.

The implementation scans berth completion times to determine whether an interval can reuse an existing berth.

**Time Complexity:** O(n²).

**Space Complexity:** O(n).

**Limitation:** This calculation assumes fixed service windows and does not model flexible start times or shared crane constraints.

Its result should not be interpreted as the minimum berth requirement for the full scheduling problem.

### 5.2 0/1 Knapsack Using Dynamic Programming

**Implementation:** `src/algorithms/knapsack.c`

The algorithm selects a subset of vessels to maximize container workload under a specified crane-hour budget.

Each vessel can either be selected completely or excluded.

**Time Complexity:** O(nC).

**Space Complexity:** O(nC), including solution reconstruction.

**Limitation:** The algorithm evaluates aggregate crane-hour capacity, not a complete berth timetable. Selected vessels must still satisfy scheduling constraints.

## 6. Terminal Network Algorithms

### 6.1 Dijkstra's Algorithm

**Implementation:** `src/algorithms/dijkstra.c`

Computes shortest paths in a directed terminal network with nonnegative edge weights.

The adjacency-matrix implementation selects the next minimum-distance node through linear scanning.

**Time Complexity:** O(V²).

**Extra Space:** O(V), excluding the stored network.

**Limitation:** Dijkstra's algorithm requires nonnegative edge weights.

### 6.2 Bellman–Ford Algorithm

**Implementation:** `src/algorithms/bellman_ford.c`

Computes shortest paths through repeated edge relaxation and can detect reachable negative-weight cycles.

The implementation scans an adjacency matrix during relaxation rounds.

**Time Complexity:** O(V³).

**Extra Space:** O(V), excluding the stored network.

**Limitation:** The application's graph representation reserves `-1` as a missing-edge sentinel, restricting how that value can be represented as an edge weight.

### 6.3 Kruskal's Algorithm

**Implementation:** `src/algorithms/kruskal.c`

Constructs a minimum spanning tree using edge sorting and a Disjoint Set Union (DSU) structure.

Edges are considered in ascending order of weight, and edges that would create cycles are excluded.

**Time Complexity:** O(E log E).

**Space Complexity:** O(E + V).

**Limitation:** The application converts directed network connections into an undirected representation for spanning-tree analysis. Edge weights represent travel-related values, not monetary infrastructure costs.

### 6.4 Edmonds–Karp Algorithm

**Implementation:** `src/algorithms/max_flow.c`

Computes maximum flow through a directed terminal network using the Ford–Fulkerson method with breadth-first search to identify augmenting paths.

Residual reverse edges allow previously assigned flow to be adjusted.

**Standard Time Complexity:** O(VE²) using adjacency-list traversal.

**Conservative Implementation Bound:** O(V³E), accounting for adjacency-matrix BFS scans.

**Space Complexity:** O(V²).

## 7. Implementation Constraints

The application uses bounded input sizes:

| Parameter | Supported Range |
|---|---|
| Vessels | Up to 256 |
| Berths | 1–16 |
| Cranes | 1–64 |
| Terminal Network Nodes | 2–16 |
| Knapsack Crane-Hour Budget | 0–5000 |

These limits help constrain memory requirements and computational workloads.

## 8. Algorithm Selection and Trade-offs

The system uses different algorithmic approaches according to the problem being solved.

**Greedy Scheduling:** Provides practical feasible schedules but does not guarantee global optimality.

**Divide and Conquer:** Merge sort efficiently orders vessel records for scheduling.

**Binary Search:** Supports efficient vessel lookup after index construction.

**Dynamic Programming:** Knapsack optimizes workload selection under a fixed aggregate resource budget.

**Graph Algorithms:** Support route analysis, network connectivity evaluation, and capacity-flow calculations.

The algorithms serve distinct functions within the application. Resource-planning and terminal-network algorithms are separate from the core berth scheduling engine.

## 9. Summary

The Container Port Berth & Crane Scheduling Suite demonstrates how multiple algorithmic techniques can be integrated into a modular C application.

Its scheduling component focuses on feasible berth and crane allocation, while supporting modules provide searching, resource planning, graph analysis, and performance evaluation.

The documented complexity bounds describe the implemented approaches and their computational trade-offs.