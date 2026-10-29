---
type: ImplementationExperiment
title: "R6b - trusted timer-entry guard elision"
description: "Accepted generated-profile specialization removing a proof-redundant pre-start timer ISR guard."
tags: [implementation, footprint, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

For the trusted generated minimum-size profile, the pre-start guard in
`arghtos_timer_interrupt()` is unreachable when the selected port/profile
guarantees:

1. initialization leaves the scheduler timer disarmed;
2. STARTED is committed before the first scheduler timer arm;
3. a disarmed scheduler timer cannot call `arghtos_timer_interrupt()`;
4. unrelated interrupt sources do not call the scheduler timer entry.

The generic profile retains the defensive guard.

# Authority

The specialization is constrained by:

* `architecture/contracts/C3-time-timer.md`
* `proof/P4-time-release-correctness.md`

# Measurement

PR #5 CI run:

```text
run:        40815995202
conclusion: success
```

Same-revision Cortex-M23 physical result:

```text
R5ag frozen reference
  linked before MSP          1470 B
  ArghtOS-only MSP             64 B
  complete resident          1534 B
  arghtos_timer_interrupt      44 B

R6b
  linked before MSP          1458 B
  ArghtOS-only MSP             64 B
  complete resident          1522 B
  arghtos_timer_interrupt      32 B

delta                         -12 B
```

All retained host, sanitizer, portability, generic Cortex-M23, frozen-control,
and challenger gates passed.

# Decision

Accept R6b as the current R6 minimum-size control at 1522 B.

R5ag remains the immutable merged R5 reference at 1534 B.
