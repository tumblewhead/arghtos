---
type: ImplementationExperiment
title: "R6w - early periodic READY normalization"
description: "Rejected-regressive experiment moving periodic RUNNING-to-READY preemption into the guaranteed-dispatch timer transaction."
tags: [implementation, footprint, timer, dispatch, cortex-m23, r6, rejected]
status: rejected-regressive
---

# Hypothesis

On top of R6v, every timer interrupt requests PendSV. R6w therefore moved the
ordinary periodic RUNNING -> READY preemption transition into the timer
transaction for every due periodic task, after computing missed-release
accounting from the old state.

The host profile verified that a RUNNING task became READY in the timer
transaction and was reselected RUNNING by the subsequent dispatcher call.

# Authority

* `architecture/contracts/C3-time-timer.md`
* `proof/P1-task-state-invariants.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40925142670
conclusion: success
```

Same-revision result:

```text
R6v control
  linked before MSP       1332 B
  ArghtOS-only MSP          64 B
  complete resident       1396 B
  process_time_locked      160 B

R6w challenger
  linked before MSP       1336 B
  ArghtOS-only MSP          64 B
  complete resident       1400 B
  process_time_locked      164 B

delta                       +4 B
```

# Decision

Reject R6w as regressive.

The transition movement is semantically admissible in the trusted profile, but
Thumb code generation is four bytes larger than the R6v control. Keep R6v as
the physical control.
