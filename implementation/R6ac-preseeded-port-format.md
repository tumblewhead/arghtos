---
type: ImplementationExperiment
title: "R6ac - preseeded selected-port crash format"
description: "Accepted generated-record specialization moving the invariant selected-port crash format out of runtime capture."
tags: [implementation, footprint, crash, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The selected integration uses one immutable crash payload format for the entire
generated image. The 64-byte crash record is already preinitialized, so its
`port_format` field can be materialized before runtime at no storage cost.

Selected crash fillers then omit the runtime format write.

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

The host profile verifies its preseeded `0x484F` value before and after crash
capture. Cortex-M23 uses preseeded `0x4D23`.

# Decision

Accept R6ac as the current physical-board R6 control at 1372 B.
