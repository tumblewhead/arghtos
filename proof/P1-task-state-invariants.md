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


# R5y trusted periodic release-state refinement

For the trusted generated minimum-size profile, the periodic release engine may
omit the defensive impossible-state branch after the following P1 facts are
combined:

* periodic tasks never enter WAITING or SLEEPING;
* a configured periodic task is DORMANT before/after jobs, READY while pending,
  or RUNNING while executing;
* fatal transition to FAULTED is terminal and normal scheduler execution does
  not resume afterward.

Therefore, on a normal call to the periodic release engine, a periodic task
whose release becomes due is either:

```text
DORMANT  -> READY, current release is not missed
READY    -> remains active, current release is missed
RUNNING  -> remains active, current release is missed
```

The generated R5y profile may encode this as:

```text
if state == DORMANT:
    state = READY
    changed = true
    missed = releases - 1
else:
    missed = releases
```

without a runtime READY/RUNNING membership check.

This refinement is valid only for the trusted generated profile that already
adopts the R5n proof-discharged scheduler invariants. Generic/debug profiles
retain the explicit impossible-state fault check.

# R6e serialized SVC transaction refinement

For the trusted R6e Cortex-M23 profile, C4 guarantees that SVC, scheduler timer,
and scheduler-mutating device handlers form one non-preempting exception class.

Consequently the lifecycle mutations performed by
`arghtos_kernel_service()` remain serialized with every competing
scheduler-state mutation even when the local PRIMASK enter/exit pair is
elided. No additional transition becomes reachable, and the existing P1 task
state invariants are unchanged.

# R6f closed service-domain refinement

Under C4/C1's trusted generated service-domain preconditions, every reachable
call to `arghtos_kernel_service()` carries one of the five defined private
service IDs. The removed BAD_SERVICE default branch is therefore unreachable
on every execution admitted by the generated profile.

All reachable state transitions remain exactly those already covered by P1.
The generic/debug profile continues to retain the BAD_SERVICE fatal path for
invalid service-ID injection.

# R6g fatal current-task domain refinement

R5n already establishes that, on supported generated-profile execution,
`kernel.current_task` is either INVALID or names a configured task selected by
the dispatcher.

R6g reuses that invariant in fatal capture. Replacing the repeated range test
with an INVALID test does not create a new task-state transition and cannot
attribute a supported fatal event to an unconfigured task. Generic/debug builds
continue to validate the full ID range at runtime.

# R6j non-faulted wake-domain refinement

R5ac removes the FAULTED state mutation only in a profile where every fatal
path is terminal. Normal scheduler execution therefore cannot resume after a
fault that would otherwise create RUNNING -> FAULTED.

Starting from pristine generated DORMANT state, the remaining nonfatal P1
transition relation closes over:

```text
DORMANT, READY, RUNNING, WAITING, SLEEPING
```

for any task that can be passed to wake.

Thus, on every execution admitted by the trusted minimum profile,
`wake_locked_compact()` cannot observe FAULTED or an unknown state encoding.
Eliding those two diagnostic branches does not change any reachable transition,
wake result, pending-wake effect, or dispatch decision. Generic/debug builds
retain both diagnostics.

# R6k preseeded periodic READY refinement

For the trusted generated R6k profile, a periodic task's physical packed state
may contain READY before STARTED, but R5n/R6b and the production API
preconditions make that state unobservable to scheduler execution until
`arghtos_start()` commits STARTED.

The logical transition remains the synchronous release-zero transition:

```text
pre-start inactive -> release at common start epoch -> READY
```

R6k moves only the READY bit materialization from runtime start code to the
generated initial image. `next_time` is still formed from the runtime start
epoch, and no task can dispatch before STARTED.

Therefore the first dispatch sees the same set of READY periodic tasks and all
subsequent P1 transitions are unchanged.

# R6n dispatch-only service-outcome refinement

For the trusted generated profile, the legal C1/C2 service domain contains no
reachable error-return transition:

* YIELD is issued from the current generated task;
* WAIT and SLEEP_UNTIL are issued only by the generated async worker;
* WAKE names the generated async worker and R6j excludes FAULTED/unknown state;
* TASK_RETURN is issued only by the generated periodic trampoline;
* R6f excludes unknown private service IDs;
* invalid relative/absolute timing inputs are outside the minimum production
  contract.

Thus the public result for every admitted service execution is exactly
`ARGHTOS_OK`. Replacing the encoded result-plus-dispatch byte with a
dispatch-only internal value preserves every reachable P1 state transition and
every public result.

# R6o current-task seed refinement

R6o moves the initial INVALID value of `kernel.current_task` from a runtime
store to the generated image.

No task-state transition depends on the physical time of that write.
INITIALIZED remains clear until successful port initialization, and dispatch is
already proved unreachable before STARTED in the trusted profile.

Therefore the dispatcher begins from the same INVALID current-task state and
all P1 transitions are unchanged.

# R6r flagless kernel-state refinement

R6r removes no state that participates in the admitted task-state transition
relation.

For the trusted generated profile:

* initialization success is a generated/selected-port invariant;
* dispatch and timer entry-before-start are already excluded by R5n/R6b;
* public lifecycle misuse checks are outside the minimal-production contract;
* recursive fatal detection is carried by the C5 commit word under R6m.

Therefore C9 INITIALIZED/STARTED/FAULT_ACTIVE bits have no remaining observer on
supported execution. Removing their storage and the final STARTED write cannot
enable or suppress any P1 transition.

The only surviving minimum C9 runtime value is `current_task`, whose
dispatcher ownership is unchanged.

# R6v redundant unit-tick dispatch refinement

Consider a unit-timer interrupt for which the time engine performs no
DORMANT/SLEEPING -> READY transition.

If no independent scheduling indication is pending, the currently RUNNING task
was the highest-priority eligible winner at the preceding dispatch boundary.
PendSV saves it, applies the ordinary legal RUNNING -> READY preemption
transition, and re-runs the unchanged selector over an unchanged eligibility
set. That task remains the winner and returns READY -> RUNNING with the same
saved continuation.

If another scheduling indication is pending, a dispatch was already required
and R6v only collapses another request into the same PendSV mechanism.

Because the selected minimum profile has runtime profiling disabled and elides
async age-reset side effects, this redundant preemption adds no persistent
scheduler state change beyond ordinary context save/restore.

Therefore unconditional timer-side dispatch preserves P1 state invariants and
observable scheduler selection.

# R6w early periodic preemption refinement

For each due periodic task, let `s` be its state before R6w normalization.

R5y limits `s` to DORMANT, READY, or RUNNING. Missed-release accounting is
computed from `s` before mutation:

```text
missed = releases - (s == DORMANT ? 1 : 0)
```

R6w then writes READY.

* DORMANT -> READY is the ordinary release transition.
* READY -> READY is idempotent.
* RUNNING -> READY is the ordinary preemption transition moved earlier into the
  timer handler.

R6v guarantees a PendSV request on that timer exit. The interrupted RUNNING
task's continuation is still saved by PendSV, and the unchanged selector then
performs READY -> RUNNING for the winning task.

Thus R6w does not add a new state edge. It only moves an already-legal
preemption edge earlier within one handler-to-PendSV scheduling transaction.

# R6ab singleton wake-target refinement

R5p partitions generated IDs into a periodic prefix followed by the async
suffix. R5ab proves that suffix has cardinality one.

Hence the async set is:

```text
A = { ARGHTOS_R5P_PERIODIC_COUNT }
```

C1/C2 permit wake only for an asynchronous task in the trusted generated
profile. Therefore every admitted wake target is the unique member of `A`.

Replacing a runtime wake ID with that compile-time ID selects the same control
record for every supported execution. No P1 state transition or wake result is
changed.

# R6ae deferred YIELD transition refinement

R6ae moves one already-legal P1 edge:

```text
RUNNING -> READY
```

from the YIELD service body to the immediately following PendSV dispatcher.

C4 proves no Thread-mode application instruction can execute in the interval.
When PendSV saves the outgoing context, `arghtos_kernel_dispatch()` observes
the current task still RUNNING and performs the same RUNNING -> READY transition
before calling the selector.

Thus the selector sees the same READY set as the ordinary implementation and
the public YIELD call can resume only after a READY -> RUNNING selection.

No P1 edge is added or removed; only the owner of the existing preemption edge
changes within one SVC-to-PendSV transaction.

# R6af crash task-ID materialization refinement

R5n establishes the generated-profile current-task domain:

```text
current_task in { INVALID } union configured_task_ids
```

R6af always copies that value to the crash record.

For INVALID, the resulting record remains unattributed because TASK_VALID is
clear. For a configured ID, R6af sets TASK_VALID and records exactly the same ID
as the ordinary conditional path.

No scheduler state is mutated and no P1 transition changes. The refinement
only changes the branch structure used to serialize already-proved current-task
state into C5 diagnostics.

# R6ah default-dispatch service proof

Under R6n every admitted generated service returns \`ARGHTOS_OK\`. Under the
accepted R6ad/R6ae/R6ag refinements, dispatch behavior partitions the legal
service outcomes as follows:

~~~text
YIELD                         -> dispatch
WAIT, wake_pending = 0        -> WAITING, dispatch
WAIT, wake_pending = 1        -> clear latch, no dispatch
SLEEP_UNTIL, future deadline  -> SLEEPING, dispatch
SLEEP_UNTIL, due deadline     -> no state change, no dispatch
WAKE                          -> dispatch
TASK_RETURN                   -> DORMANT, dispatch
~~~

R6ah changes the implementation from:

~~~text
dispatch = false
...
set dispatch = true on dispatching outcomes
...
if dispatch: request_dispatch
return OK
~~~

to:

~~~text
return OK immediately on the two no-dispatch outcomes
...
request_dispatch
return OK
~~~

Each row of the partition therefore has identical state/latch effects,
identical dispatch-request cardinality, and the same public result.

R6e serialization guarantees the early returns do not bypass a required local
critical-section exit. No P1 state transition is added, removed, or reordered
relative to an application-visible boundary.
