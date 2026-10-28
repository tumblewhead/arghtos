---
type: ImplementationCheckpoint
title: "R4z - Timer-delta recomputation"
description: "Rejected neutral experiment recomputing timer deltas instead of retaining best_delta state."
tags: [implementation, footprint, timer, size-first, rejected]
status: rejected
---

# Baseline

```text
R4y = 2294 B
```

# Experiment

Recompute timer deltas from `best_time - now` rather than retaining a separate
`best_delta` local through timer selection and scheduler start.

The experiment intentionally trades extra arithmetic for the possibility of a
smaller live local/register set.

# Verification

Authoritative same-revision CI:

```text
40667730031  success
```

Host timer/release tests, sanitizers, portability, and Cortex-M23 links passed.

# Measured result

```text
R4y  2294 B
R4z  2294 B
-----------
delta    0 B
```

Major symbol sizes are unchanged.

# Decision

Reject as a minimum-size specialization. The extra recomputation produces no
physical resident saving with the accepted Clang/LLVM `-Oz` + LTO toolchain.

Keep the clearer accepted R4y timer-selection code.
