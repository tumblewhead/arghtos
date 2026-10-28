---
type: ImplementationCheckpoint
title: "R4v - Compact PendSV save/restore"
description: "Accepted Cortex-M23 assembly compaction using STMIA/LDMIA batching while preserving the existing task context ABI."
tags: [implementation, footprint, cortex-m23, assembly, context, accepted]
status: accepted
---

# Baseline

```text
R4u = 2322 B
```

# Experiment

Keep the C4 task context and dispatch protocol unchanged, but encode the
Cortex-M23 PendSV save/restore more densely.

Outgoing save:

```text
STMIA R4-R7
move R8-R11 into R4-R7
STMIA second group
```

Incoming restore:

```text
LDMIA R4-R7
LDMIA two high-register temporaries
move into R8-R9
LDMIA next pair
move into R10-R11
```

The EXC_RETURN value is synthesized as `~2` instead of loaded from a literal
pool.

# Verification

Authoritative CI:

```text
40666295535  success
```

The corrected build contains one R4u control recipe and one R4v recipe; the
R4v assembly compile alone defines `ARGHTOS_COMPACT_PENDSV=1`.

# Measured result

```text
R4u  2322 B
R4v  2298 B
-----------
saved   24 B
```

Assembly attribution:

```text
context assembly 188 B -> 162 B
PendSV_Handler   130 B -> 108 B
```

The physical total falls 24 B rather than 26 B because 2 B is recovered by
linker/alignment placement elsewhere.

# Context and stack audit

The saved task context is unchanged:

```text
R4-R11 software save  32 B
hardware frame         32 B nominal
context token points at saved R4
```

First-dispatch MSP reclamation, xPSR alignment handling, PRIMASK save/restore,
and idle behavior are unchanged.

No C stack frame changes are introduced by the assembly compaction, so the
accepted 80 B ArghtOS-only MSP lower bound remains valid.

# Decision

Accept R4v for the minimum-size Cortex-M23 production profile.

```text
linked before MSP  2218 B
MSP                  80 B
-------------------------
R4v total          2298 B
soft target        1536 B
remaining gap       762 B
```
