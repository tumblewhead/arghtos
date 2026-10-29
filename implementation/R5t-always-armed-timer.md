---
type: ImplementationCheckpoint
title: "R5t - Always-armed generated timer"
description: "Accepted generated-profile specialization eliminating the no-timer state when at least one periodic task is guaranteed."
tags: [implementation, footprint, timer, generated-config, accepted]
status: accepted
---

# Baseline

```text
R5s = 1702 B
```

# Premise

The trusted generated profile already partitions task IDs by class through R5p
and supplies a compile-time periodic count `P`.

R5t is legal only when:

```text
P >= 1
R5q partitioned time scan is active
```

Therefore every invocation of `process_time_locked()` has at least one
periodic next-release deadline. The timer-selection result can never be empty.

# Experiment

R5t:

* removes the `have_timer` state;
* initializes the best delta to `UINT32_MAX`;
* compares periodic/async deadlines directly;
* always arms the selected deadline;
* removes the final timer-disarm branch.

Other profiles retain the generic possibly-empty timer logic.

# Verification

Authoritative CI:

```text
40730819966  success
```

All host, sanitizer, portability, and Cortex-M23 gates passed.

# Measured result

```text
R5s  1702 B
R5t  1674 B
-----------
saved   28 B
```

Primary symbol change:

```text
process_time_locked  228 -> 200 B
```

# Stack observation

The linked R5t `process_time_locked()` prologue is:

```asm
push {r4-r7,lr}   ; 20 B
sub  sp,#20       ; 20 B
```

so its C frame is 40 B rather than the earlier 48 B frame used in the R5j MSP
audit.

The MSP accounting change is recorded separately as R5u.

# Decision

Accept R5t.

```text
linked before MSP  1602 B
MSP                  72 B
-------------------------
R5t total          1674 B
soft target        1536 B
remaining gap       138 B
```
