---
type: ImplementationCheckpoint
title: "R4q - Disable jump tables"
description: "Accepted compiler-shape optimization disabling jump tables in the minimum-size LTO Cortex-M23 profile."
tags: [implementation, footprint, compiler, jump-table, accepted]
status: accepted
---

# Baseline

```text
R4p = 2434 B
```

# Experiment

Add `-fno-jump-tables` to all C compilation and the LTO link in the accepted
minimum-size profile.

The source-level switches remain unchanged.

# Measured result

CI run 40663959695 passed.

```text
R4p  2434 B
R4q  2410 B
-----------
saved   24 B
```

The linked savings are concentrated in two switch-heavy functions:

```text
wake_locked              108 ->  96 B
arghtos_kernel_service   228 -> 216 B
```

Other measured symbols are unchanged.

# MSP audit

The limiting normal chain remains:

```text
SVC explicit save            8 B
arghtos_kernel_service      24 B
process_time_locked         48 B
-------------------------------
total                       80 B
```

The wake path itself becomes smaller on stack, but it is not the limiting chain.

# Decision

Accept `-fno-jump-tables` for the minimum-size Cortex-M23 production profile.

Current accepted total:

```text
linked before MSP  2330 B
MSP                  80 B
-------------------------
R4q total          2410 B
remaining gap       874 B
```
