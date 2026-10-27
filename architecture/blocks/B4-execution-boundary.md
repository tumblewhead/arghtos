---
type: CodeBlock
title: "B4 - Execution boundary"
status: draft
---

# Key goals

* Keep architecture-specific execution-state handling isolated.
* Keep the policy core independent of processor details.
* Preserve MSP ownership for kernel-side execution and PSP ownership for task-side execution.

# Provides

[C4 context-switch port](../contracts/C4-context-switch-port.md).

# Requires

B1 runtime state and B2 task selection.

# Dependencies

[P2](../../proof/P2-context-stack-integrity.md) and [P7](../../proof/P7-bounded-memory-and-work.md).

# Dangerous aspects

* Stack ownership becoming ambiguous.
* Save/restore behavior differing between first launch and later resumes.
* Processor-specific details leaking into scheduling policy.

# Boundary specification

B4 owns the processor-facing execution boundary. It does not decide task priority or release timing.
