---
type: ImplementationCheckpoint
title: "R4p - Remove unused ARM unwind index"
description: "Accepted minimum-size linker specialization discarding .ARM.exidx/.ARM.extab in the bare-metal production footprint."
tags: [implementation, footprint, linker, unwind, accepted]
status: accepted
---

# Baseline

```text
R4n = 2450 B
```

# Experiment

Use a dedicated minimum-size linker script that discards:

```text
.ARM.exidx*
.ARM.extab*
```

The generic/reference linker script remains unchanged.

The Cortex-M23 production image is linked `-nostdlib`, compiler unwind tables
are disabled, ArghtOS has no runtime unwinder, and crash capture uses the fixed
C5 crash record rather than stack unwinding.

# Measured result

CI run 40663752503 passed.

```text
R4n  2450 B
R4p  2434 B
-----------
saved   16 B
```

The complete delta is linker metadata:

```text
.ARM.exidx  16 B -> 0 B
```

Code, task slots, crash storage, and the audited 80 B static MSP lower bound are
unchanged.

# Decision

Accept the no-unwind-index linker script for the minimum-size Cortex-M23
production profile only.

The reference footprint linker remains available for ordinary/debug builds.
