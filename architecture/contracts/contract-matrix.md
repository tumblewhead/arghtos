---
type: ContractMap
title: "ArghtOS contract matrix"
description: "Compact call-context, ownership, proof traceability, and resident-footprint reference."
tags: [architecture, contract, matrix, api, ownership, footprint]
status: draft
---

# Public API matrix

| Symbol | Valid context | State effect | May reschedule | Failure form |
|---|---|---|---|---|
| `arghtos_init(config)` | startup Thread/MSP before start | initializes runtime state | no | result code |
| `arghtos_start()` | startup Thread/MSP after init | starts scheduling | yes, permanent handoff | fatal on impossible state |
| `arghtos_yield()` | TASK | RUNNING -> READY | yes | result code |
| `arghtos_wait()` | TASK async only | WAITING or consume wake latch | yes if blocked | result code |
| `arghtos_sleep_until(t)` | TASK async only | RUNNING -> SLEEPING | yes if future | result code |
| `arghtos_sleep_for(dt)` | TASK async only | RUNNING -> SLEEPING | yes if nonzero | result code |
| `arghtos_wake(id)` | TASK | async eligibility/latch | maybe | result code |
| `arghtos_wake_from_interrupt(id)` | maskable HANDLER | async eligibility/latch | requests PendSV | result code |
| `arghtos_profile_get(id,out)` | TASK | bounded snapshot | no | result code |

# Port API matrix

| Symbol | Implemented by | Called by | Requirement |
|---|---|---|---|
| `arghtos_port_init()` | platform | init | configure time/dispatch port |
| `arghtos_port_stack_valid(base,size)` | port | init | bounded stack check |
| `arghtos_port_time_now()` | platform | kernel | bounded 32-bit monotonic read |
| `arghtos_port_timer_arm(t)` | platform | kernel | absolute one-shot target |
| `arghtos_port_timer_disarm()` | platform | kernel | cancel timer target |
| `arghtos_port_critical_enter()` | port | kernel | serialize scheduler mutation |
| `arghtos_port_critical_exit(key)` | port | kernel | exact prior-mask restore |
| `arghtos_port_is_task_context()` | port | API validation | Thread/PSP task check |
| `arghtos_port_is_handler_context()` | port | API validation | supported handler check |
| `arghtos_port_request_dispatch()` | port | kernel | pend PendSV |
| `arghtos_port_start()` | port | startup | first handoff |
| `arghtos_port_idle_wait()` | port | dispatch path | idle with no READY task |
| `arghtos_port_build_initial_psp(...)` | port | init | synthesize 64 B initial frame; configured stack reserves 68 B worst-case context overhead |
| `arghtos_port_fault_terminal()` | platform | fatal path | reset/halt, no return |

# Kernel-private objects

| Object/symbol | Owner | Resident size / purpose |
|---|---|---|
| C0 descriptor table | B0 | 16 B per configured task, read-only SRAM |
| `arghtos_task_control_t controls[]` | B1 | 20 B per configured task |
| `arghtos_kernel_state_t kernel` | top level | 20 B |
| crash record | B5/application storage | 64 B when enabled |
| `arghtos_select_next()` | B2 | bounded class-first selector |
| `arghtos_kernel_dispatch(psp)` | B2/B4 boundary | serialized execution handoff |
| `arghtos_timer_interrupt()` | B3 | bounded timed release scan |
| `arghtos_fatal(...)` | B5 | fixed crash capture |

# State transition ownership

| Transition | Owner |
|---|---|
| DORMANT -> READY by async activation | C1/C2 |
| DORMANT -> READY by periodic release | C3 |
| READY -> RUNNING | C6 |
| RUNNING -> READY by preemption | C6 |
| RUNNING -> READY by yield | C1 |
| RUNNING -> WAITING | C1 |
| WAITING -> READY | C1/C2 |
| RUNNING -> SLEEPING | C1 |
| SLEEPING -> READY | C3 |
| periodic RUNNING -> DORMANT on return | C1/C4 |
| async return -> fatal | C1/C5 |

# Proof traceability

| Contract | Primary proofs |
|---|---|
| C0 | P0, P2, P6 |
| C1 | P1, P3, P4 |
| C2 | P3, P5, P7 |
| C3 | P4, P6, P7 |
| C4 | P2, P7 |
| C5 | P6, P7 |
| C6 | P1, P5, P6, P7, P8 |
| C7 | P0, P4, P7 |
| C8 | P0, P1, P7, P8 |
| C9 | P0, P7 |
| C10 | boundary enforcement |

# Default 32-task metadata lower bound

```text
C0 descriptors   32 * 16 = 512 B
C8 controls      capacity 32 * 20 = 640 B
C9 global                  20 B
--------------------------------
metadata                   1172 B

optional crash              64 B
metadata + crash           1236 B
```

# Complete soft-target quantity

The 1536-byte target applies to complete ArghtOS resident footprint:

```text
ArghtOS + port code/const/data/metadata + MSP <= 1536 B   SHOULD
```

All ArghtOS `.text`, `.rodata`, literal pools, `.data`, `.bss`, `.noinit`, required scheduler metadata, and MSP count.

At 32 tasks, metadata leaves only 364 B before optional crash capture, or 300 B with crash capture, for code/constants/MSP/other bytes.

Therefore the soft target is currently **open and high risk**, not satisfied by architecture arithmetic.

# System total

Application task stacks and task bodies are excluded from the ArghtOS-overhead subtotal by prior requirement, but S0 must include them in the total MCU SRAM budget.

# Boundary specification

This matrix summarizes C0-C10. Owning contracts remain authoritative and must be updated together with this matrix.
