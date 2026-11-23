# Container Port Berth & Crane Scheduling Suite

A C-based maritime logistics simulation for vessel scheduling, berth allocation, crane resource management, and terminal network analysis.

The application provides an interactive fleet management interface, compares two greedy scheduling strategies, and includes algorithmic tools for resource planning and network optimization.

**Language:** C (ISO C11)  
**Build Systems:** GCC, CMake, Make  
**Platform:** Windows, Linux, macOS  
**Entry Point:** `src/main.c`

## Key Features

- **Vessel Management:** Add, edit, delete, search, import, and export vessel records.
- **Berth Scheduling:** Generate vessel timetables using arrival-first and priority-first greedy strategies.
- **Crane Allocation:** Account for shared crane availability and individual vessel requirements.
- **Resource Planning:** Estimate berth requirements and select vessel workloads within crane-hour budgets.
- **Terminal Network Analysis:** Calculate shortest paths, spanning trees, and maximum network flow.
- **Performance Evaluation:** Compare scheduling strategies across normal, capacity-shock, stress, and adversarial datasets.
- **Reporting:** Export vessel schedules in CSV format and aggregate performance metrics in JSON.

## Algorithms Implemented

| Algorithm | Application |
|---|---|
| Arrival-First Greedy Scheduling | Assigns vessels according to arrival order |
| Priority-First Greedy Scheduling | Prioritizes vessels when generating feasible schedules |
| Merge Sort | Sorts vessel indices for scheduling |
| Binary Search | Locates vessels through a sorted ID index |
| Interval Partitioning | Estimates berth requirements for fixed service intervals |
| 0/1 Knapsack (Dynamic Programming) | Selects vessel workloads under a crane-hour budget |
| Dijkstra's Algorithm | Computes shortest paths in a nonnegative terminal network |
| Bellman–Ford Algorithm | Provides an independent shortest-path comparison |
| Kruskal's Algorithm | Constructs a minimum spanning tree using an undirected network representation |
| Edmonds–Karp Algorithm | Computes maximum flow through a directed terminal network |

The berth scheduling engine uses greedy scheduling and merge sort. The remaining algorithms support fleet lookup, resource planning, and terminal network analysis.

## Scheduling Model

Each vessel is represented using the following attributes:

`id`, `name`, `arrival`, `duration`, `cranes`, `priority`, `containers`, and `deadline`.

The scheduling engine enforces these constraints:

- Each vessel occupies one berth throughout its service interval.
- A vessel receives its requested number of cranes throughout its service interval.
- Total simultaneous berth and crane allocations cannot exceed available capacity.
- Service cannot begin before a vessel arrives.
- Deadlines must be respected when specified.
- Service is non-preemptive.

All scheduling times are represented as integer hours.

Both scheduling approaches are heuristics designed to generate feasible allocations. They do not guarantee globally optimal schedules.

## Installation and Execution

### Requirements

- GCC compiler with C11 support
- PowerShell for the Windows build script, or CMake/Make for alternative builds

### Windows — PowerShell

Clone the repository:

```powershell
git clone https://github.com/Arjun591/container-port-scheduler.git
cd container-port-scheduler
```

Build the application:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
```

Launch the interactive menu:

```powershell
.\main.exe --menu
```

Additional commands:

```powershell
.\main.exe --demo
.\main.exe --benchmark
.\main.exe --network
.\main.exe --run data\normal.csv
.\port_tests.exe
```

Run these commands from the project root so the application can locate its datasets.

### Alternative CMake Build

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
.\build\main.exe --menu
ctest --test-dir build --output-on-failure
```

The MinGW generator requires `mingw32-make`.

### Linux / macOS

```bash
sh scripts/run_all.sh
./main --menu
```

Alternatively:

```bash
cmake -S . -B build
cmake --build build
./build/main --menu
```

## Interactive Application

The command-line interface supports the following operations:

| Option | Operation |
|---|---|
| 1 | View vessel and port information |
| 2 | Load predefined or custom datasets |
| 3 | Add a vessel |
| 4 | Modify vessel information |
| 5 | Delete a vessel |
| 6 | Configure berth and crane capacity |
| 7 | Export fleet data |
| 8 | Search vessels by ID |
| 9 | Generate an arrival-first schedule |
| 10 | Generate a priority-first schedule |
| 11 | Compare scheduling strategies and export results |
| 12 | Perform resource planning |
| 13 | Analyze the terminal network |
| 14 | Run performance benchmarks |
| 0 | Exit |

## Input Data Format

The application accepts CSV datasets containing vessel information and port capacity.

Example:

```csv
#berths=2
#cranes=4
id,name,arrival,duration,cranes,priority,containers,deadline
101,North-Wind,0,5,2,5,500,12
102,East-Star,2,4,1,8,350,15
103,Blue-Ship,4,3,2,3,200,0
```

Custom datasets can be loaded through the interactive menu or command line.

```powershell
.\main.exe --run data\my_fleet.csv
```

Changes made through the fleet editor must be saved explicitly using the export option.

## Benchmark Results

The project includes four scheduling scenarios with different workloads and resource constraints.

| Dataset | Vessels | Arrival-First Served | Priority-First Served |
|---|---:|---:|---:|
| Normal | 9 | 9 | 9 |
| Capacity Shock | 12 | 7 | 5 |
| Stress | 180 | 84 | 77 |
| Adversarial | 24 | 15 | 16 |

These are the reference results recorded for the supplied datasets.

The comparison demonstrates that scheduling performance depends on vessel order, resource constraints, and workload characteristics.

A higher number of served vessels does not necessarily imply better overall performance across waiting time, throughput, and deadline satisfaction.

## Generated Reports

The application exports results to the `outputs/` directory.

| File | Description |
|---|---|
| `arrival_schedule.csv` | Arrival-first vessel assignments and service times |
| `priority_schedule.csv` | Priority-first vessel assignments and service times |
| `arrival_summary.json` | Aggregate arrival-first scheduling metrics |
| `priority_summary.json` | Aggregate priority-first scheduling metrics |

Output files are overwritten when a new scheduling run generates reports with the same names.

Reference examples are available in the `examples/` directory.

## Project Structure

```text
container-port-scheduler/
├── data/                # Input datasets and terminal network
├── docs/                # Architecture, algorithms, testing documentation
├── examples/            # Reference scheduling outputs
├── include/             # Header files
├── outputs/             # Generated scheduling reports
├── scripts/             # Build and execution scripts
├── src/
│   ├── algorithms/      # Algorithm implementations
│   ├── core/            # Scheduling, fleet, reporting, and CLI
│   └── main.c           # Application entry point
├── tests/               # Automated tests
├── CMakeLists.txt
├── Makefile
├── run.bat
└── README.md
```

## Testing

The project includes automated tests covering algorithm behavior, scheduling constraints, input validation, and randomized vessel datasets.

Testing resources include:

- Normal operating scenarios
- Reduced-capacity scenarios
- High-volume stress workloads
- Adversarial scheduling inputs
- Terminal network examples
- Randomized fleet validation

Run the compiled test executable:

```powershell
.\port_tests.exe
```

## Design Limitations

This project is a scheduling simulation and algorithmic analysis tool rather than a production maritime terminal operating system.

Important limitations include:

- Greedy scheduling does not guarantee globally optimal allocation.
- Crane assignments remain constant throughout vessel service.
- Interval partitioning assumes fixed service start times and does not account for shared crane constraints.
- Knapsack performs aggregate crane-hour selection rather than producing a feasible berth timetable.
- The network spanning-tree model uses an undirected representation of directed terminal connections.
- Terminal network weights represent travel-related values rather than infrastructure construction costs.
- The simulation does not model tides, vessel dimensions, crane travel, maintenance, or crew scheduling.
- CSV input and export use a simplified format without full quoted-field support.

## Future Improvements

Potential extensions include:

- Time-varying crane allocation
- Multi-objective scheduling optimization
- Real-world port datasets
- Vessel dimension and berth compatibility constraints
- Interactive timetable visualization
- More advanced scheduling and optimization strategies

## Documentation

Additional technical documentation is available in the `docs/` directory:

- `ARCHITECTURE.md` — System design and component organization
- `ALGORITHM_COMPLEXITIES.md` — Algorithm complexity analysis
- `TESTING.md` — Testing methodology
- `WORKED_EXAMPLE.md` — Example execution and scheduling walkthrough

## Author

**Arjun Sharma**

[GitHub](https://github.com/Arjun591) | [LinkedIn](https://www.linkedin.com/in/arjun-sharma-14ab73377)