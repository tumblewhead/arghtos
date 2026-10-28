---
type: ImplementationCheckpoint
title: "R5e - Overlay async age credit with sleep deadline"
description: "Accepted lifetime overlay storing async age credit in the task entry/next-time union while preserving full 8-bit age semantics."
tags: [implementation, footprint, task-state, lifetime, accepted]
status: accepted
---

# Baseline

```text
R5d = 2270 B
```

# Experiment

In the minimum-size profile, async `age_credit` and async sleep `next_time`
are mutually exclusive in lifetime:

* READY/RUNNING/WAITING/DORMANT async task: age is meaningful, deadline is not;
* SLEEPING async task: deadline is meaningful, age is logically zero.

R5e adds `age_credit` as a third member of the existing R4u union:

```text
pre-init                 entry
periodic runtime         next_time
async sleeping           next_time
async non-sleeping       age_credit
```

The separate metadata array therefore retains only the packed status byte.

Per task:

```text
core slot    12 B unchanged
metadata      2 B -> 1 B
```

At 24 tasks the raw resident-state saving is 24 B.

Sleep transitions clear age before writing the deadline, so the 32-bit deadline
is the final active union value while SLEEPING.

# Verification

Authoritative CI:

```text
40671952282  success
```

Dedicated R5e host tests, sanitizers, portability, and all Cortex-M23 links
passed.

# Measured result

```text
R5d  2270 B
R5e  2202 B
-----------
saved   68 B
```

Resident task metadata:

```text
336 B -> 312 B   -24 B
```

The lifetime change also simplifies linked code:

```text
arghtos_kernel_dispatch 296 -> 272 B   -24 B
arghtos_kernel_service  224 -> 208 B   -16 B
process_time_locked      256 -> 252 B    -4 B
arghtos_fatal            232 -> 228 B    -4 B
arghtos_init             168 -> 164 B    -4 B
wake_locked_compact       80 ->  88 B    +8 B
```

Net code reduction is 44 B in addition to 24 B of task metadata.

# Semantics

R5e retains the full 8-bit age-credit range and therefore does not change
async aging/starvation semantics.

The union member transitions follow task lifecycle, so age and sleep deadline
are never required simultaneously.

# MSP

No new call depth or automatic storage is introduced. The accepted 80 B
ArghtOS-only MSP bound remains valid.

# Decision

Accept R5e.

```text
linked before MSP  2122 B
MSP                  80 B
-------------------------
R5e total          2202 B
remaining gap       666 B
```
