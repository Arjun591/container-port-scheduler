# Container Port Berth & Crane Scheduling Suite — focused DAA edition

**Language:** ISO C11 • **Entry point:** `src/main.c` • **Main executable:** `main.exe` on Windows, `main` on Linux/macOS • **Algorithms:** exactly ten clearly identified DAA components.

This edition deliberately focuses on **real time-based berth scheduling** rather than packing the entire DAA syllabus into a demonstration. It includes the earlier **normal (9 ships)**, **capacity-shock (12 ships)** and **stress (180 ships)** scenarios, plus an adversarial scenario, a terminal graph and a complete interactive fleet editor.

## First: run it in VS Code / PowerShell (Windows)

1. Extract the ZIP. Open **the folder that contains `CMakeLists.txt` and `data/`** in VS Code.
2. Open **Terminal → New Terminal**. If `PS ...\src>` is displayed, go back to the project root using `cd ..`.
3. Confirm that `gcc --version` displays a compiler version (MinGW-w64 GCC is recommended).
4. Paste the following into the root terminal:

   ```powershell
   powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
   ```

5. Run the single compiled application:

   ```powershell
   .\main.exe --menu
   ```

6. Other useful commands:

   ```powershell
   .\main.exe --demo
   .\main.exe --benchmark
   .\main.exe --network
   .\main.exe --run data\normal.csv
   .\port_tests.exe
   ```

**Important:** The entire application starts from `src/main.c`, but `main.c` alone cannot be compiled without the other `.c` files. The build script compiles and links all modules together into **one** executable. Run it from the **project root** so it finds `data/`.

**Alternative CMake/MinGW build:**

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
.\build\main.exe --menu
ctest --test-dir build --output-on-failure
```

The CMake generator requires `mingw32-make`. If only GCC is installed, use the PowerShell build script instead. Visual Studio generators may create `build\Debug\main.exe`.

## Linux / macOS build

```bash
sh scripts/run_all.sh           # build + tests + demo + network + benchmark
./main --menu                   # interactive menu
# Or: cmake -S . -B build && cmake --build build
# Then: ./build/main --menu
```

## What the menu does

| Option | Meaning |
|---|---|
| 1 | List all vessels, number of berths, number of cranes |
| 2 | Load normal / capacity shock / stress / adversarial / custom CSV |
| 3 | Add a ship interactively (validated) |
| 4 | Edit an existing ship, including ID |
| 5 | Delete ship by ID |
| 6 | Change 1–16 berths or 1–64 cranes (rejects incompatible vessel demand) |
| 7 | Save current ship list and port capacity as a CSV |
| 8 | Find ship by ID using a sorted index + binary search |
| 9 | Create an actual berth timetable using arrival-first greedy |
| 10 | Create an actual berth timetable using priority-first greedy |
| 11 | Compare both scheduling strategies and export CSV/JSON |
| 12 | Resource planning: fixed-interval berth requirement or 0/1 knapsack |
| 13 | Terminal graph: Dijkstra, Bellman–Ford, Kruskal, Edmonds–Karp |
| 14 | Benchmark normal, capacity shock, stress |
| 0 | Exit |

## The exactly ten algorithmic components

1. **Arrival-first greedy placement** — actual scheduling.
2. **Priority-first greedy placement** — actual scheduling comparison.
3. **Merge Sort** — orders vessel indices for both scheduling strategies.
4. **Binary Search** — searches a sorted index of vessel IDs.
5. **Interval Partitioning** — computes minimum berths for FIXED immediate-start service windows (planning only).
6. **0/1 Knapsack DP** — maximizes containers selected under a total crane-HOUR budget (planning only).
7. **Dijkstra** — shortest paths in a nonnegative directed terminal network.
8. **Bellman–Ford** — independent cross-check for network shortest paths.
9. **Kruskal** — undirected network spanning-tree design proxy.
10. **Edmonds–Karp** — maximum directed terminal flow.

Algorithms 5–10 do not secretly run inside the berth scheduling engine; they are real, accessible **planning and network features** of the same application. The scheduling engine uses algorithms 1–3. Binary search supports fleet lookup.

## Models and constraints

* A vessel has `id`, `name`, `arrival`, `duration`, `cranes`, `priority`, `containers`, `deadline`.
* `#berths=N` means **N simultaneously occupied berths** are possible.
* `#cranes=N` means **N simultaneous shared quay cranes** available across berths.
* Each vessel occupies **one berth for the entire** `[start, finish)` service interval.
* Each vessel receives its requested number of cranes **throughout** its service interval; there is **no time-varying crane allocation**.
* `start >= arrival`, `finish = start + duration`, and `finish <= deadline` when deadline is nonzero.
* No preemption, crane travel, tide restrictions, berth depth/length constraints, costs, crew shifts or maintenance are modelled.
* All times are integer **hours**, not actual calendar dates.
* Both schedulers are **heuristics** that find feasible allocations; they do **not guarantee a globally optimal** number of served vessels or average waiting time.

## Input CSV format

```csv
#berths=2
#cranes=4
id,name,arrival,duration,cranes,priority,containers,deadline
101,North-Wind,0,5,2,5,500,12
102,East-Star,2,4,1,8,350,15
103,Blue-Ship,4,3,2,3,200,0
```

Create your own file in `data/` and use **menu option 2 → 5** or `--run data/my_fleet.csv`. Menu options 3–7 additionally support editing without touching the CSV manually. Saving your changes is **not automatic**: use option 7. Vessel names must be nonempty and cannot contain commas. Quoted CSV fields are not supported.

## Generated outputs

Option 11, `--demo` and `--run` create files under `outputs/`:

- `arrival_schedule.csv` — per-ship berth, start, finish and status.
- `priority_schedule.csv` — equivalent priority-first timetable.
- `arrival_summary.json` — aggregate metrics, no per-vessel assignments.
- `priority_summary.json` — priority-first aggregate metrics.

These filenames are **overwritten** on the next run; save copies before changing datasets. The example outputs are in `examples/`.

## Reference results from the verified build

| Dataset | Ships | Arrival-first served | Priority-first served |
|---|---:|---:|---:|
| Normal | 9 | 9 | 9 |
| Capacity shock | 12 | 7 | 5 |
| Stress | 180 | 84 | 77 |
| Adversarial | 24 | 15 | 16 |

CPU timings are hardware-dependent. A high number of served ships is not the sole objective: waiting time, throughput and deadlines may trade off.

## Folder layout

```text
Container_Port_Scheduler_DAA_Focused/
  include/              public headers (Port, algorithms, terminal network, CLI)
  src/main.c            ONE program entry point
  src/core/             CSV, scheduler, reports, interactive fleet, CLI, benchmark, network
  src/algorithms/       eight focused source files implementing ten components
  data/                 normal, capacity shock, stress, adversarial, network, custom example
  tests/test_main.c     known examples, input rules, 100 randomized fleets
  docs/                 code atlas, menu tutorial, complexity, 8-minute script, viva prep
  examples/             reference schedule output files
  scripts/              PowerShell build and Unix verification
  outputs/              files exported at runtime
  CMakeLists.txt         cross-platform build system
  Makefile               Unix make alternative
```

## Design limitations (important viva answers)

* A heuristic is **not** an optimal scheduler. Priority-first might serve **fewer** ships than arrival-first; the capacity-shock dataset demonstrates this.
* Interval partitioning assumes **fixed starts at arrivals** and ignores shared crane constraints; don't present its number as the minimum berths for flexible scheduling.
* Knapsack makes an **aggregate crane-hour selection**, not a timetable. Selected ships must still undergo feasible berth scheduling separately.
* Dijkstra requires nonnegative route weights, which the terminal CSV loader enforces; Bellman–Ford supports a wider mathematical class of weights (with `-1` reserved as the missing-edge sentinel).
* Kruskal reduces directed links to an undirected proxy and uses the minimum directed travel time per connected pair. This is a demonstrative *weight* and **not a monetary infrastructure construction cost**.
* The `port_save_csv()` export is simple, comma-delimited, and does not implement full RFC 4180 quoting.

For explanations and exam questions, read `docs/START_HERE.md` and the **separately delivered detailed Word viva guide**.
