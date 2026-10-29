---
type: Proof
title: "P3 - Wake/wait race freedom"
description: "The one-bit pending-wake handshake prevents a wake from being lost at the RUNNING-to-WAITING boundary without preserving event multiplicity."
tags: [scheduler, cortex-m23, proof, interrupt, async, wait, race]
status: draft
---

# Statement

For an asynchronous task, repeated wake requests may collapse into one scheduling indication, but a wake that occurs while the task is RUNNING or while it is committing to WAITING cannot leave the task blocked with that scheduling indication forgotten.

# Concrete wake semantics

A wake targets only an async task.

* DORMANT -> READY.
* WAITING -> READY.
* READY -> READY, no additional queued work.
* RUNNING -> RUNNING and set `wake_pending=1`.
* SLEEPING -> SLEEPING and set `wake_pending=1`.
* FAULTED: error, no state change.

The scheduler never counts repeated wake requests.

# Concrete wait semantics

Inside one serialized scheduler transaction:

```text
if wake_pending == 1:
    wake_pending = 0
    remain RUNNING
else:
    RUNNING -> WAITING
```

# Race proof

Consider a wake `W` that occurs after the worker's final application-level drain check and before `arghtos_wait()` has completed its scheduler transition.

There are only two linearized orderings.

## Wake linearizes before wait

The target is still RUNNING.

C1/C2 set `wake_pending=1`.

When wait executes, it observes the latch, consumes it, and does not enter WAITING.

Therefore the wake is not lost.

## Wait linearizes before wake

Wait commits RUNNING -> WAITING.

The later wake observes WAITING and performs WAITING -> READY.

Therefore the wake is not lost.

No third interleaving exists because the latch test/consume and WAITING commit are serialized with the wake state/latch update by the C4 critical-section contract.

# Repetition semantics

If any number of wakes occur while `wake_pending=1`, they collapse into that one bit.

This is intentional and does not violate the proof because ArghtOS promises only preservation of the proposition:

```text
"the worker may have work"
```

Event multiplicity remains driver/application state.

# Sleep note

A wake while SLEEPING does not end the sleep early. It sets `wake_pending`, which is later consumed when the worker reaches wait after the timed sleep completes.

This is consistent with the v1 distinction:

* wait blocks on event;
* sleep blocks on time.

# Architecture test

A deterministic host model SHALL exercise:

1. wake immediately before wait transaction;
2. wake during the serialized wait transaction;
3. wake immediately after WAITING commit;
4. repeated wakes while RUNNING;
5. repeated wakes while READY;
6. wake while SLEEPING;
7. interrupt wake immediately before exception return.

For cases 1 through 3, after the interleaving the task must either remain RUNNING or become READY; it must not remain WAITING solely because of the race.

# Dependencies

Depends on [P1](P1-task-state-invariants.md) and the C1/C2/C4 concrete handshake contracts.

# Status

The abstract race argument is discharged by the one-bit handshake design.

Implementation discharge still requires host interleaving tests and target validation of the C4 serialization primitive.

# R6d non-preempting handler serialization refinement

P3 requires one total order between the wait transaction and the interrupt
wake transaction. In the R6d generated Cortex-M23 profile that order is still
well defined:

* while SVC performs the wait transaction, its ordinary C4 critical section
  masks scheduler-mutating device handlers;
* while a wake handler performs its transaction, no peer scheduler-mutating
  handler can preempt it by generated exception-priority construction;
* PendSV is less urgent and cannot preempt either transaction.

Therefore the wake transaction remains indivisible with respect to every other
scheduler-state mutation even when the wake handler omits its own redundant
PRIMASK enter/exit pair. The two P3 linearization cases are unchanged.
