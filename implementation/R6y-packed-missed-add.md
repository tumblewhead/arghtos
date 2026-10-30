---
type: ImplementationExperiment
title: "R6y - packed missed-release add/clamp"
description: "Neutral constant-time arithmetic alternative for compact missed-release saturation."
tags: [implementation, footprint, diagnostics, cortex-m23, r6, rejected]
status: rejected-neutral
---

# Hypothesis

Replace the accepted room/subtraction saturator with:

```text
total = old_missed + count
new_missed = min(total, 15)
```

The addition is overflow-safe under the scheduler delayed-release bound and the
packed low nibble remains untouched by the existing setter.

# Authority

* `architecture/contracts/C5-fault-diagnostics.md`
* `proof/P7-bounded-memory-and-work.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40983178221
conclusion: success
```

Same-revision result:

```text
R6v control
  linked before MSP       1332 B
  ArghtOS-only MSP          64 B
  complete resident       1396 B
  process_time_locked      160 B

R6y challenger
  linked before MSP       1332 B
  ArghtOS-only MSP          64 B
  complete resident       1396 B
  process_time_locked      160 B

delta                        0 B
```

# Decision

Reject R6y as neutral.

The Cortex-M23 optimizer emits an equivalent-size saturation sequence for both
representations. Keep R6v = 1396 B as the physical control.
