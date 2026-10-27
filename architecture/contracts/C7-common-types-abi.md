---
type: Contract
title: "C7 - Common types and ABI"
description: "Concrete scalar types, result codes, task classes, task states, and compile-time limits shared by all ArghtOS contracts."
tags: [architecture, contract, abi, types]
status: draft
---

# Key goals

* Fix widths that affect RAM, wraparound, and ABI behavior.
* Keep all public types usable from C11 without dynamic allocation.
* Make invalid/sentinel values explicit.

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
_Static_assert(ARGHTOS_MAX_TASKS <= 254u, "task IDs reserve 0xFF as invalid");
_Static_assert(ARGHTOS_ASYNC_PRIORITY_LEVELS >= 1u, "async priority domain empty");
_Static_assert(ARGHTOS_ASYNC_PRIORITY_LEVELS <= 255u, "priority must fit uint8_t");
_Static_assert(ARGHTOS_AGING_QUANTUM_TICKS > 0u, "aging quantum must be nonzero");
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

All scheduler time arithmetic is modulo 2^32. Any user-supplied relative interval must be strictly less than `ARGHTOS_TIME_HALF_RANGE`.

# Result codes

```c
typedef enum {
    ARGHTOS_OK = 0,
    ARGHTOS_ERR_BAD_ARGUMENT,
    ARGHTOS_ERR_BAD_TASK_ID,
    ARGHTOS_ERR_BAD_TASK_CLASS,
    ARGHTOS_ERR_BAD_STATE,
    ARGHTOS_ERR_BAD_CONTEXT,
    ARGHTOS_ERR_BAD_TIME,
    ARGHTOS_ERR_BAD_CONFIG
} arghtos_result_t;
```

No API returns allocator/resource-exhaustion errors because runtime resource creation does not exist.

# Task class

```c
typedef enum {
    ARGHTOS_TASK_PERIODIC = 0,
    ARGHTOS_TASK_ASYNC    = 1
} arghtos_task_class_t;
```

# Abstract task state

```c
typedef enum {
    ARGHTOS_TASK_DORMANT = 0,
    ARGHTOS_TASK_READY,
    ARGHTOS_TASK_RUNNING,
    ARGHTOS_TASK_WAITING,
    ARGHTOS_TASK_SLEEPING,
    ARGHTOS_TASK_FAULTED
} arghtos_task_state_t;
```

The implementation may pack state into bits internally, but diagnostics and host-model tests must preserve these semantics.

# Priority convention

For asynchronous tasks, larger numeric values mean greater urgency:

```text
0 ... ARGHTOS_ASYNC_PRIORITY_LEVELS-1
low                               high
```

Periodic versus asynchronous selection is a separate class decision. No numeric async priority value can equal or exceed a periodic task because the domains are never numerically compared.

# Time predicates

The canonical wrap-safe predicates are:

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

# Call-context vocabulary

Contracts use three contexts:

* **TASK**: Thread mode executing a task on PSP.
* **HANDLER**: maskable exception/interrupt handler executing on MSP.
* **KERNEL**: ArghtOS-owned Handler-mode code executing on MSP.

NMI and fatal-fault context are not normal scheduler call contexts and may call only the fatal diagnostic path.

# Dangerous aspects

* Changing scalar widths without re-proving memory and wraparound bounds.
* Using signed absolute timestamps.
* Comparing periodic and async priority numerically.
* Allowing `ARGHTOS_MAX_TASKS=255`, which collides with the invalid ID.

# Boundary specification

C7 fixes representation visible across contracts. Individual modules may use smaller packed private representations only when conversion is lossless and externally observable semantics remain identical.
