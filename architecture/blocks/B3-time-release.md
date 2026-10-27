---
type: CodeBlock
title: "B3 - Time and release"
description: "Tickless absolute-time release engine for periodic tasks and sleeping tasks."
tags: [architecture, code-block, time, release, tickless]
status: draft
---

# Key goals

* Implement wrap-safe absolute-time comparison.
* Maintain each timed task's next release.
* Determine and arm the earliest future scheduler event.
* Release all tasks due at an observed timestamp.
* Detect missed periodic releases instead of creating additional task instances.

```mermaid
flowchart TD
  TMR["timer interrupt"] --> NOW["read now"]
  NOW --> SCAN["bounded scan timed tasks"]
  SCAN --> DUE{"task due?"}
  DUE -- yes --> REL["READY or missed-release record"]
  DUE -- no --> NEXT["track earliest future release"]
  REL --> NEXT
  NEXT --> ARM["arm one-shot timer"]
  REL --> RES["request reschedule if needed"]
```

# Provides

Kernel-side implementation of [C3](../contracts/C3-time-timer.md).

# Requires

Platform timer functions, B0 descriptors, and B1 state.

# Dependencies

[P4](../../proof/P4-time-release-correctness.md), [P6](../../proof/P6-rms-schedulability.md), [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Wraparound bugs.
* Release drift caused by using the wrong epoch for the next period.
* Hardware timer horizon mismatch.
* Release-processing cost omitted from timing analysis.
* Creating multiple active instances of one periodic task.

# Boundary specification

B3 decides timed eligibility and next timer programming. It does not choose which READY task runs.
