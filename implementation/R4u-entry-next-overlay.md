---
type: ImplementationCheckpoint
title: "R4u - Entry/next-time overlay"
description: "Accepted minimum-size representation overlaying immutable task entry with runtime next_time and seeding entry through initial Cortex-M23 stacked R0."
tags: [implementation, footprint, task-slot, context, trampoline, accepted]
status: accepted
---

# Baseline

```text
R4s = 2402 B
```

R4s uses:

```text
16 B aligned core/task
 2 B parallel metadata/task
----------------------------
18 B/task
24 tasks = 432 B
```

# Observation

The generated task entry pointer is needed to construct and begin the task, but
the scheduler does not need to look it up after launch.

Conversely, `next_time` is runtime timing state and is not needed before the
initial task context is constructed.

Those lifetimes do not overlap.

# Representation

R4u overlays the two 32-bit values:

```text
context_or_initial_stack_top   4 B
entry_or_next_time             4 B
schedule                       4 B
----------------------------------
core                          12 B/task

parallel age/status metadata   2 B/task
----------------------------------
total                         14 B/task
24 tasks = 336 B
```

Raw metadata saving:

```text
432 B -> 336 B
saving = 96 B
```

# Initial entry transport

For the minimum-size profile only, the port supplies:

```c
arghtos_port_build_initial_context_top_arg(..., initial_arg);
```

The Cortex-M23 port builds the ordinary 16-word initial saved context and writes
the application entry pointer into stacked hardware R0, which is word 8 from
the saved R4 base.

On first exception return, R0 therefore contains the entry pointer and the
portable trampoline is entered as:

```c
_Noreturn void arghtos_task_start_trampoline(arghtos_task_fn_t entry);
```

The trampoline retains `entry` as ordinary C state across periodic
TASK_RETURN suspension/resume.

# Verification

Authoritative CI:

```text
40665626084  success
```

Passed:

* generic host tests;
* accepted R4s host tests;
* R4u overlay host tests;
* ASan/UBSan;
* portability boundary;
* Cortex-M23 compile;
* R3c/R4s/R4u links and symbol evidence.

# Measured result

```text
R4s  2402 B
R4u  2322 B
-----------
saved   80 B
```

Physical task metadata:

```text
432 B -> 336 B   -96 B
```

Executable growth:

```text
+16 B
```

Representative symbol changes:

```text
arghtos_init              156 -> 168 B
arghtos_kernel_service    224 -> 228 B
process_time_locked       260 -> 256 B
arghtos_kernel_dispatch   292 -> 296 B
task_start_trampoline      72 ->  72 B
```

# Stack audit

Normal MSP lower bound remains:

```text
SVC explicit save            8 B
arghtos_kernel_service      24 B
process_time_locked         48 B
-------------------------------
total                       80 B
```

Dispatch remains a 24 B C frame.

The R4u trampoline keeps the entry pointer in R4. R4 is already part of the
normal R4-R11 saved task context, so this adds no persistent PSP stack storage
and no additional task-context frame bytes.

# Decision

Accept R4u for the minimum-size Cortex-M23 production profile.

```text
linked before MSP  2242 B
MSP                  80 B
-------------------------
R4u total          2322 B
soft target        1536 B
remaining gap       786 B
```
