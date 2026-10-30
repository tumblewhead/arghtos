---
type: ImplementationExperiment
title: "R6ac - preseeded selected-port crash format"
description: "Accepted generated-record specialization removing the invariant selected-port format write from crash capture."
tags: [implementation, footprint, crash, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The selected port format is immutable for the generated image. The crash record
is already preinitialized, so the format can be materialized there at no
additional resident-storage cost.

# Authority

* `architecture/contracts/C4-context-switch-port.md`
* `architecture/contracts/C5-fault-diagnostics.md`
* `proof/P2-context-stack-integrity.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40984715361
conclusion: success
```

Same-revision result:

```text
R6ab control
  linked before MSP       1316 B
  ArghtOS-only MSP          64 B
  complete resident       1380 B
  arghtos_fatal            144 B

R6ac
  linked before MSP       1308 B
  ArghtOS-only MSP          64 B
  complete resident       1372 B
  arghtos_fatal            136 B

linked delta                -8 B
fatal symbol delta           -8 B
```

Host verification checks its preseeded format both before and after capture.

# Decision

Accept R6ac as the current physical-board R6 control at 1372 B.
