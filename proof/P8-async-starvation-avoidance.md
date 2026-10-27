---
type: Proof
title: "P8 - Asynchronous starvation avoidance"
description: "Opportunity-based age credit guarantees eventual selection of every continuously READY async task when async dispatch opportunities continue."
tags: [scheduler, proof, async, aging, starvation]
status: draft
---

# Statement

Assume:

1. the configured async task set is finite;
2. a task `t` remains continuously READY;
3. async dispatch opportunities continue to occur; and
4. periodic work does not permanently eliminate those opportunities.

Then `t` is eventually selected.

# Concrete aging rule

At each async dispatch opportunity:

1. every READY async task increments `age_credit` by one, saturating at `UINT8_MAX`;
2. effective async priority is `min(base + age_credit, ceiling)`;
3. ties use larger `age_credit`;
4. remaining ties use smaller task ID;
5. the selected task resets `age_credit=0` when it enters RUNNING.

# Proof sketch

Let task `t` remain continuously READY and not yet selected.

At every async opportunity, `t.age_credit` increases monotonically until saturation.

Its effective priority therefore eventually reaches the async priority ceiling.

After that point, any READY peer can outrank `t` only by having greater age credit, or equal credit and a lower task ID.

The number of configured peers is finite.

Whenever such a peer is selected, that peer's credit resets to zero.

A newly READY peer begins with zero credit.

Therefore the set of peers that can remain strictly ahead of continuously READY `t` decreases as selections occur and cannot be replenished with arbitrarily high-credit newcomers.

Consequently `t` must eventually become the highest-ranked candidate and be selected.

# Scope

This proof does not guarantee a wall-clock response bound.

If periodic demand permanently removes all async dispatch opportunities, no async scheduler can provide CPU service without violating P5.

The guarantee is therefore conditional on residual async scheduling opportunities existing.

# Architecture test

A host model SHALL test:

* all async base-priority combinations;
* tasks becoming READY at different opportunities;
* repeated READY wakes not resetting credit;
* saturation at `UINT8_MAX`;
* more tasks than async priority levels;
* a continuously READY lowest-base-priority task competing with repeatedly re-ready highest-base-priority tasks.

For every finite test population in which async opportunities continue, each continuously READY task must eventually be selected.

# Dependencies

Depends on C6 opportunity aging and P5 periodic-domain isolation.

# Status

The architectural liveness argument is discharged for the specified opportunity-based algorithm.

Implementation discharge requires model/property tests of the concrete selector.
