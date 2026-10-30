---
type: ImplementationExperiment
title: "R6at - cached nibble period decoder"
description: "Rejected-regressive decoder experiment reusing each R6ar table byte across two periodic tasks."
tags: [implementation, footprint, timer, cortex-m23, r6, rejected]
status: rejected-regressive
---

# Hypothesis

R6at keeps R6ar's accepted 11-byte nibble table but caches each packed byte:

* odd periodic ID loads the byte and consumes the low nibble;
* following even ID consumes the cached high nibble.

This halves table loads without changing generated data.

# Measurement

Authoritative physical-board CI:

```text
run:        41017108516
conclusion: success
```

Same-revision result:

```text
R6ar control
  linked before MSP       1228 B
  ArghtOS-only MSP          64 B
  complete resident       1292 B
  arghtos_timer_interrupt  180 B

R6at
  linked before MSP       1252 B
  ArghtOS-only MSP          64 B
  complete resident       1316 B
  arghtos_timer_interrupt  204 B

delta                       +24 B
```

# Decision

Reject R6at as regressive. The cached decoder increases live state/control-flow
cost enough to grow the timer by 24 B.

Keep R6ar = 1292 B as the semantic and physical control.
