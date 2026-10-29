---
type: ImplementationCheckpoint
title: "R5l - Generated prebuilt initial task contexts"
description: "Accepted static-profile specialization moving initial Cortex-M23 task-frame construction from runtime into the generated application stack image."
tags: [implementation, footprint, cortex-m23, static-config, context, accepted]
status: accepted
---

# Baseline

```text
R5j = 1906 B
```

# Experiment

For the trusted generated minimum-size profile, each application-owned task
stack contains its exact initial 64-byte saved Cortex-M23 context before
`arghtos_init()` runs.

The generated frame is byte-for-byte equivalent to the prior runtime builder:

```text
software R4-R11: 8 zero words
hardware R0:     application entry pointer
R1-R3,R12:       zero
LR:              arghtos_task_return_guard
PC:              arghtos_task_start_trampoline, Thumb bit cleared
xPSR:            0x01000000
```

The unified task slot therefore starts with its context token already pointing
at the saved R4 word. `arghtos_init()` preserves that token and resets only
mutable runtime state.

# Target generation

R5l adds:

* `footprint/m23_prebuilt_stacks.S` for the generated application stack image;
* `footprint/m23_prebuilt_context.ld` where `.app_stacks` is load-initialized
  instead of `NOLOAD`.

Task-stack SRAM remains application-owned and unchanged in size. The trade is
additional load-image/startup initialization bytes for those application stack
frames.

# Verification

Authoritative CI:

```text
40721286853  success
```

Host tests simulate already-built context tokens and preserve the existing
scheduler semantics. Cortex-M23 target linking verifies the exact generated
frame relocations.

# Measured result

```text
R5j  1906 B
R5l  1822 B
-----------
saved   84 B
```

```text
linked before MSP  1750 B
MSP                  72 B
-------------------------
R5l total          1822 B
soft target        1536 B
remaining gap       286 B
```

Main linked symbol change:

```text
arghtos_init  156 -> 72 B
```

The runtime context-builder code is no longer retained by the minimum image.

# Context / MSP audit

R5l does not change:

* the saved task-frame ABI;
* PSP/MSP ownership;
* context token meaning;
* PRIMASK or idle behavior;
* Handler-mode C call depth.

The accepted 72 B ArghtOS-only MSP lower bound remains valid.

# Decision

Accept R5l for the generated static minimum-size profile.

Promotion requires documenting that generated static configurations may supply
prebuilt target contexts and that this trades load-image/startup initialization
for smaller resident scheduler code.
