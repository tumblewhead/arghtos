---
type: ImplementationCheckpoint
title: "R4s - Split byte metadata from aligned core slots"
description: "Accepted C11 representation experiment splitting age/status bytes into a parallel metadata array to reduce per-task resident storage."
tags: [implementation, footprint, task-slot, metadata, accepted]
status: accepted
---

# Baseline

```text
R4q = 2410 B
```

# Experiment

In the no-profile minimum-size profile, split:

```text
age_credit  1 B
status      1 B
```

out of the unified task slot into a parallel two-byte-per-task metadata array.

This changes the physical representation from:

```text
20 B/task * 24 = 480 B
```

to:

```text
core slot: 16 B/task * 24 = 384 B
metadata:   2 B/task * 24 =  48 B
---------------------------------
total task state             432 B
```

All 32-bit context/time/entry/schedule fields remain naturally aligned. No
packed-structure or unaligned-access extension is required.

# Measured result

CI run 40664491170 passed the host split-metadata profile and all active gates.

```text
R4q  2410 B
R4s  2402 B
-----------
saved    8 B
```

Storage saves 48 B, while additional metadata address/index code costs 40 B.

# MSP audit

The limiting chain remains:

```text
SVC explicit save            8 B
arghtos_kernel_service      24 B
process_time_locked         48 B
-------------------------------
total                       80 B
```

Other relevant frames do not increase beyond that bound.

# Decision

Accept split metadata for the minimum-size production profile.

The small net win suggests the next experiment should remove pointer-to-index
metadata calculations by using task IDs directly at call sites that already
possess them.
