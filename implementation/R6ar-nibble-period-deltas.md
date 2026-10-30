---
type: ImplementationExperiment
title: "R6ar - nibble-packed periodic deltas"
description: "Accepted generated-data specialization packing adjacent periodic deltas into 4-bit nibbles."
tags: [implementation, footprint, generated-config, timer, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The accepted minimum profile consumes periodic periods only in the sequential
ascending-ID timer scan.

With generated ordering and task0 period fixed at one tick, the generator may
store adjacent nonnegative period deltas instead of absolute period bytes when
every delta fits in four bits.

For the canonical 23-periodic fixture, 22 deltas fit in 11 bytes.

# Authority

* `architecture/contracts/C0-static-task-configuration.md`
* `architecture/contracts/C3-time-timer.md`
* `proof/P0-static-configuration-validity.md`
* `proof/P4-time-release-correctness.md`

# Measurement

Authoritative physical-board CI:

```text
run:        41015852511
conclusion: success
```

Same-revision result:

```text
R6ap control
  linked before MSP          1254 B
  ArghtOS-only MSP             64 B
  complete resident          1318 B
  schedule8                    22 B
  arghtos_timer_interrupt     184 B

R6ar
  linked before MSP          1228 B
  ArghtOS-only MSP             64 B
  complete resident          1292 B
  packed schedule8             11 B
  arghtos_timer_interrupt     180 B

delta                         -26 B
```

The direct table saving is 11 B; the remainder comes from decoder/code-layout
effects, including a 4 B timer-symbol reduction.

# Decision

Accept R6ar as the current semantic and physical Cortex-M23 control at 1292 B.
