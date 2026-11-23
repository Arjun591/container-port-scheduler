# The ten algorithms — role, prerequisites, complexity and limitations

Notation: `n` = vessels, `B` = berths, `C` = knapsack crane-hour budget, `V` = network nodes, `E` = network edges.

| # | Algorithm / implementation | Applied where | Time complexity | Extra memory |
|---|---|---|---|---|
| 1 | Arrival-first greedy timetable | `scheduler.c` | `O(n^3 (log n + B))` conservative bound | `O(n)` |
| 2 | Priority-first greedy timetable | `scheduler.c` | `O(n^3 (log n + B))` conservative bound | `O(n)` |
| 3 | Merge Sort | `merge_sort.c` sorting vessel indices | `O(n log n)` all cases | `O(n)` |
| 4 | Binary Search | `binary_search.c` sorted ship ID index | `O(log n)` lookup after typical `O(n log n)` index sort | `O(1)` lookup, `O(n)` index |
| 5 | Interval Partitioning | `interval_partition.c` fixed immediate-start windows | `O(n^2)` (linear scan of berth ends) | `O(n)` |
| 6 | 0/1 Knapsack DP | `knapsack.c` aggregate crane-hour portfolio | `O(nC)` | `O(nC)` for reconstruction |
| 7 | Dijkstra, adjacency matrix | `dijkstra.c` shortest nonnegative directed route | `O(V^2)` | `O(V)` |
| 8 | Bellman–Ford, adjacency matrix | `bellman_ford.c` independently checks routing | `O(V^3)` (up to V rounds of V² scans) | `O(V)` |
| 9 | Kruskal + DSU | `kruskal.c` undirected MST proxy | `O(E log E)` | `O(E + V)` |
| 10 | Edmonds–Karp, matrix BFS | `max_flow.c` directed maximum flow | textbook `O(V E^2)` with adjacency lists; this implementation's conservative bound `O(V^3 E)` because each BFS scans a V×V matrix | `O(V^2)` |

## Detailed derivation of the greedy scheduler

The scheduler first orders `n` vessel indices with merge sort: `O(n log n)`.
For each vessel, it tries **at most n+1 candidate start times** (its arrival plus previously scheduled finishes).
For each candidate, checking cranes visits up to `n` accepted ships, sorts up to `2n` capacity events (`O(n log n)`), and checks the resulting sweep.
It may then scan `B` berths, checking each against `n` assignments: `O(Bn)`.
Thus, for each ship, `O(n × (n log n + Bn))`, and across `n` ships, an upper bound of `O(n³(log n + B))`.
Worst-case analysis is deliberately conservative; actual durations depend on accepted jobs and overlapping events. The event sweep's temporary space is `O(n)`.

## Why these are suitable for viva

- **Greedy**: fast enough for student datasets, but not globally optimal.
- **Merge Sort**: classic divide-and-conquer with `T(n)=2T(n/2)+O(n)` → `O(n log n)`.
- **Binary Search**: halves the sorted range at each iteration; builds an index first.
- **Interval partitioning**: the overlapping-at-arrival peak determines the minimum number of berths for non-movable intervals.
- **Knapsack**: optimal substructure over prefix of ships and remaining crane-hour budget; no fractional ships.
- **Dijkstra**: greedy finalization valid for nonnegative weights.
- **Bellman–Ford**: repeated relaxation and negative-cycle detection; matrix storage is intentionally simple for 16 nodes.
- **Kruskal**: sorts edges, includes only those connecting different disjoint sets.
- **Edmonds–Karp**: BFS finds augmenting paths; residual reverse arcs allow flow correction.

Remember: `n` is bounded by 256 in this implementation. Berths are 1–16, cranes 1–64, terminal nodes 2–16, knapsack budget 0–5000.
