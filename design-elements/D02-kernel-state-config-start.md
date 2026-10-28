---
type: CodeModuleDesign
title: "D02 - Kernel state, configuration, initialization, and start"
description: "Design of C8/C9 storage, descriptor validation, initial frame construction, synchronous periodic start, and global ownership."
tags: [design, state, init, startup, memory]
status: draft
---

# Goal

Own every persistent portable scheduler object and establish a fully valid dormant scheduler before any task executes.

# Logical module

```text
CONFIG_STATE
```

# Physical implementation

Primary implementation section in:

```text
src/arghtos_kernel.c
```

Private types/constants in:

```text
src/arghtos_internal.h
```

# Owned resident objects

Exactly one C8 array:

```c
static arghtos_task_control_t controls[ARGHTOS_MAX_TASKS];
```

Exactly one C9 global:

```c
static arghtos_kernel_state_t kernel;
```

No other portable module may define persistent scheduler-global writable state.

# Exported functions

Public:

```c
arghtos_result_t arghtos_init(const arghtos_config_t *config);
void arghtos_start(void);
```

Private helpers, proposed:

```c
static bool task_id_valid(arghtos_task_id_t id);
static bool task_is_periodic(arghtos_task_id_t id);
static bool task_is_async(arghtos_task_id_t id);

static arghtos_result_t validate_config(const arghtos_config_t *config);
static arghtos_result_t validate_descriptor(
    const arghtos_task_config_t *d);

static bool stack_regions_overlap(
    const arghtos_task_config_t *a,
    const arghtos_task_config_t *b);

static void clear_runtime_slot(arghtos_task_control_t *c);
static arghtos_result_t build_initial_contexts(void);
static void start_release_periodic_tasks(arghtos_time_t start);
```

These are logical helpers. The implementation may fold/remove them if doing so reduces resident code without reducing testability.

# Initialization state

Before first successful init:

```text
kernel.flags = 0
kernel.current_task = INVALID
```

After successful `arghtos_init()`:

```text
INITIALIZED = 1
STARTED     = 0
current_task = INVALID

for configured task:
    state = DORMANT
    saved_psp = valid dormant context
    counters/flags = 0
```

No task is READY after init.

# Validation order

The order is deliberately designed so invalid external input cannot cause a pointer dereference too early:

```text
1. config != NULL
2. config->tasks != NULL
3. config->crash_record != NULL
4. 1 <= task_count <= ARGHTOS_MAX_TASKS
5. validate each descriptor scalar/pointer rule
6. call pure port_stack_valid() for each task
7. pairwise stack-overlap validation
8. only after the full external configuration is valid: port_init()
9. copy persistent pointers/count into C9
10. initialize C8
11. synthesize all dormant contexts
12. mark INITIALIZED
```

If any validation fails before port_init, no persistent scheduler state or hardware-port state is committed.

After descriptor and port validation succeeds, initial-context construction is required to be deterministic and infallible for those validated stacks. A zero/impossible saved PSP from the port is an internal integration fault: init must leave INITIALIZED clear and enter/report the deterministic fatal/configuration failure path rather than exposing a partially startable scheduler.

# Pairwise stack overlap

For each configured pair `i<j`, treat stack regions as half-open byte ranges:

```text
[A, A+size)
[B, B+size)
```

They overlap iff:

```text
A < B_end && B < A_end
```

Pointer-end computation must be overflow-safe using integer-address arithmetic appropriate to the 32-bit target.

# Start path

`arghtos_start()` requires:

```text
INITIALIZED=1
STARTED=0
```

Transaction:

```text
start = port_time_now()
kernel.start_time = start

earliest = none

for each configured task:
    if period != 0:
        state = READY
        next_time = start + period
        earliest = min_future(earliest, next_time)

arm/disarm timer
kernel.STARTED = 1
port_start()   # no normal return
```

All periodic task release-0 jobs become READY before first selection.

# Start-time ordering

If equal periods exist, first dispatch still follows C6 task-ID tie break.

The order in which D02 loops over descriptors must not itself become a priority rule.

# Initial context creation

D02 calls D06:

```c
saved_psp = arghtos_port_build_initial_psp(
    stack_base,
    stack_size,
    arghtos_task_start_trampoline,
    arghtos_task_return_guard);
```

D02 stores the returned continuation only after the port reports/guarantees a valid result.

# Restart/re-init policy

V1 supports:

* one successful init;
* one start;
* no scheduler stop;
* no scheduler restart;
* no dynamic reconfiguration.

A second `arghtos_init()` after successful initialization returns `ARGHTOS_ERR_BAD_STATE` unless a future contract explicitly introduces reset/re-init.

# Footprint responsibility

D02 owns the largest resident metadata terms:

```text
controls[] = 20 * ARGHTOS_MAX_TASKS
kernel     = 20
```

C0 descriptor bytes are application-declared but scheduler-required and attributed to the ArghtOS footprint.

D02 must not introduce:

* shadow descriptor arrays;
* precomputed priority arrays;
* ready bitmaps in v1;
* task name tables;
* per-task debug state outside C8.

# Dependencies

* C0, C4, C8, C9.
* P0, P1, P2, P7.
* D06, D08.

# Dangerous code sections and tester guidance

## 1. Stack-end pointer arithmetic overflow

**Bad implementation:**

```c
uint8_t *end = d->stack_base + d->stack_size;
```

If `stack_base` is near the top of address space, C pointer arithmetic itself may be invalid or wrap after conversion.

**Tester should:**

* host-test synthetic addresses near `UINTPTR_MAX`;
* verify overflow is rejected as BAD_CONFIG;
* test zero, 67, 68, 69-byte stack sizes;
* test exact-touch ranges where A_end == B_start and confirm they are non-overlapping.

## 2. Partial initialization leaks valid-looking state

**Failure example:** descriptor 14 is invalid, but controls[0..13] already contain saved PSPs and INITIALIZED was set early.

A caller accidentally invokes start and executes a partially built table.

**Tester should:**

* inject a bad descriptor at every possible index;
* after failure, assert STARTED=0 and INITIALIZED=0;
* assert `arghtos_start()` refuses to proceed;
* verify no timer is armed.

## 3. Start releases periodic tasks one-by-one while dispatch is already possible

**Bad implementation:** mark task 0 READY, enable scheduling, then continue initializing task 1.

A higher-priority later descriptor could lose the first dispatch.

**Tester should:**

* configure equal and unequal periods in shuffled descriptor order;
* verify all periodic tasks are READY before the first selector invocation;
* instrument selector call count and assert it is zero during the release loop.

## 4. Pairwise overlap test uses inclusive ends

**Bad logic:**

```text
A_end >= B_start
```

This falsely rejects adjacent stacks.

**Tester should:**

* test [0x1000,0x1100) and [0x1100,0x1200) as legal;
* test one-byte overlap as illegal;
* test identical bases;
* test one range fully contained in another.

## 5. Runtime task_count mistakenly shrinks controls footprint in reports

**Failure example:** build capacity 32, configure 4 tasks, footprint report claims C8 costs 80 B.

The actual static array still costs 640 B.

**Tester should:**

* inspect ELF symbol size of `controls`;
* report both capacity C and configured count N;
* reject a footprint report that uses `20*N` for C8 unless the physical design has explicitly changed.

## 6. Start time read twice

**Failure example:** each periodic task uses a separate `port_time_now()` call when computing its first epoch.

Then "synchronous" tasks have different phase origins.

**Tester should:**

* mock `port_time_now()` to increment every call;
* verify start calls it once for the common epoch before release setup;
* assert all periodic next releases equal `start + period_i`.

# Test seam

Host build should permit injecting:

* fake task descriptor arrays;
* fake stack addresses;
* fake port-init result;
* fake initial-PSP builder;
* deterministic clock.

The module must be testable without executing Cortex exception instructions.

# Boundary specification

D02 owns initialization, persistent portable scheduler state, and the transition from dormant scheduler to first dispatch. It does not select the winning task or manipulate CPU registers.
