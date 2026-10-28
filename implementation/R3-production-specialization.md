---
type: ImplementationCheckpoint
title: "R3 - Production-profile validation and service-path specialization"
description: "Measured production-profile reductions from merged R2c, preserving scheduler semantics for valid generated usage while moving selected misuse checks to build/application preconditions."
tags: [implementation, footprint, production-profile, validation, service-path]
status: complete
---

# Baseline

R3 starts from merged R2c:

```text
R2c total = 3376 B
```

Default capacity remains 24 tasks.

# R3 strategy

R3 keeps the generic/debug profiles intact and adds progressively stricter
production profiles.

The compact production profiles preserve scheduler semantics for valid use, but
move selected misuse checks out of runtime code.

Internal dispatch/state corruption checks remain.

# R3a - trusted call context

Production preconditions:

* task APIs are called only from task context;
* interrupt wake is called only from a supported handler context.

Compiled out:

* `arghtos_port_is_task_context()` checks in public task APIs;
* `arghtos_port_is_handler_context()` check in interrupt wake.

Still checked:

* invalid task IDs;
* null profile output;
* invalid sleep duration;
* task-class legality;
* scheduler started state;
* internal kernel invariants.

CI run 40652922833 passed all gates.

Measured:

```text
R2c  3376 B
R3a  3204 B
-----------
saved  172 B
```

Breakdown of the saving:

* portable kernel/data: 2076 -> 1962 B, saving 114 B;
* Cortex-M23 port C: 340 -> 282 B, saving 58 B because unused context-query
  functions dead-strip from the production profile.

# R3b - specialized service legality

Additional production preconditions:

* a task issues only services legal for its generated class;
* wake IDs are valid configured async task IDs;
* the port invokes kernel services only for the currently running task.

Compiled out:

* `current_running()` service-entry validation;
* wake ID/class checks inside `wake_locked()`;
* WAIT/SLEEP async-class checks;
* TASK_RETURN periodic-class check.

Still checked:

* FAULTED wake state;
* unknown service IDs;
* invalid sleep horizon;
* internal dispatch/state corruption.

CI run 40653262699 passed all gates.

Measured:

```text
R3a  3204 B
R3b  3164 B
-----------
saved    40 B
```

Largest direct changes:

```text
wake_locked              140 -> 112 B
arghtos_kernel_service   284 -> 272 B
```

# R3c - minimal production validation

Additional production preconditions:

* init occurs once;
* start occurs once after successful init;
* interrupt wake occurs only after scheduler start;
* profile output pointer and task ID are valid;
* sleep_for duration is less than half-range;
* sleep_until future deadlines lie within the legal half-range.

Compiled out:

* repeated-init check;
* start lifecycle guard;
* pre-start interrupt-wake check;
* profile pointer/ID checks;
* sleep_for duration-range check;
* service-side sleep deadline-range check;
* unused duration-validation helper in this profile.

Still retained:

* context-construction failure check;
* FAULTED wake handling;
* unknown service ID fatal handling;
* dispatch current-task/context/state invariants;
* task trampoline ID invariant;
* fatal recursion handling;
* timer state invariants.

CI run 40653715778 passed all host/debug/R1/R2/R3 profiles, sanitizers,
portable-source checks, Cortex-M23 builds, footprint links, and symbol reports.

Measured:

```text
R3b  3164 B
R3c  3072 B
-----------
saved    92 B
```

# R3 total result

```text
R2c  3376 B
R3c  3072 B
-----------
saved   304 B
```

Relative to frozen I0:

```text
I0   4226 B
R3c  3072 B
-----------
saved 1154 B
```

Remaining gap:

```text
3072 - 1536 = 1536 B
```

The current implementation is now exactly 2x the soft target.

# R3c footprint breakdown

```text
kernel code/data        1830 B
Cortex-M23 port C        282 B
context assembly         188 B
board adapter             12 B
unified task slots        576 B
crash record              64 B
linker metadata           16 B
ArghtOS-only MSP         104 B
------------------------------
total                   3072 B
```

# Largest retained R3c symbols

```text
arghtos_static_slots          576 B
process_time_locked           292 B
arghtos_kernel_service        256 B
arghtos_select_next           232 B
arghtos_kernel_dispatch       204 B
arghtos_fatal                 180 B
PendSV_Handler                130 B
arghtos_start                 128 B
wake_locked                   112 B
arghtos_init                  100 B
arghtos_port_crash_fill        84 B
arghtos_task_start_trampoline  76 B
arghtos_profile_get            60 B
```

# R3 decision

R3c is the best measured production profile.

Do not remove more scheduler-internal invariant checks under R3. The remaining
validation code is no longer the dominant footprint lever.

The next useful axis is R4: API/feature specialization and optional-surface
measurement, with every API removal treated as an explicit requirement/contract
decision rather than an implementation optimization.
