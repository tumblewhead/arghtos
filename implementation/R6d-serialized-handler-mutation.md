---
type: ImplementationExperiment
title: "R6d - serialized scheduler-handler mutation"
description: "Accepted generated Cortex-M23 integration specialization removing redundant handler-local PRIMASK serialization."
tags: [implementation, footprint, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

For a trusted generated Cortex-M23 board profile, handler-local
`arghtos_port_critical_enter/exit()` is redundant in
`arghtos_timer_interrupt()` and `arghtos_wake_from_interrupt()` when all
scheduler-mutating maskable handlers execute in one non-preempting priority
class, PendSV is less urgent, and task/SVC mutation retains the ordinary C4
critical section.

The generic profile retains handler-side PRIMASK serialization.

# Authority

The specialization is constrained by:

* `architecture/contracts/C4-context-switch-port.md`
* `architecture/contracts/C2-wake.md`
* `proof/P3-wake-wait-race-freedom.md`
* `proof/P4-time-release-correctness.md`

# Measurement

PR #5 CI run:

```text
run:        40816356031
conclusion: success
```

Same-revision physical result:

```text
R6c control
  linked before MSP                  1450 B
  ArghtOS-only MSP                     64 B
  complete resident                  1514 B
  arghtos_timer_interrupt              32 B
  arghtos_wake_from_interrupt           40 B

R6d
  linked before MSP                  1430 B
  ArghtOS-only MSP                     64 B
  complete resident                  1494 B
  arghtos_timer_interrupt              24 B
  arghtos_wake_from_interrupt           28 B

linked delta                          -20 B
timer symbol delta                     -8 B
interrupt-wake symbol delta            -12 B
```

All retained host, sanitizer, portability, generic Cortex-M23, frozen R5ag,
R6b, R6c, and R6d gates passed.

# Required generated integration preconditions

R6d is valid only when:

1. every maskable handler permitted to mutate ArghtOS scheduler state belongs to
   one non-preempting scheduler-handler priority class;
2. no member of that class can preempt another active member;
3. PendSV is strictly less urgent than the scheduler-handler class;
4. NMI and fatal-fault handlers never call normal scheduler APIs; and
5. task/SVC scheduler mutation retains C4 PRIMASK serialization.

# Decision

Accept R6d as the current R6 minimum-size control at 1494 B.

R5ag remains the immutable merged R5 reference at 1534 B.
