---
type: ImplementationExperiment
title: "R6aa - inlined trusted crash payload"
description: "Accepted composition of the trusted selected-port crash-payload proof with forced Cortex-M23 crash-fill inlining."
tags: [implementation, footprint, crash, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

R6z proved that the selected host/Cortex-M23 crash fillers always produce a
nonzero payload of at most ten words, allowing the generated profile to preseed
PORT_STATE_VALID and omit post-fill clamp/zero checks.

Standalone R6z regressed because LTO outlined the 92-byte Cortex-M23 crash
filler.

R6aa keeps the R6z semantics and forces the selected crash filler inline.

# Authority

R6aa adds no new semantic assumption beyond R6z:

* `architecture/contracts/C4-context-switch-port.md`
* `architecture/contracts/C5-fault-diagnostics.md`
* `proof/P2-context-stack-integrity.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40983889248
conclusion: success
```

Same-revision result:

```text
R6v accepted control
  linked before MSP          1332 B
  ArghtOS-only MSP             64 B
  complete resident          1396 B
  arghtos_fatal               156 B

R6z outlined
  complete resident          1400 B
  arghtos_fatal                68 B
  arghtos_port_crash_fill       92 B

R6aa
  linked before MSP          1320 B
  ArghtOS-only MSP             64 B
  complete resident          1384 B
  arghtos_fatal               144 B
  arghtos_port_crash_fill   inlined
```

```text
R6aa delta from R6v = -12 B
```

All host, recursive-fatal, sanitizer, portability, historical-reproduction,
physical-control, symbol-report, and artifact gates passed.

# Decision

Accept R6aa as the current physical-board R6 control at 1384 B.

R6z remains preserved as a useful rejected experiment showing that the proof
alone was insufficient without controlling LTO code shape.
