---
type: ImplementationCheckpoint
title: "R4 freeze - Minimum-footprint production specialization"
description: "Frozen merge checkpoint for accepted R4 footprint reductions and supported build surfaces."
tags: [implementation, footprint, cortex-m23, production, freeze]
status: accepted
---

# Frozen accepted result

Default task capacity:

```text
24 tasks
```

Complete physical ArghtOS resident footprint:

```text
R3c physical reference  3080 B
R4y frozen minimum      2294 B
-----------------------------
saved                    786 B
soft target             1536 B
remaining gap            758 B
```

R4y consists of:

* runtime profiling optional only in the minimum-size production profile;
* 24-task capacity;
* trusted/static production validation specialization;
* size-first scheduler selector;
* removal of the no-profile dispatch clock read;
* removal of the unused start epoch;
* split task metadata;
* entry/next-time lifetime overlay;
* full LTO;
* `-fomit-frame-pointer`;
* `-fno-jump-tables`;
* minimum linker script without unused ARM unwind index;
* compact Cortex-M23 PendSV save/restore;
* direct SVC branch to saved EXC_RETURN.

Measured R4y:

```text
linked before MSP  2214 B
ArghtOS-only MSP      80 B
-------------------------
total               2294 B
```

# Supported build surfaces after freeze

The main Makefile intentionally keeps only:

* generic host tests;
* accepted size-first selector host regression;
* profiling-enabled R3c host/reference profile;
* accepted minimum-source R4u host profile;
* ASan/UBSan host tests;
* generic Cortex-M23 compile;
* generic footprint harness;
* R3c physical footprint reference;
* final R4y minimum footprint;
* footprint reporting and cleanup.

Rejected R4 experiments remain documented under `implementation/` but are no
longer exposed as supported Makefile/CI profiles.

# Preserved semantics

R4y preserves:

* synchronous periodic RMS scheduling;
* periodic-overrun arithmetic catch-up;
* async residual-capacity scheduling and age credit;
* pending-wake race closure;
* required 64-byte crash record;
* target-independent portable kernel boundary;
* 32-byte software R4-R11 task context;
* first-dispatch MSP reclamation;
* PRIMASK serialization and race-free idle protocol;
* no heap / statically bounded resident state and work.

# Verification authority

Final merge CI must pass:

* portable-source boundary;
* generic host kernel tests;
* accepted selector tests;
* R3c profiling reference;
* R4u accepted minimum-source host tests;
* ASan/UBSan;
* generic Cortex-M23 compile;
* R3c footprint link;
* R4y footprint link;
* R3c/R4y symbol and map evidence.

The linker map remains authoritative for complete resident footprint.
