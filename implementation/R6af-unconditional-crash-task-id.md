---
type: ImplementationExperiment
title: "R6af - unconditional crash task-ID materialization"
description: "Neutral crash-attribution representation experiment."
tags: [implementation, footprint, crash, cortex-m23, r6, rejected]
status: rejected-neutral
---

# Hypothesis

R5n proves `kernel.current_task` is either INVALID or a configured task ID.
R6af therefore always writes that value into the crash record and conditions
only the TASK_VALID flag.

# Authority

* `architecture/contracts/C5-fault-diagnostics.md`
* `proof/P1-task-state-invariants.md`

# Regression

The host profile verifies both directions:

* no current task -> `task_id == INVALID`, TASK_VALID clear;
* dispatched task 0 -> `task_id == 0`, TASK_VALID set.

Recursive-fatal and selected-port payload/format behavior remain unchanged.

# Measurement

Authoritative physical-board CI:

```text
run:        40988335478
conclusion: success
```

Same-revision result:

```text
R6ae control
  linked before MSP       1284 B
  ArghtOS-only MSP          64 B
  complete resident       1348 B
  arghtos_fatal            136 B

R6af
  linked before MSP       1284 B
  ArghtOS-only MSP          64 B
  complete resident       1348 B
  arghtos_fatal            136 B

delta                        0 B
```

# Decision

Reject R6af as neutral. The compiler already emits an equivalent-size task
attribution sequence for the R6ae control.

Keep R6ae = 1348 B as the accepted physical control.
