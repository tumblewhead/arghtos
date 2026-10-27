---
type: Proof
title: "P5 - Priority-domain isolation"
description: "Class-first selection prevents asynchronous aging from outranking any READY periodic task."
tags: [scheduler, cortex-m23, proof, rms, priority, aging]
status: draft
---

# Statement

No asynchronous task can be selected while any periodic task is READY, regardless of async base priority or age credit.

Therefore async aging cannot change periodic RMS ordering or invalidate the periodic schedulability model.

# Concrete selector structure

C6 performs selection in this order:

```text
if any periodic READY task exists:
    return best periodic by RMS
else:
    age/compare READY async tasks
    return best async
```

Async numeric priority is never compared with a periodic priority.

# Proof

Assume at least one periodic task is READY.

The selector's periodic scan finds at least one periodic candidate.

By contract, the selector returns the best periodic candidate before entering the asynchronous selection stage.

Therefore no async task is considered as a competing winner in that dispatch decision.

Async age credit can affect only the ordering among async tasks in decisions for which the periodic READY set is empty.

Periodic ordering itself depends only on immutable period and deterministic task-ID tie break.

Thus:

1. async aging cannot reorder periodic tasks;
2. async aging cannot cause async execution ahead of READY periodic work;
3. the periodic higher-priority interference set in P6 contains only periodic jobs plus explicit kernel/interrupt overhead.

# Starvation scope

P5 makes no claim that async work receives processor time when periodic demand consumes all available capacity.

C6's starvation-avoidance property is conditional on continuing async dispatch opportunities.

# Architecture test

Exhaustively or property-test:

* every combination of periodic READY/non-READY state;
* every representable async base priority;
* every representable async age credit.

Invariant:

```text
periodic_ready_count > 0
    => selected_task.class == PERIODIC
```

Also assert that changing any async priority/age value cannot change the relative ordering of two periodic candidates.

# Dependencies

[P6](P6-rms-schedulability.md) relies on this structural isolation property.

# Status

The architectural proof is discharged by the class-first selector contract.

Implementation discharge requires tests showing the concrete selector preserves that control flow.
