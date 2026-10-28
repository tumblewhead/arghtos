---
type: ImplementationCheckpoint
title: "R5j - Requalify ArghtOS-only MSP to 72 bytes"
description: "Accepted static MSP re-audit of the R5i linked Cortex-M23 minimum-size image."
tags: [implementation, footprint, cortex-m23, msp, accepted]
status: accepted
---

# Baseline

```text
R5i linked before MSP  1834 B
R5i MSP                  80 B
R5i total              1914 B
```

# Re-audit

R5j does not change the linked binary.

The linked R5i disassembly gives the deepest normal ArghtOS-owned MSP chain:

```text
SVC explicit push                 8 B
arghtos_kernel_service frame     16 B
process_time_locked frame        48 B
-------------------------------------
worst normal service chain       72 B
```

Other observed ArghtOS-owned paths remain below that bound:

```text
timer_interrupt + time engine     8 + 48 = 56 B
arghtos_init                      <= 40 B
arghtos_kernel_dispatch             24 B
HardFault bridge                    16 B
wake interrupt + wake helper         16 B
arghtos_start                         8 B
```

D10 tracks this `M_MSP_arghtos` value separately from the full physical
system MSP required by allowed interrupt nesting.

# Verification

Authoritative CI:

```text
40719129049  success
```

R5j reuses the exact R5i linker map and changes only the MSP accounting input to
the footprint reporter.

# Measured result

```text
R5i  1914 B
R5j  1906 B
-----------
saved    8 B
```

```text
linked before MSP  1834 B
MSP                  72 B
-------------------------
R5j total          1906 B
soft target        1536 B
remaining gap       370 B
```

# Decision

Accept 72 B as the static ArghtOS-only MSP lower bound for the R5i/R5j
minimum-size image.

As with the earlier 80 B bound, hardware high-water and full allowed
interrupt-nesting qualification remain separate system-integration evidence.
