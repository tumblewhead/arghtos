---
type: ImplementationExperiment
title: "R6i - known-unmasked PendSV entry"
description: "Accepted Cortex-M23 specialization replacing prior-PRIMASK save/restore with known normal mask state."
tags: [implementation, footprint, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

In the trusted generated Cortex-M23 profile, every supported PendSV entry has:

```text
PRIMASK = 0
```

because:

* trusted tasks do not modify PRIMASK;
* startup restores normal interrupt state before first dispatch;
* R6d/R6e scheduler-mutating SVC/timer/device handlers do not leave PRIMASK set;
* the idle loop masks only internally and restores normal state before Thread
  return.

PendSV can therefore mask with `cpsid i` and restore with `cpsie i` without
saving the incoming PRIMASK value.

The generic Cortex-M23 path retains exact prior-mask save/restore.

# Authority

The specialization is constrained by:

* `architecture/contracts/C4-context-switch-port.md`
* `proof/P2-context-stack-integrity.md`

# Measurement

PR #5 CI run:

```text
run:        40818322636
conclusion: success
```

Same-revision physical result:

```text
R6f control
  linked before MSP       1414 B
  ArghtOS-only MSP          64 B
  complete resident       1478 B
  PendSV_Handler           100 B

R6i
  linked before MSP       1406 B
  ArghtOS-only MSP          64 B
  complete resident       1470 B
  PendSV_Handler            92 B

linked delta                -8 B
PendSV symbol delta          -8 B
```

All retained host, sanitizer, portability, generic Cortex-M23, frozen R5ag,
R6b-R6f, and R6i gates passed.

# Decision

Accept R6i as the current R6 minimum-size control at 1470 B.

R5ag remains the immutable merged R5 reference at 1534 B.
