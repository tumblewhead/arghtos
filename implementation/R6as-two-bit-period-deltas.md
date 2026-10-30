---
type: ImplementationExperiment
title: "R6as - two-bit periodic deltas"
description: "Rejected-regressive generated-data experiment packing four period deltas per byte."
tags: [implementation, footprint, generated-config, timer, cortex-m23, r6, rejected]
status: rejected-regressive
---

# Hypothesis

R6as strengthens R6ar's generated adjacent-period bound from 0..15 to 0..3 and
packs four deltas per byte.

The canonical 22 deltas require only six bytes, versus R6ar's eleven.

# Authority

* `architecture/contracts/C0-static-task-configuration.md`
* `architecture/contracts/C3-time-timer.md`
* `proof/P0-static-configuration-validity.md`
* `proof/P4-time-release-correctness.md`

# Measurement

Authoritative physical-board CI:

```text
run:        41016430756
conclusion: success
```

Same-revision result:

```text
R6ar control
  linked before MSP          1228 B
  ArghtOS-only MSP             64 B
  complete resident          1292 B
  packed schedule8             11 B
  arghtos_timer_interrupt     180 B

R6as
  linked before MSP          1258 B
  ArghtOS-only MSP             64 B
  complete resident          1322 B
  packed schedule8              6 B
  arghtos_timer_interrupt     204 B

delta                         +30 B
```

# Decision

Reject R6as as regressive.

The five-byte data saving is outweighed by the variable two-bit shift/mask
decoder and resulting code/layout growth. Keep R6ar = 1292 B as control.
