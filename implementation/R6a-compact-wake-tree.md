---
type: ImplementationExperiment
title: "R6a - compact wake comparison tree"
description: "Neutral Cortex-M23 code-generation experiment against frozen R5ag."
tags: [implementation, footprint, cortex-m23, r6, rejected]
status: rejected-neutral
---

# Hypothesis

The six-state `wake_locked_compact()` transition might compile smaller as an
explicit comparison tree than as the frozen R5ag `switch` when linked for
Cortex-M23 with the minimum-size profile.

There was no intended semantic or contract change.

# Measurement

PR #5 CI run:

```text
run: 40815760686
```

Same-revision physical results:

```text
R5ag control
  linked before MSP       1470 B
  ArghtOS-only MSP          64 B
  resident total          1534 B
  wake_locked_compact       72 B

R6a challenger
  linked before MSP       1470 B
  ArghtOS-only MSP          64 B
  resident total          1534 B
  wake_locked_compact       72 B

delta                       0 B
```

Host R6a, portability, sanitizers, generic Cortex-M23, frozen controls, and the
R6a link all passed.

# Decision

Reject R6a as neutral.

The compiler already produces an equivalent-size branch shape. Do not retain
R6a as a live Makefile/CI profile and do not revisit without a materially
different state representation or hypothesis.
