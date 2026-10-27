---
type: Contract
title: "C7 - Common types and ABI"
description: "Concrete scalar types, fixed-width status/class/state values, compile-time limits, and wrap-safe time rules shared by all contracts."
tags: [architecture, contract, abi, types]
status: draft
---

# Key goals

* Fix widths that affect RAM, wraparound, and ABI behavior.
* Avoid implementation-defined C enum storage widths.
* Keep public types usable from C11 without dynamic allocation.
* Make sentinel and invalid values explicit.

# Required compile-time configuration

```c
#ifndef ARGHTOS_MAX_TASKS
#define ARGHTOS_MAX_TASKS 32u
#endif

#ifndef ARGHTOS_ASYNC_PRIORITY_LEVELS
#define ARGHTOS_ASYNC_PRIORITY_LEVELS 8u
#endif

#ifndef ARGHTOS_AGING_QUANTUM_TICKS
#define ARGHTOS_AGING_QUANTUM_TICKS 1000u
#endif
```

Required static constraints:

```c
_Static_assert(ARGHTOS_MAX_TASKS >= 1u, "at least one task slot required");
_Static_assert(ARGHTOS_MAX_TASKS <= 254u, "0xFF is reserved as invalid ID");

_Static_assert(ARGHTOS_ASYNC_PRIORITY_LEVELS >= 1u, "empty async domain");
_Static_assert(ARGHTOS_ASYNC_PRIORITY_LEVELS <= 255u, "priority must fit uint8_t");

_Static_assert(ARGHTOS_AGING_QUANTUM_TICKS > 0u, "aging quantum must be nonzero");
_Static_assert(
    ARGHTOS_AGING_QUANTUM_TICKS <
        (0x80000000u / ARGHTOS_ASYNC_PRIORITY_LEVELS),
    "async aging must reach its ceiling within the wrap-safe horizon");
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
typedef uint32_t arghtos_irq_key_t;

#define ARGHTOS_TASK_ID_INVALID ((arghtos_task_id_t)UINT8_MAX)
#define ARGHTOS_TIME_HALF_RANGE ((arghtos_duration_t)0x80000000u)
```

# Fixed-width result codes

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

No API returns allocator/resource-exhaustion errors because runtime resource creation does not exist.

# Fixed-width task class

```c
typedef uint8_t arghtos_task_class_t;

enum {
    ARGHTOS_TASK_PERIODIC = 0u,
    ARGHTOS_TASK_ASYNC    = 1u
};
```

# Fixed-width task state

```c
typedef uint8_t arghtos_task_state_t;

enum {
    ARGHTOS_TASK_DORMANT = 0u,
    ARGHTOS_TASK_READY   = 1u,
    ARGHTOS_TASK_RUNNING = 2u,
    ARGHTOS_TASK_WAITING = 3u,
    ARGHTOS_TASK_SLEEPING = 4u,
    ARGHTOS_TASK_FAULTED = 5u
};
```

The implementation may pack state further internally only if C5 snapshots preserve these exact values.

# Priority convention

For asynchronous tasks, larger numeric values mean greater urgency:

```text
0 ... ARGHTOS_ASYNC_PRIORITY_LEVELS-1
low                               high
```

Periodic versus asynchronous selection is a separate class decision. The two domains are never numerically compared.

# Time model

All scheduler time is modulo 2^32.

Any user-supplied relative interval must be strictly less than `ARGHTOS_TIME_HALF_RANGE`.

Canonical predicates:

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

Unsigned elapsed time for two timestamps known to lie within the valid horizon is:

```c
arghtos_duration_t elapsed = now - then;
```

# Call-context vocabulary

Contracts use three normal contexts:

* **TASK**: Thread mode executing an ArghtOS task on PSP.
* **HANDLER**: supported maskable interrupt/exception handler on MSP.
* **KERNEL**: ArghtOS-owned Handler-mode execution on MSP.

NMI and fatal-fault context may call only the fatal diagnostic path.

# Language/ABI requirements

* C11 or newer for kernel C.
* 32-bit `uintptr_t` on the Cortex-M23 target.
* No reliance on compiler short-enum options.
* Public headers must be valid in both C and C++ translation units through conventional `extern "C"` guards when compiled as C++.
* Architecture-specific assembly is isolated behind C4.

# Dangerous aspects

* Changing scalar widths without re-proving memory and time bounds.
* Using implementation-defined enum size in stored objects.
* Comparing periodic and async priority numerically.
* Allowing task ID 0xFF to become valid.
* Configuring aging so slowly that its ceiling lies beyond the wrap-safe horizon.

# Boundary specification

C7 fixes representation visible across contracts. Private packing is allowed only when conversion is exact and no observable contract changes.
