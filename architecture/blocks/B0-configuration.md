---
type: CodeBlock
title: "B0 - Configuration"
description: "Immutable task descriptors and build-time configuration validation."
tags: [architecture, code-block, configuration]
status: draft
---

# Key goals

* Keep immutable configuration compact; read-only does not imply non-SRAM.
* Validate task IDs, stack ranges, task class, timing parameters, and priority-domain membership.
* Expose a compact indexed descriptor table.
* Make `ARGHTOS_MAX_TASKS` compile-time adjustable, default 32.

# Provides

[C0 static task configuration](../contracts/C0-static-task-configuration.md).

# Requires

Application task declarations, stack declarations, compiler/linker constants, and platform alignment rules.

```mermaid
flowchart LR
  DECL["task declarations"] --> TAB["descriptor table"]
  TAB --> CHECK["build/static checks"]
  CHECK --> VIEW["read-only configuration view"]
```

# Dependencies

[P0](../../proof/P0-static-configuration-validity.md), [P2](../../proof/P2-context-stack-integrity.md), [P6](../../proof/P6-rms-schedulability.md).

# Dangerous aspects

* Writable descriptors.
* Duplicate IDs.
* Duplicate copies of already-resident immutable data.
* Inconsistent configuration macros across translation units.
* Invalid stack alignment or overlapping stack regions.

# Boundary specification

B0 provides immutable facts. It never changes task state, allocates memory, or selects a task.
