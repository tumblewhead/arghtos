---
type: ImplementationCheckpoint
title: "R5c - Compact wake result"
description: "Rejected neutral experiment encoding wake dispatch and public result in one internal byte."
tags: [implementation, footprint, wake, service, rejected]
status: rejected
---

# Baseline

```text
R5b = 2286 B
```

# Experiment

For the minimum specialized profile, replace the two-output wake helper:

```text
bool dispatch
arghtos_result_t result via pointer
```

with one internal byte:

```text
bits 0..2  public result code 0..7
bit 3      request dispatch
```

All public API results and wake-state transitions remain unchanged.

# Verification

Authoritative CI:

```text
40671153477  success
```

Dedicated R5c host wake-state coverage, sanitizers, portability, and
Cortex-M23 links passed.

# Measured result

```text
R5b  2286 B
R5c  2286 B
-----------
delta    0 B
```

Symbol movement:

```text
wake_locked             92 B
wake_locked_compact     80 B   -12 B

arghtos_kernel_service 228 B
R5c service            240 B   +12 B
```

The interrupt wake wrapper remains 40 B.

# Decision

Reject R5c as a standalone specialization.

The result is useful: compact wake encoding saves 12 B locally, but decoding it
back into separate service `result` and `dispatch` variables costs exactly
12 B.

Next experiment should keep the encoded outcome through the complete service
dispatcher rather than decoding at the wake call site.
