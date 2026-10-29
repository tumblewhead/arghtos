---
type: ImplementationExperiment
title: "R6v - unconditional unit-tick dispatch"
description: "Accepted physical-board size-for-cycles specialization removing time-engine change tracking and requesting PendSV every unit timer tick."
tags: [implementation, footprint, timer, dispatch, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

Under the accepted unit-period profile, timer entry may request PendSV every
tick instead of only when the time engine creates new READY work.

When eligibility is unchanged, the current RUNNING task was the selector winner
at the preceding dispatch boundary. With runtime profiling disabled and async
age-reset side effects elided, a redundant PendSV saves that continuation,
performs the ordinary RUNNING -> READY preemption transition, and selects the
same task again unless another scheduling indication was already pending.

That lets the time engine remove its `changed` return and all change tracking.

# Runtime-cost caveat

R6v may add one bounded PendSV/dispatcher pass on a unit timer tick that did not
change eligibility. P6 requires this cost to be included in `K_i(R)`.

This is an intentional size-for-cycles trade.

# Authority

* `architecture/contracts/C3-time-timer.md`
* `proof/P1-task-state-invariants.md`
* `proof/P6-rms-schedulability.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40924511450
conclusion: success
```

Same-revision result:

```text
R6u physical control
  linked before MSP          1356 B
  ArghtOS-only MSP             64 B
  complete resident          1420 B
  process_time_locked         180 B
  arghtos_timer_interrupt      32 B

R6v
  linked before MSP          1332 B
  ArghtOS-only MSP             64 B
  complete resident          1396 B
  process_time_locked         160 B
  arghtos_timer_interrupt      28 B

linked delta                  -24 B
time-engine symbol delta       -20 B
timer ISR symbol delta          -4 B
```

The R6v host profile explicitly verifies that an overrun tick requests dispatch
while leaving the interrupted periodic task RUNNING until the simulated
dispatch occurs.

All host, sanitizer, portability, historical-reproduction, physical-control,
symbol-report, and artifact gates passed.

# Decision

Accept R6v as the current physical-board R6 control at 1396 B.
