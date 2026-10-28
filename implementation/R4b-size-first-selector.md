---
type: ImplementationCheckpoint
title: "R4b - Size-first selector code generation"
description: "Micro-optimization experiment that trades source friendliness for smaller Cortex-M23 selector code while preserving C6 ordering and bounded scans."
tags: [implementation, footprint, selector, size-first, codegen]
status: experimental
---

# Baseline

Accepted minimum-size production baseline:

```text
R4a = 2845 B
```

R4b changes only the selector implementation in the minimum-size target profile.

# Size-first policy

For minimum-size production work, runtime/cycle cost may increase when linked resident size decreases.

Still non-negotiable:

* scheduler semantics;
* C11 defined behavior;
* portable-kernel boundary;
* bounded work required by P7;
* crash integrity;
* wake/wait race behavior.

Readable source is not itself a footprint requirement.

# Selector rewrite

The original selector repeatedly reconstructs the current best async task's
effective score for every challenger and explicitly checks ID ties.

R4b instead:

* uses ascending scan order as the equal-period/equal-score task-ID tie break;
* caches the best periodic period;
* caches the incumbent async effective priority and age;
* keeps every READY async task's one-per-opportunity age increment;
* retains the two bounded scans required by C6.

Generic host tests now explicitly lock:

* equal periodic period -> lower ID;
* equal async effective priority and age -> lower ID;
* all READY async candidates age before selection.

# Measurement

R4a and R4b are linked in the same CI revision.

Authoritative linked result: pending.
