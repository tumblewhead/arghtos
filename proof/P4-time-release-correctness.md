---
type: Proof
title: "P4 - Time and release correctness"
description: "Tickless absolute-time scheduling preserves due ordering across wraparound and advances delayed periodic releases with bounded arithmetic."
tags: [scheduler, cortex-m23, proof, tickless, timer, wraparound]
status: draft
---

# Statement

Using a 32-bit unsigned monotonic scheduler clock:

1. absolute due-time comparison is correct across wraparound for legal horizons shorter than half-range;
2. synchronous periodic releases remain locked to their ideal epoch rather than drifting with handler latency;
3. a delayed timer handler advances missed periodic releases in bounded arithmetic, not a loop proportional to elapsed time; and
4. at most one active instance of any periodic task exists.

# Definitions

```text
HALF_RANGE = 2^31
due(now, t) := (int32_t)(now - t) >= 0
```

Every configured period and relative sleep interval is:

```text
0 < interval < HALF_RANGE
```

A timer event must be serviced before its lateness reaches HALF_RANGE. This is an explicit system integration assumption.

# Wraparound proof

Unsigned subtraction is modulo 2^32.

For two relevant timestamps separated by less than HALF_RANGE, exactly one direction around the modular ring represents the valid temporal ordering.

Therefore interpreting `now - deadline` as signed distinguishes before from due/late correctly within the legal horizon.

# Synchronous periodic release epoch

For periodic task `i`:

```text
release_i(k) = scheduler_start_time + k*T_i
```

with `D_i=T_i`.

The next ideal release is stored as `next_time`.

It is advanced from the prior ideal release epoch, never from handler-arrival time. Timer latency therefore does not create cumulative phase drift.

# Bounded delayed-release arithmetic

Suppose `next_time` is due at observed `now`.

Define:

```text
late = now - next_time
releases_due = 1 + floor(late / period)
```

Because service lateness is less than HALF_RANGE and period is nonzero, this calculation is finite and uses constant-time arithmetic.

The next ideal release becomes:

```text
next_time = next_time + releases_due * period    modulo 2^32
```

No loop over `releases_due` is required.

# One active periodic instance

If the periodic task is DORMANT when releases are processed:

* one release makes it READY;
* `releases_due - 1` later releases are counted as missed/overrun releases.

If the task is already READY or RUNNING:

* no new instance is created;
* all `releases_due` are counted as missed/overrun releases.

The saturating C8 missed-release count is updated arithmetically.

# Sleep release

A SLEEPING async task becomes READY iff its absolute sleep deadline is due.

There is only one sleep deadline per task, so no catch-up multiplicity exists.

# Tickless property

The kernel programs the platform for the earliest future timed scheduler event.

Intermediate hardware wakeups needed to bridge a narrower physical timer are a C3 port implementation detail and do not create task releases.

# Architecture test

Tests SHALL cover:

* equality, one tick before, and one tick after a deadline;
* wraparound near zero;
* largest legal interval;
* rejection of intervals at/above HALF_RANGE;
* several tasks due together;
* timer reprogramming to an earlier deadline;
* one, several, and many missed periods using arithmetic advancement;
* periodic task already active at release;
* no release loop whose count depends on `releases_due`.

# Dependencies

P1 consumes the SLEEPING->READY and periodic release transitions.

P6 consumes release jitter bounds.

P7 consumes the constant-time catch-up property.

# Status

The arithmetic proof is architectural.

Implementation discharge requires unit/property tests of the exact 32-bit arithmetic and target confirmation that timer service cannot be delayed by HALF_RANGE ticks.

# R5q generated class-partition scan refinement

Under C0/R5p, task class is proved by the generated ID partition. Replacing one
mixed-class scan with a periodic-prefix scan followed by an async-suffix scan
does not alter any due-time comparison or release transition:

* every periodic task is visited exactly once with its nonzero generated
  period;
* every async task is visited exactly once and only SLEEPING tasks participate
  in deadline processing;
* the same set of future deadlines is presented to the earliest-timer
  selection;
* delayed periodic catch-up remains the same constant-time arithmetic.

Therefore the R5q scan partition is semantically equivalent to the ordinary C3
timer scan and preserves the bounded-work statement.

# R6b timer-entry guard refinement

For the trusted generated minimum-size profile, C3 may require that the
scheduler timer is disarmed before start, STARTED is committed before the
first arm, and a disarmed timer cannot invoke the kernel timer entry.

Given those preconditions:

```text
arghtos_timer_interrupt reachable => scheduler timer previously armed
scheduler timer previously armed  => STARTED already committed
therefore timer entry              => STARTED
```

Eliding the pre-start guard from that profile does not change any reachable
release, catch-up, sleep, timer-selection, or dispatch behavior. The ordinary
portable profile retains the guard for ports that do not make this stronger
integration guarantee.

# R6d timer-handler serialization refinement

In the R6d generated Cortex-M23 profile, the scheduler timer handler may omit
its local PRIMASK enter/exit pair when C4's serialized scheduler-handler
precondition holds.

No other scheduler-mutating handler can preempt the timer transaction, task/SVC
mutation masks the timer handler while active, and PendSV is less urgent.
Therefore the bounded release scan, sleep release, earliest-deadline update,
and dispatch-request decision remain one serialized scheduler transaction.

# R6h single-async suffix refinement

Under R5p/R5ab generated ordering, the trusted minimum profile contains exactly
one async task and it occupies the first ID after the periodic prefix.

Replacing:

```text
for id in [P,N):
    process async timed state
```

with the direct operation at `id=P` visits the same singleton set exactly
once. Therefore async sleep release, earliest-deadline selection, periodic
catch-up, wrap-safe due comparison, and the O(N) bounded-work claim are
unchanged.

# R6q preseeded first-deadline offset refinement

For every generated periodic task i, R6q initializes its pre-start
`next_time` word to `T_i`. No timer/release path is reachable before STARTED.

At the common start epoch S, R6q performs:

```text
next_time_i <- next_time_i + S
            = T_i + S
```

which is identical modulo 2^32 to the ordinary assignment
`next_time_i = S + T_i`.

After that conversion, the existing P4 release/catch-up proof operates on the
same absolute deadlines and the same schedule periods. Therefore release phase,
wrap behavior, missed-release arithmetic, and timer selection are unchanged.

# R6s delta-only timer equivalence

Let the ordinary R5t scan choose candidate `t*` because its unsigned
`delta* = t* - now` is minimal among all considered future deadlines.

R6s performs the identical candidate scan and comparison but stores only
`delta*`. It then arms:

```text
now + delta*
= now + (t* - now)
= t*                  modulo 2^32
```

Therefore R6s arms the exact same 32-bit absolute deadline as the ordinary
always-armed path for every admitted scheduler state.

No release predicate, catch-up count, wake/sleep transition, or wrap-safe
ordering changes. The proof is algebraic and independent of the physical-board
harness; the physical harness merely makes the code-size effect measurable.

# R6t unit-period timer-dominance proof

Assume generated task 0 has period `T0 = 1`.

After each call to the time engine, task 0's next release is the first
representable scheduler tick strictly after `now`.

If task 0 is due:

```text
late     = now - old_next
releases = 1 + late / 1
new_next = old_next + releases
         = old_next + 1 + now - old_next
         = now + 1            modulo 2^32
```

If task 0 is not due, the induction hypothesis from the prior time-engine call
already places its next deadline at the first future tick. Since scheduler time
is integer-valued, this is also `now+1`.

Every other future candidate accepted by C3 has unsigned positive delta at
least 1. Thus:

```text
delta(task0) = 1 <= delta(any other candidate)
```

Therefore arming `next_time[0]` is equivalent to selecting the minimum
deadline across all periodic and sleeping-async candidates.

The optimization removes only minimum-tracking work. Release detection,
periodic catch-up, missed-release accounting, async sleep expiration, and
dispatch-change reporting are unchanged.
