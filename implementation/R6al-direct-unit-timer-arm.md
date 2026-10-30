---
type: ImplementationExperiment
title: "R6al - direct unit-tick timer arm"
description: "Neutral substitution of now+1 for the proved-equal stored period-1 deadline."
tags: [implementation, footprint, timer, cortex-m23, r6, rejected]
status: rejected-neutral
---

# Hypothesis

R6t proves that after time processing:

```text
next_time[0] = now + 1
```

R6al therefore arms `now + 1` directly rather than reloading
`next_time[0]`.

# Authority

* `architecture/contracts/C3-time-timer.md`
* `proof/P4-time-release-correctness.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40996547586
conclusion: success
```

Same-revision result:

```text
R6ak control
  linked before MSP       1264 B
  ArghtOS-only MSP          64 B
  complete resident       1328 B
  timer interrupt          184 B

R6al
  linked before MSP       1264 B
  ArghtOS-only MSP          64 B
  complete resident       1328 B
  timer interrupt          184 B

delta                        0 B
```

# Decision

Reject R6al as neutral. The optimizer emits an equivalent timer-arm shape.

Keep R6ak = 1328 B as the semantic and physical control.
