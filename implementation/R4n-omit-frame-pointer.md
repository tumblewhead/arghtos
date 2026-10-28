---
type: ImplementationCheckpoint
title: "R4n - Omit Cortex-M23 frame pointers"
description: "Accepted compiler-size experiment adding -fomit-frame-pointer to the LTO minimum-size Cortex-M23 profile and requalifying MSP."
tags: [implementation, footprint, compiler, frame-pointer, msp, accepted]
status: accepted
---

# Baseline

Accepted R4m:

```text
linked before MSP  2442 B
static MSP            88 B
total               2530 B
```

# Experiment

Add `-fomit-frame-pointer` to every C compilation and the LTO link in the
minimum-size Cortex-M23 profile.

The previous linked disassembly showed widespread `r7` frame-pointer setup in
non-leaf Thumb functions despite `-Oz`.

# Linked result

```text
R4m linked before MSP  2442 B
R4n linked before MSP  2370 B
-----------------------------
linked saving             72 B
```

Representative symbol shrinkage:

```text
arghtos_init              160 -> 152 B
arghtos_task_trampoline    76 ->  72 B
arghtos_start             104 ->  96 B
wake_locked               112 -> 108 B
arghtos_fatal             240 -> 236 B
process_time_locked       272 -> 256 B
arghtos_kernel_dispatch   276 -> 264 B
timer_interrupt            48 ->  44 B
```

# MSP requalification

R4n disassembly gives:

```text
SVC explicit save                  8 B
arghtos_kernel_service frame      24 B
process_time_locked frame         48 B
--------------------------------------
worst normal service chain        80 B

timer_interrupt + time engine      8 + 48 = 56 B
wake_from_interrupt + wake_locked 16 + 24 = 40 B
kernel_dispatch                   24 B
arghtos_init                      40 B
arghtos_start                      8 B
```

The static ArghtOS-only MSP lower bound therefore becomes 80 B.

# Accepted footprint

```text
linked before MSP  2370 B
MSP                  80 B
-------------------------
R4n total          2450 B
soft target        1536 B
remaining gap       914 B
```

Relative to the physically corrected R3c 3080 B profile:

```text
3080 -> 2450 B
saved = 630 B
```

# Decision

Accept `-fomit-frame-pointer` together with full LTO for the minimum-size
Cortex-M23 production profile.

As with prior MSP values, 80 B is a static lower bound pending hardware
high-water and full allowed interrupt-nesting qualification.
