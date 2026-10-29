---
type: ImplementationCheckpoint
title: "R5b - Compact EXC_RETURN stack-selection tests"
description: "Accepted Cortex-M23 Thumb-1 compaction of PendSV and HardFault EXC_RETURN bit-2 tests."
tags: [implementation, footprint, cortex-m23, pendsv, hardfault, assembly, accepted]
status: accepted
---

# Baseline

```text
R5a = 2290 B
```

# Experiment

PendSV uses EXC_RETURN bit 2 to distinguish PSP task return from the
MSP/first-dispatch path.

R5b replaces:

```asm
movs r2, #4
tst  r3, r2
beq  1f
```

with:

```asm
lsrs r3, r3, #3
bcc  1f
```

The carry output of LSR #3 is original bit 2.

HardFault similarly replaces its mask/test with:

```asm
lsrs r1, r0, #3
bcc  1f
```

while R0 retains the original EXC_RETURN value passed to the C fault bridge.

# Verification

Authoritative CI:

```text
40670635274  success
```

All host, sanitizer, portability, and Cortex-M23 gates passed.

# Measured result

```text
R5a  2290 B
R5b  2286 B
-----------
saved    4 B
```

Symbol changes:

```text
PendSV_Handler    104 -> 102 B
HardFault_Handler  28 ->  26 B
context assembly   156 -> 152 B
```

All C symbols are unchanged.

# MSP / context audit

The task frame, PSP/MSP ownership, PRIMASK behavior, first-dispatch alignment
semantics, and C call depth are unchanged. The accepted ArghtOS-only MSP bound
remains 80 B.

# Decision

Accept R5b.

```text
linked before MSP  2206 B
MSP                  80 B
-------------------------
R5b total          2286 B
remaining gap       750 B
```
