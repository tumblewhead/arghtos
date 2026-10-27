---
type: Proof
title: "P3 - Wake/wait race freedom"
description: "Idempotent asynchronous wakeup does not lose a wake at the transition from servicing an event to waiting."
tags: [scheduler, cortex-m23, proof, interrupt, async, wait, race]
generated:
  by: openai/gpt-5.6-sol
  at: 2026-10-27T20:08:00Z
status: draft
---

# Statement

For an asynchronous task, a wake request is idempotent and the scheduler does not promise event multiplicity. Nevertheless, an event that makes work observable during the task's transition into `WAITING` must not leave the task asleep while work remains pending.

# Semantics

`wake(task)` means "this task has work or may have work."

* If the task is `WAITING`, wake changes it to `READY`.
* If the task is already `READY` or `RUNNING`, another wake does not create another job instance.
* The scheduler does not count, queue, or preserve repeated wake requests.
* A driver or application that requires event multiplicity owns its own counter, FIFO, ring buffer, DMA state, or other persistent event representation.

# Hazard

The naive sequence

```text
task checks device: no work
interrupt occurs and calls wake(task)
wake observes task RUNNING and does nothing
task calls wait()
task is now WAITING although work exists
```

loses the scheduling indication.

# Proof obligation

The implementation SHALL make the transition from "I have drained observable work" to `WAITING` atomic with respect to `wake_from_isr()`, or SHALL provide an equivalent handshake that remembers a wake arriving in that interval.

One sufficient abstract protocol is:

1. enter a bounded scheduler critical section;
2. publish the task as intending to wait;
3. re-evaluate a scheduler-visible wake latch or driver-visible work predicate;
4. if a wake raced, remain or become `READY`;
5. otherwise commit `RUNNING -> WAITING`;
6. leave the critical section.

Another implementation is acceptable only if it proves the same property.

# Proof

Let `W` mean that at least one wake occurs after the task's final successful drain check and before the wait transition is committed.

The wait protocol linearizes either before or after that wake:

* If wait linearizes first, the later wake observes `WAITING` and performs `WAITING -> READY`.
* If wake linearizes first, the wait protocol observes the wake handshake/latch or re-check and does not leave the task blocked.

Thus there is no execution ordering in which `W` occurs and the final state remains `WAITING` solely because the task crossed the wait boundary.

Repeated wakes may collapse into one scheduling indication, which is intentional.

# Architecture test

A deterministic host test SHALL exercise at least these interleavings:

1. wake before wait begins;
2. wake inside the wait critical section/handshake;
3. wake immediately after wait commits;
4. multiple wakes while READY;
5. multiple wakes while RUNNING;
6. wake plus preemption; and
7. wake from ISR immediately before exception return.

For cases 1 through 3, the task SHALL eventually be READY or RUNNING when application-visible work remains.

# Dependencies

Depends on the state model in [P1](P1-task-state-invariants.md).

The concrete architecture must define the atomic primitive or handshake before this proof can be marked stable.
