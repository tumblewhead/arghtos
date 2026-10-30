---
type: ImplementationExperiment
title: "R6x - bounded iterative missed-release saturation"
description: "Rejected-regressive compact missed-counter loop experiment."
tags: [implementation, footprint, missed-release, bounded-work, cortex-m23, r6, rejected]
status: rejected-regressive
---

# Hypothesis

Replace compact missed-release saturation arithmetic with a loop over the packed
four-bit high nibble. Each iteration adds 0x10, preserving state/wake bits, and
the loop is capped at 15 iterations.

The result is exactly `min(15, old + count)`, and P7 proves the work bound is
independent of arbitrary delayed-release history.

# Measurement

Authoritative physical-board CI:

```text
run:        40926398752
conclusion: success
```

Same-revision result:

```text
R6v control
  linked before MSP       1332 B
  ArghtOS-only MSP          64 B
  complete resident       1396 B
  process_time_locked      160 B

R6x challenger
  linked before MSP       1336 B
  ArghtOS-only MSP          64 B
  complete resident       1400 B

delta                       +4 B
```

The host profile passed both exact small-count and saturation checks.

# Decision

Reject R6x as regressive.

The bounded-loop representation is valid, but Cortex-M23 code generation is
larger than the arithmetic R6v control. Keep R6v = 1396 B as the accepted
physical control.
