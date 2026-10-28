---
type: ImplementationCheckpoint
title: "R4 - Profiling-surface specialization"
description: "Experimental measurement of the complete linked footprint attributable to runtime profiling, preserving R3c as the same-revision control."
tags: [implementation, footprint, profiling, specialization, experiment]
status: measured
---

# Baseline

R4 starts from merged R3c on `main`:

```text
R3c = 3072 B
```

Default capacity remains 24 tasks.

# R4a question

How many resident bytes are attributable to the runtime profiling feature if that feature is compiled out while all scheduler semantics, crash logging, periodic missed-release evidence, and async age credit remain?

This is a measurement experiment only. Runtime profiling remains a documented v1 requirement unless the user explicitly changes the requirement after seeing the evidence.

# R4a profile

The experimental profile is R3c plus:

```text
ARGHTOS_RUNTIME_PROFILING=0
```

It compiles out:

* public `arghtos_profile_get()`;
* per-task `total_cpu_ticks`;
* per-task `max_run_ticks`;
* global `run_start`;
* dispatch-side execution accounting;
* profiling snapshot copy code.

It retains:

* crash logging and fatal handling;
* periodic missed-release evidence;
* async age credit;
* time/release behavior;
* wait/wake behavior;
* scheduling policy;
* R3c internal invariant checks.

# Structural observation before link

R3c's 24-byte unified slot contains six profiling data bytes:

```text
total_cpu_ticks  4 B
max_run_ticks    2 B
```

Removing those fields does not imply a 6 B/task linked saving because the remaining 32-bit `entry` and `schedule` fields preserve 4-byte structure alignment.

The R4a no-profile unified slot is therefore asserted at:

```text
20 B/task
24 tasks = 480 B
```

Compared with R3c:

```text
576 B -> 480 B
slot saving = 96 B
```

The no-profile static kernel state is asserted at 8 B instead of 12 B, removing the profiling-only `run_start` field.

The remaining saving must come from linked executable code and any dead-stripped dependencies. Only CI linker/map evidence is authoritative.

# Verification plan

R4a keeps R3c unchanged in the same source revision and adds:

* host R4a static-profile scheduler tests;
* generic/debug tests;
* sanitizer run;
* portability check;
* Cortex-M23 compile;
* R3c target footprint link;
* R4a target footprint link;
* footprint JSON;
* linker map;
* symbol-size report.

# Active evidence

Draft PR: #3.

Authoritative CI run: 40658079082, success.

# Measured R4a result

```text
R3c  3072 B
R4a  2845 B
-----------
saved  227 B
```

Breakdown:

```text
                           R3c    R4a   delta
kernel code/data          1830   1699   -131 B
Cortex-M23 port C          282    282      0 B
context assembly           188    188      0 B
board adapter               12     12      0 B
unified task slots         576    480    -96 B
crash record                64     64      0 B
linker metadata             16     16      0 B
ArghtOS-only MSP           104    104      0 B
------------------------------------------------
total                     3072   2845   -227 B
```

Remaining gap to the 1536 B soft target:

```text
2845 - 1536 = 1309 B
```

Notable symbol changes:

```text
arghtos_profile_get       60 ->   0 B
arghtos_kernel_dispatch 204 -> 160 B
arghtos_select_next      232 -> 224 B
arghtos_init             100 ->  92 B
arghtos_start            128 -> 124 B
arghtos_static_slots     576 -> 480 B
```

The 227 B result is meaningful, but insufficient to reach the target. Profiling remains a documented v1 requirement, so R4a is not promoted canonically without an explicit user decision.
