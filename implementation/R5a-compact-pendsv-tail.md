---
type: ImplementationCheckpoint
title: "R5a - Compact PendSV alignment decode and return tail"
description: "Accepted Cortex-M23 assembly micro-optimization for first-dispatch alignment decode and direct PendSV exception return."
tags: [implementation, footprint, cortex-m23, pendsv, assembly, accepted]
status: accepted
---

# Baseline

```text
R4y = 2294 B
```

# Experiment

Two independent Thumb-1 encodings are combined in the R5a PendSV-only path.

First-dispatch xPSR alignment decode:

```asm
ldr  r1, [r5, #28]
lsrs r1, r1, #7
movs r2, #4
ands r1, r2
```

This maps stacked xPSR bit 9 to exactly 0 or 4, matching the prior
shift/mask/shift sequence.

PendSV exception return:

```asm
bx r2
```

instead of copying the synthesized EXC_RETURN value to LR first.

# Verification

Authoritative CI:

```text
40670407899  success
```

All frozen R4 host, sanitizer, portability, and Cortex-M23 gates passed.

# Measured result

```text
R4y  2294 B
R5a  2290 B
-----------
saved    4 B
```

Physical attribution:

```text
context assembly 160 -> 156 B
PendSV_Handler   108 -> 104 B
```

All measured C symbols are unchanged.

# Context / MSP audit

R5a does not change:

* the 32-byte R4-R11 software frame;
* context-token address;
* PSP advance on restore;
* first-dispatch MSP frame amount;
* xPSR alignment semantics;
* PRIMASK handling;
* idle protocol;
* C call depth or automatic stack use.

The accepted ArghtOS-only MSP bound remains 80 B.

# Decision

Accept R5a.

```text
linked before MSP  2210 B
MSP                  80 B
-------------------------
R5a total          2290 B
remaining gap       754 B
```
