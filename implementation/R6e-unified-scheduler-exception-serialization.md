---
type: ImplementationExperiment
title: "R6e - unified scheduler-exception serialization"
description: "Accepted generated Cortex-M23 specialization removing redundant service-side PRIMASK serialization."
tags: [implementation, footprint, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

R6d already serializes scheduler timer and interrupt-wake mutation through a
generated non-preempting scheduler-handler priority class.

R6e extends that class to SVC. When SVC, scheduler timer, and every
scheduler-mutating maskable device handler cannot preempt one another, the
local `arghtos_port_critical_enter/exit()` pair in
`arghtos_kernel_service()` is redundant.

The generic profile retains explicit PRIMASK serialization.

# Authority

The specialization is constrained by:

* `architecture/contracts/C4-context-switch-port.md`
* `architecture/contracts/C1-task-api-lifecycle.md`
* `proof/P3-wake-wait-race-freedom.md`
* `proof/P1-task-state-invariants.md`

# Required generated integration preconditions

R6e is valid only when:

1. SVC and every scheduler-mutating timer/device handler execute at one
   priority level, or otherwise at priorities that cannot preempt one another;
2. PendSV is strictly less urgent than that scheduler-exception class;
3. NMI and fatal-fault handlers never call normal scheduler APIs;
4. no scheduler-state mutation is performed by any other preempting handler
   class; and
5. application tasks cannot modify the generated exception-priority
   configuration after scheduler start.

# Measurement

PR #5 CI run:

```text
run:        40817081956
conclusion: success
```

Same-revision physical result:

```text
R6d control
  linked before MSP          1430 B
  ArghtOS-only MSP             64 B
  complete resident          1494 B
  arghtos_kernel_service      172 B

R6e
  linked before MSP          1418 B
  ArghtOS-only MSP             64 B
  complete resident          1482 B
  arghtos_kernel_service      160 B

linked delta                  -12 B
service symbol delta          -12 B
```

All retained host, sanitizer, portability, generic Cortex-M23, frozen R5ag,
R6b, R6c, R6d, and R6e gates passed.

# Decision

Accept R6e as the current R6 minimum-size control at 1482 B.

R5ag remains the immutable merged R5 reference at 1534 B.
