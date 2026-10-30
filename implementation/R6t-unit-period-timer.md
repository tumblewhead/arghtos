---
type: ImplementationExperiment
title: "R6t - unit-period timer dominance"
description: "Accepted physical-board specialization proving the generated period-1 task dominates every future timer candidate."
tags: [implementation, footprint, timer, cortex-m23, r6, accepted]
status: accepted
---

# Hypothesis

The generated minimum profile contains a period-1 periodic task at task ID 0.

After each time-engine scan at sampled time `now`, that task's next deadline
is exactly the first future scheduler tick:

```text
next_time[0] = now + 1
```

Every other legal future periodic or async-sleep deadline has positive integer
unsigned delta at least one tick. Therefore no other candidate can precede task
0. Equal one-tick deadlines are harmless because they arm the same absolute
tick.

The minimum profile may therefore eliminate earliest-deadline tracking and arm
`controls[0].next_time` directly after processing due state transitions.

# Authority

* `architecture/contracts/C3-time-timer.md`
* `proof/P4-time-release-correctness.md`

# Measurement

Authoritative physical-board CI:

```text
run:        40923091938
conclusion: success
```

Same-revision result:

```text
R6s physical control
  linked before MSP       1416 B
  ArghtOS-only MSP          64 B
  complete resident       1480 B
  process_time_locked      240 B

R6t
  linked before MSP       1360 B
  ArghtOS-only MSP          64 B
  complete resident       1424 B
  process_time_locked      184 B

linked delta               -56 B
time-engine symbol delta    -56 B
```

All host, sanitizer, portability, historical-reproduction, physical-control,
symbol-report, and artifact gates passed.

# Physical chain

```text
R6q physical  1504 B
R6r           1492 B
R6s           1480 B
R6t           1424 B
```

R6t is 80 B smaller than the corrected physical R6q control and 112 B below the
1536 B target.

# Decision

Accept R6t as the current physical-board R6 control at 1424 B.

Historical legacy-harness values remain separate provenance evidence and are
not mixed into physical deltas.
