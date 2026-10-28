---
type: ImplementationCheckpoint
title: "R4y - Direct EXC_RETURN branch in SVC handler"
description: "Accepted Cortex-M23 assembly micro-optimization branching directly to the saved EXC_RETURN value from R3."
tags: [implementation, footprint, cortex-m23, svc, assembly, accepted]
status: accepted
---

# Baseline

```text
R4v = 2298 B
```

# Experiment

After `arghtos_kernel_service()`, SVC restores the saved EXC_RETURN value into
R3.

R4v used:

```asm
mov lr, r3
bx  lr
```

R4y uses:

```asm
bx  r3
```

The Cortex-M exception-return mechanism recognizes an EXC_RETURN value used as
the target of the branch; it does not require the value to be copied back into
LR first.

# Verification

The experiment was initially staged under the temporary R4x target name before
the repository handoff's reserved R4x label was noticed. The measured artifact
is therefore named `r4x`, but this checkpoint canonically records the result as
R4y.

CI run:

```text
40666779700  success
```

# Measured result

```text
R4v  2298 B
R4y  2294 B
-----------
saved    4 B
```

Symbol change:

```text
SVC_Handler 22 B -> 20 B
```

The assembly bucket shrinks 2 B; linker/alignment packing yields a 4 B physical
resident reduction.

# Decision

Accept R4y for the minimum-size Cortex-M23 profile.

```text
linked before MSP  2214 B
MSP                  80 B
-------------------------
R4y total          2294 B
soft target        1536 B
remaining gap       758 B
```
