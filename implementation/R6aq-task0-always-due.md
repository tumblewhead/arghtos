---
type: ImplementationExperiment
title: "R6aq - timer-entry task0 always due"
description: "Rejected-regressive specialization processing the period-1 task unconditionally at timer entry."
tags: [implementation, footprint, timer, cortex-m23, r6, rejected]
status: rejected-regressive
---

# Hypothesis

C3 guarantees timer service at or after the armed deadline. R6t makes task 0
the period-1 timer-dominant task, R6ag makes the timer interrupt the sole
time-engine owner, and R6b excludes spurious/disarmed timer entry.

Therefore task 0 is due on every admitted time-engine call.

R6aq processed task 0 directly, set its next ideal release to `now + 1`, and
then scanned periodic IDs 1..P-1 normally.

# Regression

The host late-timer test passed and verified, from a dormant task 0 at
`now = 10`:

```text
state      = READY
next_time  = 11
missed     = 9
```

so the period-1 arithmetic specialization was semantically consistent with the
ordinary catch-up path.

# Measurement

Authoritative physical-board CI:

```text
run:        41015067566
conclusion: success
```

Same-revision result:

```text
R6ap control
  linked before MSP       1254 B
  ArghtOS-only MSP          64 B
  complete resident       1318 B
  arghtos_timer_interrupt  184 B
  schedule8                 22 B

R6aq
  linked before MSP       1294 B
  ArghtOS-only MSP          64 B
  complete resident       1358 B
  arghtos_timer_interrupt  184 B
  schedule8                 22 B
  missed_release_add        40 B

delta                       +40 B
```

# Interpretation

The visible timer symbol did not grow. The changed control-flow shape caused
LLVM/LTO to outline `missed_release_add()` as a new 40-byte symbol; it was
inlined in R6ap.

# Decision

Reject R6aq as regressive.

Do not revisit task0 control-flow duplication unless a materially different
code-generation strategy also prevents the missed-release helper from
outlining. Keep R6ap = 1318 B as the semantic and physical control.
