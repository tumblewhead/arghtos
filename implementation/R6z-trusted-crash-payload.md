---
type: ImplementationExperiment
title: "R6z - trusted selected crash payload"
description: "Rejected-regressive selected-port specialization removing redundant crash payload range checks."
tags: [implementation, footprint, crash, cortex-m23, r6, rejected]
status: rejected-regressive
---

# Hypothesis

The selected verification and Cortex-M23 crash fillers prove:

```text
1 <= port_words <= 10
```

R6z therefore preseeds PORT_STATE_VALID, ORs TASK_VALID into that flag, and
removes the portable post-fill clamp and zero-payload test while retaining the
R6m commit-word recursion guard.

# Authority

* `architecture/contracts/C4-context-switch-port.md`
* `architecture/contracts/C5-fault-diagnostics.md`
* `proof/P2-context-stack-integrity.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40983706583
conclusion: success
```

Same-revision result:

```text
R6v control
  linked before MSP          1332 B
  ArghtOS-only MSP             64 B
  complete resident          1396 B
  arghtos_fatal               156 B
  arghtos_port_crash_fill   inlined

R6z challenger
  linked before MSP          1336 B
  ArghtOS-only MSP             64 B
  complete resident          1400 B
  arghtos_fatal                68 B
  arghtos_port_crash_fill       92 B

delta                          +4 B
```

# Interpretation

The proof removed portable checks, but LTO responded by outlining the selected
port crash filler. The two linked symbols total 160 B, exactly four bytes more
than R6v's 156-byte inlined fatal path.

# Decision

Reject standalone R6z as regressive.

The semantic specialization remains valid and is worth recomposing with an
explicit crash-fill inline policy. Keep R6v = 1396 B as the accepted physical
control until that composition is measured.
