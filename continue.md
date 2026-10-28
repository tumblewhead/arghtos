# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-28

Read [agents.md](agents.md) first. Repository state is authoritative.

## Canonical repository

```text
https://github.com/tumblewhead/arghtos
```

Canonical base: R3 merged to `main` at:

```text
dcf188aae8765d0d65d729af706da4bdaa922474
PR #2: R3 production-profile validation and service specialization
```

## Active work

```text
branch: implementation/r4-feature-specialization-001
draft PR: #3
https://github.com/tumblewhead/arghtos/pull/3
```

Current branch head when this handoff was refreshed:

```text
4c8a93b1204c1f939f501ce16e192c6b211c413c
```

## Objective and accounting

Soft target:

```text
1536 B complete ArghtOS resident footprint
```

Default capacity: 24 tasks.

All ArghtOS-induced resident bytes count: code, const/literals, data, BSS/noinit,
configuration/runtime metadata, crash state, target port, assembly, linker
resident metadata, alignment/fill, and ArghtOS-only MSP.

Application task bodies and PSP stacks remain outside the ArghtOS-overhead
subtotal but still count in total MCU SRAM.

The footprint reporter now treats physical linker output-section occupancy as
authoritative and subtracts only explicit application-owned `.app_*` inputs.
This corrected historical R3c from 3072 B attributed to 3080 B physical.

## Minimum-size production policy

Runtime profiling is optional in the minimum-size production configuration.

Generic/debug and ordinary production configurations retain profiling.

Minimum-size work may trade runtime/cycles for fewer resident bytes, but must
preserve:

* scheduler semantics;
* C11 defined behavior;
* portable-kernel boundary;
* P7 bounded work;
* crash integrity;
* wake/wait race behavior;
* required internal invariants unless an upstream contract/proof change is made.

Only linked Cortex-M23 evidence counts.

## Accepted R4 chain

```text
physical R3c                          3080 B

R4a no runtime profiling             2853 B physical-equivalent
     historical same-revision report 2845 B before +8 B accounting correction

R4b size-first selector              -64 B
R4g remove dead dispatch clock read   -8 B
R4h remove unused start epoch         -3 B source-level linked delta

R4h corrected physical baseline      2778 B

R4m full LTO + requalified MSP       2530 B
     linked before MSP               2442 B
     MSP                               88 B

R4n + -fomit-frame-pointer           2450 B
     linked before MSP               2370 B
     MSP                               80 B
-------------------------------------------
current accepted minimum             2450 B
soft target                          1536 B
remaining gap                         914 B
```

R4n is the current accepted minimum-size Cortex-M23 profile.

R4n CI evidence is recorded in
[implementation/R4n-omit-frame-pointer.md](implementation/R4n-omit-frame-pointer.md).

## Rejected / neutral R4 experiments

Do not repeat these without a materially different hypothesis:

```text
R4c looped crash copy              +8 B
R4d whole-prefix crash clear        0 B
R4e timer sentinel                 +4 B
R4f specialized startup timer       0 B
R4i service if-chain                0 B
R4j fault range coding              0 B
R4k two-pass time engine           +4 B
R4l wake-result hoist               0 B
R4o lld --icf=safe                  0 B
```

R4o authoritative CI:

```text
40663421625  success
R4n 2450 B
R4o 2450 B
```

See the corresponding `implementation/R4*.md` checkpoints.

## Current largest R4n symbols

```text
arghtos_static_slots           480 B
arghtos_kernel_dispatch        264 B
process_time_locked            256 B
arghtos_fatal                  236 B
arghtos_kernel_service         228 B
arghtos_init                   152 B
PendSV_Handler                 130 B
wake_locked                    108 B
arghtos_start                   96 B
arghtos_task_start_trampoline   72 B
crash record                    64 B
```

The R4n linked symbol report and ELF are archived by CI.

## Exact next action: R4p

A prepared target already exists:

```text
make m23-footprint-r4p
footprint/m23_min_no_exidx.ld
```

R4p uses the accepted R4n compiler/profile configuration but discards
`.ARM.exidx*` and `.ARM.extab*` in the minimum-size linker script.

The current R4n footprint contains 16 B of linker resident metadata, so the
maximum direct opportunity is 16 B.

Next agent must:

1. Keep R4n as the accepted same-revision control.
2. Replace R4o as the one live CI experiment with R4p.
3. Run generic host tests, accepted size-first selector tests, R3c reference,
   R4h non-LTO reference, sanitizers, portability, Cortex-M23 compile, R4n, and
   R4p.
4. Record R4p footprint JSON/map/symbol evidence.
5. Confirm the minimum profile has no exception-unwind consumer requiring
   `.ARM.exidx`/`.ARM.extab`; the build is freestanding, C-only plus explicit
   assembly, with unwind tables already disabled.
6. Accept R4p only if physical resident size decreases with no semantic/build
   regression.
7. Update `implementation/` and this file before moving to the next experiment.

Do not merge PR #3 unless the user asks.

## Important architecture rules

* `src/arghtos_kernel.c` remains portable C11.
* Cortex-M23 mechanisms belong under `port/cortex_m23/`.
* No dynamic allocation.
* Periodic tasks: synchronous RMS, phase 0, D=T, one active instance.
* Async work never outranks READY periodic work.
* Async age credit remains scheduler state.
* Wake/wait retains the one-bit pending-wake handshake.
* Delayed periodic release uses arithmetic catch-up.
* Required 64-byte crash record remains.
* Do not hide ArghtOS-required state in application-owned exclusions.
* Do not remove scheduler-internal invariants merely for bytes.

## Useful files for the next step

* [agents.md](agents.md)
* this file
* [implementation/R4m-lto.md](implementation/R4m-lto.md)
* [implementation/R4n-omit-frame-pointer.md](implementation/R4n-omit-frame-pointer.md)
* [implementation/R4o-safe-icf.md](implementation/R4o-safe-icf.md)
* [design-elements/D10-build-footprint.md](design-elements/D10-build-footprint.md)
* [proof/P7-bounded-memory-and-work.md](proof/P7-bounded-memory-and-work.md)
* `Makefile`
* `.github/workflows/ci.yml`
* `footprint/m23.ld`
* `footprint/m23_min_no_exidx.ld`
* `tools/footprint_report.py`
