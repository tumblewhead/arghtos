# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-28

Read [agents.md](agents.md) first. Repository state is authoritative.

## Repository / active work

```text
repo:   https://github.com/tumblewhead/arghtos
base:   main @ dcf188aae8765d0d65d729af706da4bdaa922474
branch: implementation/r4-feature-specialization-001
PR:     #3 (draft)
```

## Objective

```text
soft target: 1536 B complete ArghtOS resident footprint
tasks:       24
```

All ArghtOS-induced resident bytes count: code, constants/literals, data,
BSS/noinit, generated scheduler metadata, crash state, target port, assembly,
linker metadata/alignment, and ArghtOS-only MSP.

Physical linker output-section occupancy is authoritative. Only explicit
application-owned `.app_*` inputs are excluded from the ArghtOS subtotal.

## Minimum-size production policy

Runtime profiling is optional in the minimum-size profile and remains enabled
by default elsewhere.

Minimum-size work may trade cycles/runtime for resident bytes, but must
preserve scheduler semantics, C11 defined behavior, portability boundaries,
bounded work, crash integrity, wake/wait correctness, and required invariants.

## Accepted footprint chain

```text
physical R3c                           3080 B

R4a optional runtime profiling         -227 B feature delta
R4b size-first selector                 -64 B
R4g remove dead dispatch clock read      -8 B
R4h remove unused start epoch            -3 B

R4h corrected physical baseline        2778 B
R4m full LTO, MSP 88 B                 2530 B
R4n omit frame pointer, MSP 80 B       2450 B
R4p discard unused unwind index        2434 B
R4q -fno-jump-tables                   2410 B
R4s split task metadata                2402 B
---------------------------------------------
current accepted minimum               2402 B
soft target                            1536 B
remaining gap                           866 B
```

R4s:

```text
linked before MSP  2322 B
MSP                  80 B
total               2402 B
```

Authoritative R4s CI:

```text
40664462833  success
```

Task metadata changed:

```text
R4q 480 B
R4s 432 B
raw saving 48 B
code-growth tax 40 B
net saving 8 B
```

See [implementation/R4s-split-meta.md](implementation/R4s-split-meta.md).

## Rejected / neutral experiments

```text
R4c looped crash copy              +8 B
R4d whole-prefix crash clear        0 B
R4e timer sentinel                 +4 B
R4f specialized startup timer       0 B
R4i service if-chain                0 B
R4j fault-range source rewrite      0 B
R4k two-pass time engine           +4 B
R4l wake-result hoist               0 B
R4o lld --icf=safe                  0 B
R4r forced machine outliner         0 B
```

## Exact next action: R4t entry/next-time overlay experiment

The accepted R4s core is:

```text
context    4 B
next_time  4 B
entry      4 B
schedule   4 B
----------------
core      16 B/task

parallel age/status metadata = 2 B/task
total                         = 18 B/task
```

Observation: `entry` is needed only when the task begins. `next_time` is
runtime state and is not needed before initialization has built the initial
task context.

R4t hypothesis:

1. Reuse one 4-byte core field as:
   * pre-init: task entry pointer;
   * post-init: `next_time`.
2. Pass the entry pointer into the initial task context in the Cortex-M23
   hardware frame R0 slot.
3. Change the common portable trampoline to accept the entry function as its
   initial argument and retain/use that argument for periodic re-entry.
4. Keep `context/initial_stack_top` and `schedule` as the other two 4-byte
   core fields.

Target R4t core:

```text
context_or_initial_stack_top   4 B
entry_or_next_time             4 B
schedule                       4 B
----------------------------------
core                          12 B/task
parallel metadata              2 B/task
----------------------------------
total                         14 B/task
```

Raw metadata opportunity versus R4s:

```text
(18 - 14) * 24 = 96 B
```

This is an architecture/representation experiment, not a silent canonical
change.

Before accepting R4t:

* preserve R4s as same-revision control;
* update/extend the architecture-neutral port ABI only if required;
* keep target-specific frame construction under `port/cortex_m23/`;
* verify task-entry argument survives initial dispatch and repeated periodic
  jobs;
* verify async-return fatal behavior;
* verify no extra ArghtOS-resident table appears;
* report any PSP stack/register impact separately;
* re-audit MSP;
* update C4/C8/D06/D08 or other owning contracts/design docs before promotion.

If R4t is not smaller than 2402 B, reject and remove the experimental path.

## Architecture rules

* Portable kernel remains target-independent C11.
* Cortex-M23 details remain under `port/cortex_m23/`.
* No dynamic allocation.
* Periodic tasks: synchronous RMS, phase 0, D=T, one active instance.
* Async tasks never outrank READY periodic work.
* Age credit remains scheduler state.
* Wake/wait uses the one-bit pending-wake handshake.
* Periodic missed-release catch-up remains arithmetic.
* Required 64-byte crash record remains.
* Do not hide scheduler-required resident state in application-owned exclusions.
* Do not remove internal invariants merely for bytes.

## Useful files

* [agents.md](agents.md)
* this file
* [implementation/R4q-no-jump-tables.md](implementation/R4q-no-jump-tables.md)
* [implementation/R4s-split-meta.md](implementation/R4s-split-meta.md)
* [architecture/contracts/C4-context-switch-port.md](architecture/contracts/C4-context-switch-port.md)
* [architecture/contracts/C8-task-control-record.md](architecture/contracts/C8-task-control-record.md)
* [design-elements/D06-cortex-m23-port.md](design-elements/D06-cortex-m23-port.md)
* [design-elements/D08-task-trampoline.md](design-elements/D08-task-trampoline.md)
* [design-elements/D10-build-footprint.md](design-elements/D10-build-footprint.md)
* [proof/P2-context-stack-integrity.md](proof/P2-context-stack-integrity.md)
* [proof/P7-bounded-memory-and-work.md](proof/P7-bounded-memory-and-work.md)
* `src/arghtos_internal.h`
* `src/arghtos_kernel.c`
* `src/arghtos_kernel_port.h`
* `include/arghtos/arghtos_port.h`
* `port/cortex_m23/arghtos_port.c`
* `footprint/m23_footprint.c`
* `tests/test_static_profile.c`
* `Makefile`
* `.github/workflows/ci.yml`
