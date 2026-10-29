---
type: ImplementationMethodology
title: "Footprint harness classes"
description: "Separates historical constant-foldable footprint evidence from the physical-board volatile/MMIO-like measurement lane."
tags: [implementation, footprint, methodology, cortex-m23]
status: active
---

# Problem discovered during R6r

The historical Cortex-M23 footprint harness modeled board time and timer state
as ordinary file-static variables in `footprint/m23_board_stub.c`.

With whole-program LTO, the compiler could prove:

* the synthetic clock never changed from zero; and
* scheduler-timer stores were not externally observable.

That allowed constant folding and dead-store elimination across the
kernel/board boundary. The resulting measurements are exactly reproducible, but
they are optimistic relative to a real board where time and timer state are
MMIO/external and can change independently.

A later change made that state `volatile`, correctly preventing the folding,
but because the board stub was shared it also changed every frozen historical
control in-place. R5ag moved from 1534 B to 1674 B even though the R5ag source
profile itself had not changed.

# Resolution

The harness now has two explicit modes.

## Historical legacy harness

```text
ARGHTOS_FOOTPRINT_LEGACY_CONSTANT_BOARD=1
```

This reproduces the original constant-foldable board model. It is used only to
reproduce already-recorded R3/R4/R5/R6b-q evidence.

CI requires the canonical R5ag result to reproduce exactly:

```text
R5ag resident = 1534 B
```

Historical values remain useful for experiment-to-experiment provenance because
each accepted comparison used the same harness, but they are not the preferred
absolute physical-board footprint claim.

## Physical-board harness

Default board-stub mode uses volatile synthetic board state placed in
`.app_data`.

This:

* prevents LTO from assuming time is permanently zero;
* prevents timer-arm/disarm writes from becoming unobservable;
* excludes the synthetic state itself from ArghtOS resident accounting, matching
  application/MMIO ownership; and
* preserves the code consequences of a real externally mutable time/timer
  boundary.

New absolute footprint claims must use this lane.

# First physical control pair

CI run 40922006295 established:

```text
R6q physical control
  linked before MSP  1440 B
  ArghtOS-only MSP      64 B
  resident total      1504 B

R6r physical challenger
  linked before MSP  1428 B
  ArghtOS-only MSP      64 B
  resident total      1492 B

delta                   -12 B
```

Both remain below the original 1536 B target.

# Rule going forward

1. Frozen historical targets keep the legacy harness solely for reproduction.
2. Every new accepted absolute footprint result uses the physical-board harness.
3. A challenger and its control must use the same harness.
4. Changing board-observability assumptions creates a new measurement class; it
   must never silently rewrite prior controls.
5. CI gates both historical reproducibility and the current physical target.
