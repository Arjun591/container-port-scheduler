# Hand-worked scheduling example

Consider **2 berths**, **3 cranes** and these input ships:

| ID | Arrival | Duration | Cranes | Priority | Deadline |
|---|---:|---:|---:|---:|---:|
| A (1) | 0 | 4 | 2 | 2 | 10 |
| B (2) | 1 | 3 | 2 | 5 | 8 |
| C (3) | 2 | 2 | 1 | 3 | 10 |

**Arrival-first (A, B, C):**

1. Place A at berth 1 during `[0,4)` using 2 cranes.
2. B cannot start at hour 1 because 2 (A) + 2 (B) = 4 cranes > 3 available. Place B at berth 1 during `[4,7)`.
3. C arrives at hour 2. It can fit into the earlier gap `[2,4)` on berth 2 using 1 crane alongside A's 2. The scheduler can fill earlier gaps even when a previous vessel was assigned a later hour.

Waiting times: A = `0-0=0`, B = `4-1=3`, C = `2-2=0`, so **average wait = (0+3+0)/3 = 1 hour**.

**Priority-first (B, C, A):**

1. Place B `[1,4)` on berth 1 (2 cranes).
2. Place C `[2,4)` on berth 2 (1 crane), using all 3 cranes during that overlap.
3. A cannot start at hour 0: its four-hour service would overlap B and C and exceed the crane capacity. Earliest accepted candidate is hour 4. Place A `[4,8)`.

Waiting times: B = `1-1=0`, C = `2-2=0`, A = `4-0=4`, so **average wait = 4/3 = 1.33 hours**. Both strategies serve all ships, but different choices produce different waits and assignments.

**Why half-open intervals?** `[0,4)` and `[4,7)` do not overlap, so the same berth can be reused at exactly hour 4. Grouping simultaneous crane event changes ensures the same logic applies to crane counts.

**Why it matters:** greedy order is not a global optimization proof. The result depends on priority and arrival ordering, deadlines and resource pressure.
