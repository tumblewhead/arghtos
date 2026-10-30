---
type: ImplementationExperiment
title: "R6h - singleton async time suffix"
description: "Neutral generated-profile experiment replacing a one-iteration async timer loop with a direct slot operation."
tags: [implementation, footprint, cortex-m23, r6, rejected]
status: rejected-neutral
---

# Hypothesis

The accepted generated profile has exactly one async task after the periodic
prefix. R6h replaced the async suffix loop in `process_time_locked()` with a
direct operation on that singleton slot.

The periodic scan and all sleep/timer semantics were unchanged.

# Authority

The equivalence is documented in:

* `architecture/contracts/C3-time-timer.md`
* `proof/P4-time-release-correctness.md`

# Measurement

PR #5 CI run:

```text
run:        40818103108
conclusion: success
```

Same-revision result:

```text
R6f control
  linked before MSP       1414 B
  ArghtOS-only MSP          64 B
  complete resident       1478 B
  process_time_locked      176 B

R6h challenger
  linked before MSP       1414 B
  ArghtOS-only MSP          64 B
  complete resident       1478 B
  process_time_locked      176 B

delta                        0 B
```

# Decision

Reject R6h as neutral.

The compiler already collapses the one-iteration generated async suffix to an
equivalent code shape. Do not retain R6h as a live footprint challenger.
