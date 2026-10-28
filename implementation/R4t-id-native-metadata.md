---
type: ImplementationCheckpoint
title: "R4t - ID-native split-metadata access"
description: "Rejected neutral experiment replacing control-pointer metadata indexing with direct task-ID indexing."
tags: [implementation, footprint, task-slot, metadata, rejected]
status: rejected
---

# Baseline

```text
R4s = 2402 B
```

# Experiment

Keep the accepted 16-byte core slot plus two-byte metadata array, but use task
IDs directly in hot paths instead of deriving metadata indices from
`control* - slot_base`.

Generic runtime profiles use their ordinary direct fields.

# Verification

CI run 40665299726 passed:

* generic host tests;
* R4s host tests;
* R4t host tests;
* sanitizers;
* portability;
* R3c/R4s/R4t Cortex-M23 links.

# Measured result

```text
R4s  2402 B
R4t  2402 B
-----------
delta    0 B
```

Symbol-size reports are identical.

# Decision

Reject as a footprint specialization. LTO already recovers equivalent machine
code from the pointer-based representation, so retain the simpler R4s access
surface.
