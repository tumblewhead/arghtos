---
type: Contract
title: "C7 - Common types and ABI"
description: "Concrete scalar types, fixed-width status/class/state values, compile-time limits, and wrap-safe time rules."
tags: [architecture, contract, abi, types]
status: draft
---

# Key goals

* Fix widths that affect resident SRAM, wraparound, and ABI behavior.
* Avoid implementation-defined enum storage widths.
* Keep public types usable from C11.
* Make sentinel and invalid values explicit.

# Required compile-time configuration

```c
#ifndef ARGHTOS_MAX_TASKS
#define ARGHTOS_MAX_TASKS 24u
#endif

#ifndef ARGHTOS_ASYNC_PRIORITY_MAX
#define ARGHTOS_ASYNC_PRIORITY_MAX 8u
#endif
```

Required constraints:

```c
_Static_assert(ARGHTOS_MAX_TASKS >= 1u, "at least one task required");
_Static_assert(ARGHTOS_MAX_TASKS <= 254u, "0xFF reserved as invalid ID");

_Static_assert(ARGHTOS_ASYNC_PRIORITY_MAX >= 1u, "empty async priority domain");
_Static_assert(ARGHTOS_ASYNC_PRIORITY_MAX <= 254u,
               "0xFF is reserved as invalid async priority");
```

# Scalar ABI

```c
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t  arghtos_task_id_t;
typedef uint8_t  arghtos_priority_t;
typedef uint32_t arghtos_time_t;
typedef uint32_t arghtos_duration_t;

#define ARGHTOS_TASK_ID_INVALID ((arghtos_task_id_t)UINT8_MAX)
#define ARGHTOS_TIME_HALF_RANGE ((arghtos_duration_t)0x80000000u)
```

# Result codes

```c
typedef uint8_t arghtos_result_t;

enum {
    ARGHTOS_OK                 = 0u,
    ARGHTOS_ERR_BAD_ARGUMENT   = 1u,
    ARGHTOS_ERR_BAD_TASK_ID    = 2u,
    ARGHTOS_ERR_BAD_TASK_CLASS = 3u,
    ARGHTOS_ERR_BAD_STATE      = 4u,
    ARGHTOS_ERR_BAD_CONTEXT    = 5u,
    ARGHTOS_ERR_BAD_TIME       = 6u,
    ARGHTOS_ERR_BAD_CONFIG     = 7u
};
```

# Task class

Task class is not stored explicitly in C0.

```text
descriptor.period == 0  => asynchronous
descriptor.period != 0  => periodic
```

For APIs that need an abstract class value:

```c
typedef uint8_t arghtos_task_class_t;

enum {
    ARGHTOS_TASK_PERIODIC = 0u,
    ARGHTOS_TASK_ASYNC    = 1u
};
```

# Task state

```c
typedef uint8_t arghtos_task_state_t;

enum {
    ARGHTOS_TASK_DORMANT  = 0u,
    ARGHTOS_TASK_READY    = 1u,
    ARGHTOS_TASK_RUNNING  = 2u,
    ARGHTOS_TASK_WAITING  = 3u,
    ARGHTOS_TASK_SLEEPING = 4u,
    ARGHTOS_TASK_FAULTED  = 5u
};
```

# Priority convention

Async larger numeric priority means greater urgency.

Periodic priority is not represented by this number; it is derived from period.

Periodic versus async is a class-first scheduling decision, not a numeric comparison.

# Time model

Scheduler time is modulo 2^32.

Every relative sleep and periodic interval must be strictly less than `ARGHTOS_TIME_HALF_RANGE`.

```c
static inline bool arghtos_time_due(arghtos_time_t now, arghtos_time_t t)
{
    return (int32_t)(now - t) >= 0;
}

static inline bool arghtos_duration_valid(arghtos_duration_t dt)
{
    return dt < ARGHTOS_TIME_HALF_RANGE;
}
```

For timestamps known to be within the valid horizon:

```c
arghtos_duration_t elapsed = now - then;
```

Async aging does not use scheduler time and therefore has no wraparound dependency.

# Call-context vocabulary

* **TASK**: Thread mode executing an ArghtOS task on PSP.
* **HANDLER**: supported maskable exception/interrupt handler on MSP.
* **KERNEL**: ArghtOS-owned Handler-mode code on MSP.

NMI and fatal-fault context may call only fatal diagnostics.

# Language / ABI requirements

* C11 or newer.
* 32-bit `uintptr_t`.
* no reliance on short-enum compiler options;
* architecture-specific context and critical-section token types live in the architecture-neutral port ABI (C4/C10), not in the public application ABI;
* architecture-specific assembly isolated behind the selected port.

# Resident-memory rule

Compiler/linker section names do not change accounting.

Any ArghtOS byte resident in SRAM counts toward the scheduler-footprint budget, including code, constants, literal pools, jump tables, mutable data, no-init data, and required scheduler metadata.

# Dangerous aspects

* Treating `const` or `.text` as non-SRAM.
* Changing scalar widths without re-proving footprint and time bounds.
* Comparing async and periodic priority numerically.
* Allowing task ID 0xFF.

# Boundary specification

C7 fixes common representation and the universal resident-memory accounting rule.
