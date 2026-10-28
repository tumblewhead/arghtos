---
type: ImplementationCheckpoint
title: "R4s - Split compact task metadata"
description: "Accepted size-first representation splitting the two mutable metadata bytes from the aligned 16-byte task core."
tags: [implementation, footprint, task-state, size-first, accepted]
status: accepted
---

# Baseline

```text
R4q = 2410 B
```

# Experiment

In the minimum-size no-profile unified representation, split the task record
into:

```text
core:
  context
  next_time
  entry
  schedule          = 16 B/task

parallel meta:
  age_credit
  packed status     =  2 B/task
```

At 24 tasks:

```text
core  16 * 24 = 384 B
meta   2 * 24 =  48 B
--------------------
total            432 B
```

The prior 20-byte record cost 480 B, so the raw metadata saving is 48 B.

The representation keeps all 32-bit fields naturally aligned. No packed or
unaligned 32-bit access is used.

# Verification

Static host tests exercise state and age-credit access through the split
metadata representation.

Authoritative CI run:

```text
40664462833  success
```

# Measured result

```text
R4q  2410 B
R4s  2402 B
-----------
saved    8 B
```

Physical task metadata:

```text
480 B -> 432 B   -48 B
```

Executable growth consumes 40 B of that saving.

Representative symbol changes:

```text
wake_locked              96 ->  92 B
arghtos_start            96 -> 100 B
arghtos_init            152 -> 156 B
arghtos_kernel_service  216 -> 224 B
arghtos_fatal           236 -> 232 B
process_time_locked     256 -> 260 B
arghtos_kernel_dispatch 264 -> 292 B
```

# MSP audit

The limiting normal chain remains unchanged:

```text
SVC explicit save            8 B
arghtos_kernel_service      24 B
process_time_locked         48 B
-------------------------------
total                       80 B
```

Other observed frames remain below that bound. Dispatch remains a 24 B frame.

# Decision

Accept R4s for the minimum-size production profile.

```text
linked before MSP  2322 B
MSP                  80 B
-------------------------
R4s total          2402 B
soft target        1536 B
remaining gap       866 B
```
