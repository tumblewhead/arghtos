---
type: IntegratedModule
title: "M3 - Diagnostics and fault path"
description: "Composition for runtime timing evidence, invariant violations, processor faults, and fixed crash records."
tags: [architecture, integrated-module, diagnostics, fault]
status: draft
---

# Key goals

* Gather runtime evidence for scheduler overhead and observed task execution.
* Capture fatal state with minimal dependencies.
* Keep fault handling separate from transport and presentation.

# Integrated code blocks and contracts

B1 runtime state + B4 processor boundary + B5 diagnostics, implementing C5.

```mermaid
flowchart TB
  DISP["dispatch/deschedule"] --> PROF["runtime accounting"]
  INV["kernel invariant violation"] --> CR["crash capture"]
  CPU["processor exception"] --> CR
  CR --> REC["fixed record"]
  REC --> POL["halt/reset hook"]
  REC -. post-reset .-> APP["application exporter"]
```

# Dependencies

[P2](../../proof/P2-context-stack-integrity.md), [P6](../../proof/P6-rms-schedulability.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Fault capture itself failing.
* Depending on corrupted PSP state.
* Export code entering the fatal path.
* Profiling overhead materially changing the quantity being measured.

# Boundary specification

M3 captures evidence and invokes a minimal terminal policy. Recovery and export beyond that point are outside the kernel.
