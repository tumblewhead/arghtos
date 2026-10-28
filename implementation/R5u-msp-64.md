---
type: ImplementationCheckpoint
title: "R5u - Requalify ArghtOS-only MSP to 64 bytes"
description: "Accepted static MSP re-audit after the R5t time-engine frame reduction."
tags: [implementation, footprint, cortex-m23, msp, accepted]
status: accepted
---

# Baseline

```text
R5t linked before MSP  1602 B
R5t MSP                  72 B
R5t total              1674 B
```

# Re-audit

R5u changes no linked code.

The R5t linked disassembly gives the deepest normal ArghtOS-owned service
chain:

```text
SVC explicit push                  8 B
arghtos_kernel_service frame      16 B
process_time_locked frame         40 B
--------------------------------------
worst normal service chain        64 B
```

Other normal ArghtOS-owned paths remain below this depth.

# Verification

Authoritative CI:

```text
40731326271  success
```

The R5u footprint target reuses the exact R5t map and changes only the
ArghtOS-only MSP input to the footprint reporter.

# Measured result

```text
R5t  1674 B
R5u  1666 B
-----------
saved    8 B
```

```text
linked before MSP  1602 B
MSP                  64 B
-------------------------
R5u total          1666 B
soft target        1536 B
remaining gap       130 B
```

# Decision

Accept the 64 B static ArghtOS-only MSP lower bound for the R5t/R5u minimum
image. Full system interrupt-nesting reserve remains separate integration
evidence.
