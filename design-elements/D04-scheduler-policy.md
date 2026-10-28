---
type: CodeModuleDesign
title: "D04 - Scheduling policy and asynchronous aging"
description: "Design of bounded class-first RMS selection, opportunity-based async aging, tie breaks, and selector testability."
tags: [design, scheduler, rms, async, aging, policy]
status: draft
---

# Goal

Implement C6 as a deterministic, host-testable selector with no hardware dependencies and no storage beyond C0/C8/C9.

# Logical module

```text
POLICY
```

# Physical implementation

Static functions in:

```text
src/arghtos_kernel.c
```

No separate ready queue module exists in v1.

# Private interface

```c
static arghtos_task_id_t select_next(void);
```

Potential helpers:

```c
static arghtos_task_id_t select_periodic(void);
static arghtos_task_id_t select_async_and_age(void);
static uint8_t async_effective_priority(
    const arghtos_task_config_t *d,
    const arghtos_task_control_t *c);
```

These helpers may be folded into one function if smaller.

# Periodic scan

One bounded scan over `kernel.task_count`.

Candidate condition:

```text
descriptor.period != 0
control.state == READY
```

Winner relation:

```text
smaller period wins
equal period -> smaller task ID wins
```

No async field is read for periodic ranking.

# Async scan

Executed only when no periodic READY task exists.

For each configured READY async task:

1. saturating increment `age_credit`;
2. compute effective priority:
   `min(base + age_credit, async_ceiling)`;
3. compare:
   * larger effective priority;
   * then larger age_credit;
   * then smaller task ID.

After winner is chosen, the dispatch transaction resets winner credit to zero when state becomes RUNNING.

# Important mutation note

The async selector is intentionally **not pure** because aging is defined per async dispatch opportunity.

Therefore:

* calling `select_next()` speculatively changes age credit;
* tests/debug code must not call selector merely to inspect who would win;
* D06 must call selector exactly once per actual dispatch decision unless the design is deliberately changed.

This is a critical module invariant.

# Selector call count

One PendSV dispatch decision causes exactly one logical selector evaluation.

If the same outgoing task remains the winner, that still counts as a dispatch opportunity and async credit increments according to C6 only if the periodic READY set was empty.

# Idle result

If no READY task exists:

```text
return ARGHTOS_TASK_ID_INVALID
```

D04 does not execute WFI and does not alter C9 current task itself.

# Complexity

Worst reference path:

```text
periodic scan: N
async scan:    N   only if no periodic READY
----------------
<= 2N inspections
```

No search depends on historical event count or missed-period count.

# Dispatch integration

D09 owns the transaction around the selector:

```text
save/account outgoing
normalize outgoing state if still RUNNING
winner = select_next()
mark winner RUNNING
set run_start
return context
```

D04 itself does not update saved PSP, run_start, or current_task.

# Key goals

* Class-first isolation.
* Fixed RMS ordering.
* Deterministic ties.
* Bounded starvation avoidance.
* No queue/table footprint.
* No timer dependence for aging.

# Dependencies

* C0, C6, C8.
* P5, P6, P8.
* D02/D09.

# Dangerous code sections and tester guidance

## 1. Selector called twice for one dispatch

Because async selection increments credit, a seemingly harmless pattern is wrong:

```c
if (select_next() != INVALID)
    next = select_next();
```

Every READY async task ages twice.

**Tester should:**

* instrument selector invocation count per PendSV;
* require exactly one logical selection per dispatch;
* run a two-async-task trace and compare exact credit values after each dispatch opportunity.

## 2. Async scan runs even when periodic task is READY

**Failure example:** code scans/ages async tasks first, then later notices a periodic winner.

Even though periodic still runs, async credit changes during a non-async opportunity, violating P8 semantics and making aging depend on periodic load.

**Tester should:**

* hold one periodic task READY for 100 selector calls;
* verify all async credits remain unchanged;
* property-test: periodic_ready_count>0 implies zero async control mutation.

## 3. Addition overflow before clamp

**Bad implementation:**

```c
uint8_t raised = base + age_credit;
if (raised > ceiling) raised = ceiling;
```

The 8-bit addition can wrap before the comparison.

**Tester should:**

* use base=7, credit=255;
* assert effective priority equals ceiling, not wrapped low value;
* compile with UBSan plus explicit exhaustive 256-credit sweep.

## 4. Age credit increments WAITING/SLEEPING tasks

**Failure scenario:** selector loops every async descriptor and increments credit without checking READY first.

A sleeping task wakes with huge credit it did not earn.

**Tester should:**

* place tasks in every C7 state with sentinel credits;
* invoke async selection opportunities;
* assert only READY async controls mutate.

## 5. Equal-period tie changes with descriptor memory order

Task ID is descriptor index, so scanning backwards and using `<=` can invert the intended tie.

**Tester should:**

* configure 2-8 equal-period periodic tasks;
* permute unrelated async descriptors;
* assert smallest periodic task ID always wins.

## 6. Starvation through credit reset on preemption bug

C8 says RUNNING -> READY resets credit to zero. That is correct for the task that just ran. But nonselected READY tasks must retain/increase credit.

**Tester should:**

* run three async tasks where A is repeatedly selected/preempted;
* verify B/C credit accumulates;
* demonstrate eventual B/C selection under P8 conditions.

## 7. Debug "peek next task" mutates scheduler state

A debugger/telemetry helper calling the real selector advances async aging.

**Tester should:**

* static-review that selector is private;
* ensure no profile/debug API calls it;
* if a host-only peek helper exists, it must use a nonmutating model function outside production code.

# Test seam

This module should have the strongest host property-test suite because it has no hardware dependencies.

Suggested exhaustive dimensions:

* task_count 1..small N exhaustive, then randomized up to 32;
* all task states;
* period ties;
* async base priorities;
* age credits including 0, ceiling boundaries, 254, 255;
* redundant wakes that do not reset READY credit.

# Boundary specification

D04 decides the winner and mutates async age credit only as specified. It does not release tasks, switch context, or program time.
