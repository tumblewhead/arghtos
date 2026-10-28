---
type: Contract
title: "C10 - Header and symbol boundaries"
description: "Concrete public, platform-port, and kernel-private header split for ArghtOS."
tags: [architecture, contract, headers, api, visibility]
status: draft
---

# Key goals

* Keep the public API very small.
* Prevent application code from depending on private scheduler structures.
* Give platform integration a separate stable port surface.
* Make accidental cross-layer coupling visible at compile time.

# Required header layout

```text
include/
└── arghtos/
    ├── arghtos.h       public application API
    └── arghtos_port.h  architecture-neutral port ABI

src/
├── arghtos_internal.h      kernel-private data/contracts
└── arghtos_kernel_port.h   private kernel callbacks callable by a port
```

Equivalent paths are permitted only if the same visibility split is preserved.

# Public header: arghtos.h

The public application header contains only:

* C7 public scalar types and result codes;
* C0 task/configuration types;
* C1 task lifecycle API;
* C5 public profiling snapshot/API;
* C5 crash-record type and validity constants, because v1 requires application-provided crash storage;
* public constants required to configure tasks.

Required public function set:

```c
arghtos_result_t arghtos_init(const arghtos_config_t *config);
void arghtos_start(void);

arghtos_result_t arghtos_yield(void);
arghtos_result_t arghtos_wait(void);
arghtos_result_t arghtos_sleep_until(arghtos_time_t absolute_time);
arghtos_result_t arghtos_sleep_for(arghtos_duration_t duration);
arghtos_result_t arghtos_wake(arghtos_task_id_t task_id);

arghtos_result_t arghtos_wake_from_interrupt(arghtos_task_id_t task_id);

arghtos_result_t arghtos_profile_get(
    arghtos_task_id_t task_id,
    arghtos_task_profile_t *out);
```

`arghtos_wake_from_interrupt()` is public because device handlers must call it, but its valid execution context remains restricted by C2.

The public header must not expose:

* `arghtos_task_control_t`;
* `arghtos_kernel_state_t`;
* selector internals;
* saved-context frame layout;
* SVC numbers;
* kernel-global objects;
* private profiling mutation helpers.

# Abstract port header: arghtos_port.h

`arghtos_port.h` is architecture-neutral. It is the only platform abstraction surface that portable kernel C may include.

It must not include vendor headers or expose Cortex-specific registers, exception names, frame layouts, or interrupt-controller details.

A target port implements or binds:

```c
arghtos_result_t arghtos_port_init(void);
bool arghtos_port_stack_valid(void *stack_base, uint32_t stack_size);

typedef uintptr_t arghtos_context_t;
#define ARGHTOS_CONTEXT_NONE ((arghtos_context_t)0)

arghtos_time_t arghtos_port_time_now(void);
void arghtos_port_timer_arm(arghtos_time_t absolute_deadline);
void arghtos_port_timer_disarm(void);

arghtos_irq_key_t arghtos_port_critical_enter(void);
void arghtos_port_critical_exit(arghtos_irq_key_t key);

bool arghtos_port_is_task_context(void);
bool arghtos_port_is_handler_context(void);

arghtos_result_t arghtos_port_service_call(
    uint8_t service_id,
    uint32_t arg0);

void arghtos_port_request_dispatch(void);
void arghtos_port_start(void);
void arghtos_port_idle_wait(void);

arghtos_context_t arghtos_port_build_initial_context(
    void *stack_base,
    uint32_t stack_size,
    void (*thread_start)(void),
    void (*return_guard)(void));

void arghtos_port_crash_fill(
    arghtos_crash_record_t *record,
    const void *port_fault_context);

_Noreturn void arghtos_port_fault_terminal(void);
```

It also declares:

```c
void arghtos_timer_interrupt(void);
```

for invocation by the selected timer vector/adapter.

# Kernel-private header: arghtos_internal.h

This header may contain:

* C8 `arghtos_task_control_t`;
* C9 `arghtos_kernel_state_t`;
* C4 SVC IDs;
* C6 selector/dispatch callbacks;
* internal lifecycle transition helpers;
* fatal entry;
* saturating-counter helpers;
* private globals.

Representative required internal symbols include the task trampoline pair:

```c
_Noreturn void arghtos_task_start_trampoline(void);
_Noreturn void arghtos_task_return_guard(void);
```

Representative required internal symbols:

```c
arghtos_task_id_t arghtos_select_next(void);
uintptr_t arghtos_kernel_dispatch(uintptr_t outgoing_saved_psp);

arghtos_result_t arghtos_kernel_service(
    uint8_t service_id,
    uint32_t arg0);

_Noreturn void arghtos_fatal(
    arghtos_fault_reason_t reason,
    const arghtos_exception_frame_t *exception_frame,
    uint32_t exc_return);
```

# Visibility rule

Application translation units may include `arghtos.h`.

Port translation units may include both `arghtos.h` and `arghtos_port.h`, plus target-private headers under `port/<target>/`.

Portable kernel translation units may include `arghtos_internal.h` and the architecture-neutral `arghtos_port.h`, but no target-private header.

Application code must never include `arghtos_internal.h`.

# C/C++ linkage

Public and port headers use:

```c
#ifdef __cplusplus
extern "C" {
#endif

/* declarations */

#ifdef __cplusplus
}
#endif
```

# Symbol namespace

All externally linkable scheduler symbols begin with:

```text
arghtos_
```

All preprocessor configuration symbols begin with:

```text
ARGHTOS_
```

No generic names such as `task_t`, `yield`, `init`, or `timer_arm` are exported.

# Resident-code rule

Public headers should prefer declarations over nontrivial `static inline` implementations.

Any machine code or literal data emitted into application translation units because of an ArghtOS header still counts as ArghtOS-induced resident footprint in the size report. Inlining is permitted only when measured to reduce total linked SRAM footprint.

# Dangerous aspects

* Exposing C8/C9 in the public header and freezing implementation layout accidentally.
* Putting vendor device headers in `arghtos.h`, `arghtos_port.h`, `arghtos_internal.h`, or `arghtos_kernel.c`.
* Allowing application code to call private dispatch/context helpers.
* Creating duplicate public and interrupt-specific APIs with inconsistent semantics.
* Leaking Cortex-M assembly details into portable kernel C.

# Boundary specification

C10 defines compile-time visibility only. Behavioral meaning remains owned by C0-C9.

# Port-private header

A target may define:

```text
port/<target>/arghtos_port_private.h
```

for target-only register definitions, vendor/CMSIS includes, exception constants, frame details, and port-local helpers.

That header must never be included by portable kernel code.

# No target conditionals in portable kernel

The portable kernel must not contain target-selection branches such as:

```c
#ifdef __ARM_ARCH_8M_BASE__
...
#endif
```

Target selection occurs at build/link composition by choosing one `port/<target>/` implementation.
