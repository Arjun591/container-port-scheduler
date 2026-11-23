# Testing and limitations

Use `./port_tests.exe` from the project root on Windows after building with `scripts/build_windows.ps1`. Use `./port_tests` on Linux after `sh scripts/run_all.sh`. Or run CMake's `ctest --test-dir build --output-on-failure`.

The suite checks (a) scenario file loading, (b) expected scheduled counts for four datasets and two strategies, (c) every scheduled berth/time/deadline/crane constraint through `port_verify_schedule`, (d) missing-file rejection, (e) adding, editing, deleting and refusing invalid changes, (f) CSV save/reload, (g) Merge Sort order, (h) interval partition reuse, (i) knapsack objective and reconstructed selection, (j) Dijkstra vs Bellman–Ford, Bellman–Ford on a negative-edge example, (k) Kruskal MST, (l) Edmonds–Karp max flow, (m) terminal-network loading, and (n) 100 repeatably randomized valid fleets (both heuristics).

**Verified result:** `passed=581, failed=0` using a strict GCC C11 warning-as-error build. CMake + CTest passed. UBSan passed. This does not prove all bugs are absent or that the greedy scheduling is globally optimal.

### Known limitations

1. Integer hours only; no date/timezone handling.
2. Berths are identical; no length, depth or specialized cargo restrictions.
3. Cranes are identical global capacity units; no individually labeled cranes, movement time, downtime, maintenance or energy usage.
4. Durations and container totals are given, not predicted from crane productivity.
5. Ships cannot pause service (no preemption).
6. The two greedy orderings prioritize different vessels, but neither optimizes a single formal global objective exactly.
7. A job that cannot be inserted around earlier commitments is marked unscheduled, even if another global combination could be better.
8. 256-vessel max, 16 berths, 64 cranes, 16 network nodes and 5000 crane-hours budget in the DP planner.
9. CSV is simple comma-separated text; no quoted commas in names and no escaped record fields.
10. DP selection, fixed-window berth planning and network-analysis features are distinct from the timetable engine.
11. The network CSV is fixed in menu option 13; update `data/terminal_network.csv` to change routing information.
12. Benchmark records CPU time, not a statistically controlled OS-level elapsed-time experiment.
