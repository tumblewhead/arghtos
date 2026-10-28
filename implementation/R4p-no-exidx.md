---
type: ImplementationCheckpoint
title: "R4p - Remove unused ARM unwind index"
description: "Accepted minimum-size linker specialization discarding unused .ARM.exidx/.ARM.extab metadata."
tags: [implementation, footprint, linker, unwind, accepted]
status: accepted
---

# Baseline

Accepted R4n:

```text
linked before MSP  2370 B
MSP                  80 B
total               2450 B
```

# Experiment

Use the minimum-size linker script
`footprint/m23_min_no_exidx.ld` and discard:

```text
.ARM.exidx*
.ARM.extab*
```

The minimum-size build is freestanding C plus explicit assembly and already
uses:

```text
-fno-unwind-tables
-fno-asynchronous-unwind-tables
```

ArghtOS does not use language exception unwinding. Context switching and fatal
capture are explicit scheduler/port mechanisms.

# Measured result

Authoritative CI run 40663752503 passed.

```text
R4n  linked before MSP  2370 B
R4p  linked before MSP  2354 B
                              -16 B

R4n total               2450 B
R4p total               2434 B
------------------------------
saved                      16 B
```

Footprint evidence:

```text
linker resident metadata   16 B -> 0 B
all code symbol sizes      unchanged
task slots                 480 B unchanged
crash record                64 B unchanged
context assembly           188 B unchanged
MSP                          80 B unchanged
```

# Decision

Accept the no-exidx linker script for the minimum-size Cortex-M23 production
profile.

The ordinary/reference linker script remains available for non-minimum builds.

Remaining gap:

```text
2434 - 1536 = 898 B
```
