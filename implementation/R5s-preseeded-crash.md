---
type: ImplementationCheckpoint
title: "R5s - Preseeded generated crash record"
description: "Accepted trusted generated-profile specialization that preloads invariant crash-record fields and elides their fatal-path rewrites."
tags: [implementation, footprint, crash, generated-config, accepted]
status: accepted
---

# Baseline

```text
R5r = 1734 B
```

# Experiment

The generated minimum profile preloads crash-record fields that are invariant
for the image and need not be recomputed on fatal entry.

The required 64-byte crash record remains resident and the C5 commit protocol
remains intact:

* clear commit marker first;
* write dynamic reason/task/time/port payload;
* execute the release fence;
* write commit marker last.

The specialization is restricted to the trusted generated profile and does not
change generic/debug crash initialization.

# Verification

Authoritative CI:

```text
40730139694  success
```

All host, sanitizer, portability, and Cortex-M23 gates passed after the target
fixture was built with the same accepted generated-profile prerequisites as
the kernel.

# Measured result

```text
R5r  1734 B
R5s  1702 B
-----------
saved   32 B
```

Primary symbol change:

```text
arghtos_fatal  228 -> 196 B
```

The required crash-record allocation remains 64 B.

# Decision

Accept R5s.

```text
linked before MSP  1630 B
MSP                  72 B
-------------------------
R5s total          1702 B
soft target        1536 B
remaining gap       166 B
```
