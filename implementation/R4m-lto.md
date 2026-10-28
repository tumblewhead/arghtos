---
type: ImplementationCheckpoint
title: "R4m - Full LTO on the minimum-size Cortex-M23 profile"
description: "Accepted compiler/linker experiment applying full Clang LTO to the current minimum-size profile and requalifying static MSP from linked disassembly."
tags: [implementation, footprint, lto, cortex-m23, accepted]
status: accepted
---

# Corrected physical baseline

The footprint reporter now uses physical output-section occupancy and subtracts
only explicit application-owned `.app_*` inputs.

This exposes 8 B of linker alignment that the earlier input-symbol sum omitted.

Therefore the accepted non-LTO R4h baseline is:

```text
legacy attributed total  2770 B
physical resident total  2778 B
```

All previously measured feature/code deltas remain unchanged.

# R4m experiment

Apply full Clang `-flto` to all C translation units in the minimum-size
Cortex-M23 footprint profile. The context assembly remains a normal object.

No scheduler semantics or API requirements change.

# Physical linked result before MSP

The LTO map reports:

```text
linked ArghtOS before MSP  2442 B
```

The merged LTO object prevents meaningful kernel-vs-port object attribution, so
the authoritative total is physical output-section occupancy rather than object
bucket sums.

# LTO MSP audit

The old 104 B lower bound came from:

```text
SVC explicit save            8 B
kernel_service frame        40 B
process_time_locked frame   56 B
-------------------------------
total                       104 B
```

R4m linked disassembly shows:

```text
SVC explicit save            8 B
kernel_service frame        24 B
process_time_locked frame   56 B
-------------------------------
new normal-path bound       88 B
```

Other normal chains are smaller:

```text
timer_interrupt + process_time_locked   16 + 56 = 72 B
wake_from_interrupt + wake_locked       24 + 24 = 48 B
kernel_dispatch                         32 B
arghtos_init                            44 B
arghtos_start                           24 B
```

As before, this is a static ArghtOS-only lower bound. Hardware high-water and
full interrupt-nesting qualification remain separate target evidence.

# Accepted R4m footprint

```text
R4h physical total    2778 B
R4m linked pre-MSP    2442 B
R4m MSP                 88 B
----------------------------
R4m total             2530 B
saving                 248 B
```

Remaining gap to the 1536 B soft target:

```text
2530 - 1536 = 994 B
```

# Decision

Accept full LTO for the minimum-size Cortex-M23 production profile.

Keep the non-LTO build in CI as a source-level/reference comparison and retain
the separate LTO MSP assumption in the build.
