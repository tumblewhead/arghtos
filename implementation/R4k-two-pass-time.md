---
type: ImplementationCheckpoint
title: "R4k - Two-pass time engine"
description: "Rejected size-first experiment trading an additional bounded task scan for centralized timer selection."
tags: [implementation, footprint, timer, bounded-work, rejected]
status: rejected
---

# Baseline

```text
R4h = 2770 B
```

# Experiment

Split `process_time_locked()` into two bounded passes:

1. update periodic releases and async sleep state;
2. rescan tasks to choose the earliest remaining timer deadline.

This intentionally increases worst-case task inspections from roughly `N` to
`2*N` in exchange for simpler branch-local timer bookkeeping.

A mixed periodic plus sleeping-async regression verifies earliest-deadline
selection across both timer classes.

# Measured result

CI run 40661598571 passed.

```text
R4h  2770 B
R4k  2774 B
-----------
delta   +4 B
```

`process_time_locked` grew:

```text
292 B -> 296 B
```

# Decision

Reject. The original one-pass implementation is already smaller on Cortex-M23
with Clang `-Oz`.
