---
type: Proof
title: "P1 - Task-state invariants"
description: "Legal task transitions preserve single-running-task behavior and prevent blocked tasks from executing."
tags: [scheduler, cortex-m23, proof, state-machine]
generated:
  by: openai/gpt-5.6-sol
  at: 2026-10-27T20:08:00Z
status: draft
---

# Statement

At every scheduler decision boundary:

1. each configured task is in exactly one valid scheduler state;
2. at most one task is `RUNNING`;
3. only a `READY` task may become `RUNNING`;
4. a `WAITING` task cannot execute until a wake transition occurs;
5. a `SLEEPING` task cannot execute before its release time; and
6. `yield()` preserves runnability while `wait()` and `sleep_until()` remove runnability.

# State set

The abstract task states are:

`DORMANT, READY, RUNNING, WAITING, SLEEPING, FAULTED`.

An implementation MAY encode these states differently provided the externally observable transition system is equivalent.

# Allowed transitions

| From | Operation / cause | To |
|---|---|---|
| DORMANT | activation | READY |
| READY | dispatch | RUNNING |
| RUNNING | preemption | READY |
| RUNNING | yield | READY |
| RUNNING | wait | WAITING |
| RUNNING | sleep_until | SLEEPING |
| RUNNING | completion of bounded periodic job | DORMANT |
| RUNNING | fatal task condition | FAULTED |
| WAITING | wake | READY |
| SLEEPING | release time reached | READY |

No other transition is valid unless added to this proof before implementation.

# Proof

Represent each task state as one member of the finite state set above.

The dispatcher has exactly one state-changing authority for selecting execution:

`READY -> RUNNING`.

Before performing that transition, the currently running task, if any, must first leave `RUNNING` by preemption, yield, blocking, completion, or fault. Therefore a scheduler decision cannot introduce a second running task.

Because neither `WAITING` nor `SLEEPING` has a direct transition to `RUNNING`, a blocked task must first become `READY`. A sleeping task can become `READY` only when the time-release predicate is true.

Therefore all six invariants are inductive over the allowed transition relation.

# Architecture test

A host-buildable scheduler model SHALL enumerate or fuzz every public transition and assert:

* `running_count <= 1`;
* dispatch target state was `READY`;
* no `WAITING -> RUNNING` edge exists;
* no `SLEEPING -> RUNNING` edge exists;
* a sleep release occurs only when the wrap-safe due predicate is true; and
* an invalid transition produces a deterministic scheduler fault rather than silently mutating state.

# Dependencies

Depends on [P0](P0-static-configuration-validity.md).

[P3](P3-wake-wait-race-freedom.md) proves the concurrency condition required for the `WAITING -> READY` edge.

[P4](P4-time-release-correctness.md) proves the release predicate required for the `SLEEPING -> READY` edge.

# R5n trusted generated-profile diagnostic refinement

The generic/debug profile retains runtime assertions that detect impossible
dispatcher state combinations and converts them to deterministic scheduler
faults.

The trusted generated minimum-size profile may omit a runtime assertion only
when the asserted condition is an inductive consequence of this proof plus the
C0/C4 trusted-profile preconditions.

For R5n, the discharged conditions are:

* dispatch is entered only after STARTED is committed;
* a nonzero outgoing task context implies `current_task` names the previously
  selected task;
* a zero outgoing context occurs only for first dispatch or the no-winner idle
  loop, where `current_task` is INVALID;
* `arghtos_select_next()` returns either INVALID or a READY configured task;
* every configured task has a valid nonzero saved context by generated
  configuration evidence;
* the private task trampoline begins only from a dispatcher-selected configured
  task and receives the generated non-NULL entry pointer;
* the specialized periodic TASK_RETURN service returns OK or does not return
  because a fatal path was entered.

Omitting these checks does not add legal state transitions and therefore does
not change the P1 transition system for supported execution.

The minimum profile no longer claims runtime diagnostic detection for these
proof-impossible combinations if a trusted premise or arbitrary memory
integrity is violated. Generic/debug builds retain that diagnostic coverage.
