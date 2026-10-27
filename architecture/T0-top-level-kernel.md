---
type: TopLevelModule
title: "T0 - ArghtOS top-level kernel module"
description: "Integration point for all ArghtOS code blocks, internal contracts, and scheduler execution paths."
tags: [architecture, top-level, kernel, integration]
status: draft
---

# Key goals

* Present one coherent scheduler module to the application.
* Integrate all code blocks without circular ownership.
* Keep policy, mechanism, time, and diagnostics separable.
* Ensure every external dependency crosses a named contract.
* Preserve direct traceability from implementation modules to proofs.

# Integrated topology

```mermaid
flowchart TB
  subgraph ARGHTOS["ArghtOS top-level module"]
    B0["B0 Config"]
    B1["B1 Runtime state"]
    B2["B2 Policy"]
    B3["B3 Time/release"]
    B4["B4 Execution boundary"]
    B5["B5 Diagnostics"]
    B6["B6 API gateway"]

    B0 --> B1
    B0 --> B2
    B3 --> B1
    B6 --> B1
    B1 <--> B2
    B2 --> B4
    B4 --> B1
    B1 --> B5
    B4 --> B5
  end
```

# Start path

```mermaid
sequenceDiagram
  participant A as Application startup
  participant K as ArghtOS
  participant T as Timer port
  participant B4 as Execution boundary
  A->>K: arghtos_init(config)
  K->>K: validate + build dormant frames
  K->>T: leave timer disarmed
  A->>K: arghtos_start()
  K->>K: capture common start epoch
  K->>K: all periodic DORMANT -> READY
  K->>K: next_time = start + period
  K->>T: arm earliest next release
  K->>B4: port_start()
  B4-->>K: first PendSV dispatch
```

# Periodic release path


```mermaid
sequenceDiagram
  participant TM as Timer
  participant B3 as Time/release
  participant B1 as State
  participant B2 as Policy
  participant B4 as Execution boundary
  TM->>B3: deadline event
  B3->>B1: due task -> READY
  B3->>B4: request reschedule
  B4->>B2: select next
  B2-->>B4: periodic task ID
  B4-->>B1: update running ownership
```

# Asynchronous wake path

```mermaid
sequenceDiagram
  participant H as Device interrupt handler
  participant B6 as Interrupt-safe API
  participant B1 as State
  participant B4 as Execution boundary
  participant B2 as Policy
  H->>B6: wake(task)
  B6->>B1: idempotent readiness update
  B6->>B4: request reschedule if needed
  B4->>B2: select next
  B2-->>B4: best eligible task
```

# Task wait/yield/sleep path

```mermaid
sequenceDiagram
  participant T as Task / PSP
  participant B6 as API gateway
  participant B1 as State
  participant B3 as Time
  participant B4 as Execution boundary
  T->>B6: wait / yield / sleep
  B6->>B1: validated transition on MSP
  opt sleep
    B6->>B3: register absolute release
  end
  B6->>B4: request reschedule
```

# Dependencies

All contracts C0-C10 and proofs P0-P8.

# Dangerous aspects

* Shared-state ownership becoming ambiguous between B1, B3, B4, and B6.
* Context mechanics leaking into policy.
* Timer code selecting tasks directly.
* Diagnostics growing into a logging subsystem.
* A second public API bypassing B6.
* Immutable configuration copied into mutable RAM unnecessarily.

# Boundary specification

T0 owns the scheduler as a subsystem. Its only external surfaces are static task configuration, the public task API, interrupt-safe wake, platform time/timer services, Cortex-M23 execution hooks, application task entry points/stacks, and the fixed crash-record/halt-reset hooks.

Everything else is private implementation.
