---
type: ImplementationExperiment
title: "R6ap - omitted unit-period schedule byte"
description: "Accepted generated-data specialization removing task0's invariant period-1 byte."
tags: [implementation, footprint, generated-config, timer, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

R6t proves generated task 0 has period exactly one scheduler tick. R6ao already
stores only periodic periods, so task 0's stored period byte is redundant.

R6ap stores only periods for periodic IDs 1..P-1 and reconstructs:

```text
id == 0  -> 1
id > 0   -> schedule8[id - 1]
```

# Authority

* `architecture/contracts/C0-static-task-configuration.md`
* `architecture/contracts/C3-time-timer.md`
* `proof/P4-time-release-correctness.md`

# Measurement

Authoritative CI:

```text
run:        41014470378
conclusion: success
```

Same-revision physical result:

```text
R6ao control
  linked before MSP          1255 B
  ArghtOS-only MSP             64 B
  complete resident          1319 B
  arghtos_static_schedule8      23 B
  arghtos_timer_interrupt      184 B

R6ap
  linked before MSP          1254 B
  ArghtOS-only MSP             64 B
  complete resident          1318 B
  arghtos_static_schedule8      22 B
  arghtos_timer_interrupt      184 B

delta                          -1 B
```

# Decision

Accept R6ap as the current semantic and physical Cortex-M23 control at 1318 B.
