---
type: Contract
title: "C0 - Static task configuration"
description: "Concrete 16-byte immutable task descriptor and static scheduler configuration."
tags: [architecture, contract, task, static-memory]
status: draft
---

# Key goals

* Define every task and stack before `arghtos_start()`.
* Keep task descriptors immutable after initialization.
* Keep all task storage statically allocated.
* Make task ID equal to descriptor-table index.
* Minimize resident configuration footprint because all sections, including read-only data, reside in SRAM on the target.

# Public task entry type

```c
typedef void (*arghtos_task_fn_t)(void);
```

Periodic entries execute exactly one bounded job and return.

Asynchronous entries are persistent workers and must not return during normal operation.

# Concrete 16-byte task descriptor

```c
typedef struct {
    arghtos_task_fn_t entry;       /* 4 B: immutable entry point */
    void             *stack_base;  /* 4 B: lowest address of task stack */
    arghtos_time_t    period;      /* 4 B: 0 => async, nonzero => periodic */
    uint16_t          stack_size;  /* 2 B: bytes, maximum 65535 */
    uint8_t           async_priority;
    uint8_t           flags;       /* reserved, zero in v1 */
} arghtos_task_config_t;

_Static_assert(sizeof(arghtos_task_config_t) == 16u,
               "task descriptor resident-footprint contract changed");
```

Task class is derived rather than stored:

```text
period == 0  => asynchronous
period != 0  => periodic
```

Task ID is the zero-based descriptor-table index. No redundant task ID is stored.

# V1 periodic timing contract

V1 periodic tasks are synchronous rate-monotonic tasks:

* first release occurs at scheduler start;
* phase is zero;
* relative deadline equals period: `D_i = T_i`;
* shorter period means higher RMS priority.

Arbitrary phase offsets and `D != T` are not part of v1. Adding them requires revisiting P6 and the resident-footprint budget.

# V1 asynchronous priority

For asynchronous tasks:

```text
period == 0
async_priority < ARGHTOS_ASYNC_PRIORITY_LEVELS
```

For periodic tasks, `async_priority` must be zero.

# Kernel configuration passed to init

```c
typedef struct {
    const arghtos_task_config_t *tasks;
    struct arghtos_crash_record *crash_record; /* may be NULL */
    uint8_t                      task_count;
    uint8_t                      reserved[3];
} arghtos_config_t;

_Static_assert(sizeof(arghtos_config_t) == 12u,
               "init configuration ABI changed");
```

The `arghtos_config_t` object itself is needed only for the duration of `arghtos_init()`. The task descriptor table and task stacks remain valid for the scheduler lifetime.

C9 copies the persistent task-table pointer, crash-record pointer, and task count into kernel-global state. This avoids retaining a second configuration object.

# Configuration API

```c
arghtos_result_t arghtos_init(const arghtos_config_t *config);
void arghtos_start(void);
```

`arghtos_init()` is called once before scheduling begins.

`arghtos_start()` is called once after successful initialization and does not return during normal operation.

# Concrete scheduler start transaction

Before C4 performs the first task handoff, `arghtos_start()` performs one bounded scan:

1. read `start_time = arghtos_port_time_now()`;
2. store C9 `start_time`;
3. for every periodic descriptor (`period != 0`):
   * DORMANT -> READY;
   * set `next_time = start_time + period`;
4. leave async tasks DORMANT;
5. arm the platform timer for the earliest periodic `next_time`, or disarm if no periodic task exists;
6. set the C9 started flag;
7. call `arghtos_port_start()`.

Thus every synchronous periodic task has release 0 at the common scheduler start epoch and is eligible for the first dispatch.

# Descriptor validity rules

For every descriptor:

* `entry != NULL`;
* `stack_base != NULL`;
* `stack_size >= 68` and satisfies the stricter C4 port check; the 68-byte minimum covers worst-case scheduler context overhead including one hardware alignment word;
* stack regions are pairwise non-overlapping;
* `flags == 0`.

For a periodic descriptor:

* `period > 0`;
* `period < ARGHTOS_TIME_HALF_RANGE`;
* `async_priority == 0`.

For an async descriptor:

* `period == 0`;
* `async_priority < ARGHTOS_ASYNC_PRIORITY_LEVELS`.

# Initialization sequence

Before returning `ARGHTOS_OK`:

1. validate the top-level pointer/count;
2. call `arghtos_port_init()`;
3. validate each descriptor including `arghtos_port_stack_valid()`;
4. perform pairwise stack-overlap checks;
5. copy persistent configuration pointers/count into C9;
6. initialize C8 runtime state;
7. build every initial PSP frame through C4;
8. leave the scheduler timer disarmed until start.

On success:

* all async tasks begin DORMANT;
* all periodic tasks begin DORMANT and become due at scheduler start;
* no task has executed.

# Resident-memory accounting

The descriptor table is read-only by contract but is physically SRAM-resident and counts against the ArghtOS resident-footprint budget.

At 32 configured tasks:

```text
32 * 16 B = 512 B
```

Application task stacks are excluded from the ArghtOS overhead target by explicit requirement, but remain part of total device SRAM consumption.

Application task bodies are likewise not ArghtOS overhead; ArghtOS kernel/port code is.

# Dependencies

[C7](C7-common-types-abi.md), [C9](C9-kernel-global-state.md), [P0](../../proof/P0-static-configuration-validity.md), [P2](../../proof/P2-context-stack-integrity.md), [P6](../../proof/P6-rms-schedulability.md).

# Dangerous aspects

* Treating read-only data as free because it is `const`.
* Overlapping stacks.
* Stack sizes too small for a saved frame plus task execution.
* Reintroducing phase/deadline/budget fields without accounting their per-task SRAM cost.
* Treating task activation as allocation.

# Boundary specification

C0 defines immutable application-supplied scheduler metadata. It is read-only after initialization but is resident SRAM and is included in the scheduler-footprint accounting.
