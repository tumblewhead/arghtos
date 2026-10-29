---
type: ImplementationCheckpoint
title: "R5y - Proven periodic release-state membership"
description: "Accepted generated-profile specialization removing the impossible periodic-release state fault branch."
tags: [implementation, footprint, proof, periodic, accepted]
status: accepted
---

# Baseline

```text
R5u = 1666 B
```

# Proof obligation

Under the accepted generated profile, periodic tasks enter the time engine only
in the lifecycle states reachable from synchronous periodic RMS execution.
At a due release the active prior job is therefore either:

* DORMANT, meaning the new release becomes READY; or
* READY/RUNNING, meaning the release is missed while the prior job remains
  active.

WAITING and SLEEPING are async-only states. A FAULTED task does not resume
normal scheduler mutation.

R5y removes only the generated-profile fatal branch for impossible periodic
release state membership. Generic/debug behavior is unchanged.

# Verification

Authoritative CI:

```text
40734862015  success
```

All host, sanitizer, portability, and Cortex-M23 gates passed.

# Measured result

```text
R5u  1666 B
R5y  1646 B
-----------
saved   20 B
```

Primary symbol change:

```text
process_time_locked  200 -> 180 B
```

The linked code before MSP is 1582 B. The accepted 64 B static ArghtOS-only
MSP bound is unchanged.

# Decision

Accept R5y.

```text
linked before MSP  1582 B
MSP                  64 B
-------------------------
R5y total          1646 B
soft target        1536 B
remaining gap       110 B
```
