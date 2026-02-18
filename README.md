## Restaurant Thread Synchronization (HY345 Operating Systems)

A multithreaded restaurant simulation demonstrating pthread synchronization primitives. Customer groups and a waiter coordinate using mutexes and semaphores.

---

### What It Does

- Multiple customer groups arrive at random intervals (each group is a thread)
- One waiter thread manages table assignments
- Groups wait in a queue if no suitable table is available
- Groups eat for a random time, then leave and free their table
- Uses semaphores for waiter-group synchronization
- Uses mutexes to protect shared table data

---

### Project Structure

```
restaurant-thread-sync/
├── restaurant.h          # Data structures and function declarations
├── restaurant.c          # Core implementation (queues, seating logic, threads)
├── main.c                # Entry point, thread creation
├── Makefile              # Build with gcc
├── CMakeLists.txt        # Alternative build with cmake
└── README.md
```

---

### Building

#### With Make

```bash
make
./restaurant
```

#### With CMake

```bash
mkdir build && cd build
cmake ..
make
./assignment2
```

---

### Synchronization Mechanisms

| Primitive | Usage |
|-----------|--------|
| pthread_mutex_t | Protects table occupancy counters and queue state |
| sem_t (waiter_sem) | Waiter sleeps until groups arrive or tables free up |
| sem_t (group_sem) | Groups block until waiter assigns them a table |

#### Key synchronization points

- Group arrival signals waiter via `sem_post(&waiter_sem)`
- Waiter signals seated group via `sem_post(&group->sem)`
- Group departure signals waiter via `sem_post(&waiter_sem)`
- Table occupancy protected by per-table mutex locks

---

### Thread Functions

- `group_thread_fn()` – arrives, joins queue, waits for table, eats, leaves
- `waiter_thread_fn()` – checks queue, finds suitable tables, seats groups

---

### Course Info

HY345: Operating Systems  
University of Crete, CS Department  
Fall Semester 2025
