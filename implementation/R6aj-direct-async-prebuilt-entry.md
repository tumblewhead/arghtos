---
type: ImplementationExperiment
title: "R6aj - generated direct async prebuilt entry"
description: "Accepted zero-byte-cost repair of the R5z async trampoline defect by discharging task class in the generated Cortex-M23 initial frame."
tags: [implementation, correctness, footprint, trampoline, context, cortex-m23, r6, accepted]
status: accepted
---

# Problem repaired

R6ai proved that the R5z raw schedule lineage misclassified the async suffix
inside the shared task-start trampoline. The direct runtime class repair was
correct but cost 20 B:

```text
R6ag historical size-only  1340 B
R6ai correctness control   1360 B
```

# R6aj design

R6aj moves the class decision into generated initial-context construction.

For periodic slots 0..22:

```text
stacked R0 = application entry
stacked LR = return guard
stacked PC = periodic trampoline
```

For async slot 23:

```text
stacked R0 = unused
stacked LR = return guard
stacked PC = application async entry
```

The periodic trampoline is therefore periodic-only and needs no runtime class
test.

Under R6aj, the existing return guard records
`ARGHTOS_FAULT_ASYNC_RETURNED`. Reaching that LR from the periodic trampoline
is proof-impossible on admitted execution because the periodic trampoline is
non-returning.

# Machine-checked frame evidence

The R6aj physical build runs:

```text
tools/check_r6aj_prebuilt_context.py
```

against the assembled prebuilt-context object.

Authoritative CI output:

```text
R6aj prebuilt-context relocation check: PASS
(23 periodic frames + 1 direct async frame)
```

The checker verifies:

* 23 periodic stacked-PC relocations target
  `arghtos_task_start_trampoline`;
* all 24 LR relocations target `arghtos_task_return_guard`;
* periodic R0 relocations carry `task_stub`;
* async R0 has no entry relocation;
* async stacked PC targets `task_stub`, not the trampoline.

# Host regression

The R6aj host profile directly exercises the return guard and verifies the
committed crash record contains:

```text
reason = ARGHTOS_FAULT_ASYNC_RETURNED
```

# Measurement

Authoritative physical-board CI:

```text
run:        40995123969
conclusion: success
```

Same-revision result:

```text
R6ag historical size-only
  linked before MSP          1276 B
  ArghtOS-only MSP             64 B
  complete resident          1340 B
  trampoline                   18 B
  return guard                  8 B

R6ai runtime correctness control
  linked before MSP          1296 B
  ArghtOS-only MSP             64 B
  complete resident          1360 B
  trampoline                   40 B
  return guard                  8 B

R6aj generated-context repair
  linked before MSP          1276 B
  ArghtOS-only MSP             64 B
  complete resident          1340 B
  trampoline                   18 B
  return guard                  8 B
```

```text
R6aj vs R6ai = -20 B
R6aj vs historical R6ag = 0 B
```

# Decision

Accept R6aj as the current **semantic and physical** R6 control at 1340 B.

The R5z/R5ag historical footprint measurements remain valid provenance, but
their async-return semantic erratum remains attached. R6aj is the repair that
restores the invariant without resident-byte cost.
