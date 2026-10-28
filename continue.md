# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-28

Read [agents.md](agents.md) first. Repository state is authoritative.

## Canonical repository

```text
https://github.com/tumblewhead/arghtos
```

Canonical base:

```text
main @ dcf188aae8765d0d65d729af706da4bdaa922474
PR #2 merged: R3 production-profile validation and service specialization
```

## Active work

```text
branch: implementation/r4-feature-specialization-001
draft PR: #3
https://github.com/tumblewhead/arghtos/pull/3
```

Current accepted minimum-size profile:

```text
R4q = 2410 B
```

Soft target:

```text
1536 B
remaining gap = 874 B
```

## Accounting authority

All ArghtOS-induced resident bytes count: code, constants/literals, data,
BSS/noinit, generated scheduler metadata, crash state, port C, assembly,
linker metadata/alignment, and ArghtOS-only MSP.

The footprint reporter uses physical linker output-section occupancy and
subtracts only explicit application-owned `.app_*` inputs.

Historical R3c physical total is 3080 B.

Application task bodies and PSP stacks are outside the ArghtOS-overhead subtotal
but still count in total MCU SRAM.

## Minimum-size policy

Runtime profiling is optional in minimum-size production and default-on
elsewhere.

Minimum-size work may trade cycles/runtime for resident bytes, but must
preserve:

* scheduler semantics;
* C11 defined behavior;
* portable-kernel boundary;
* bounded work required by P7;
* crash integrity;
* wake/wait race behavior;
* required internal invariants unless upstream contracts/proofs change.

Only linked Cortex-M23 evidence is authoritative.

## Accepted R4 chain

```text
physical R3c                           3080 B

R4a optional runtime profiling         -227 B feature delta
R4b size-first selector                 -64 B
R4g remove dead dispatch clock read      -8 B
R4h remove unused start epoch            -3 B

R4h corrected physical baseline        2778 B

R4m full LTO, MSP requalified to 88 B  2530 B
R4n omit frame pointer, MSP = 80 B      2450 B
R4p discard unused ARM unwind index     2434 B
R4q -fno-jump-tables                    2410 B
----------------------------------------------
current accepted minimum                2410 B
soft target                             1536 B
remaining gap                            874 B
```

R4q linked before MSP:

```text
2330 B
```

R4q MSP lower bound:

```text
80 B
```

R4q authoritative CI:

```text
40663959695  success
```

R4q saving is concentrated in:

```text
wake_locked              108 ->  96 B
arghtos_kernel_service   228 -> 216 B
```

See [implementation/R4q-no-jump-tables.md](implementation/R4q-no-jump-tables.md).

## Rejected / neutral experiments

Do not repeat without a materially different hypothesis:

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
```

## Live experiment: R4r machine outliner

R4r adds LLVM machine outlining to accepted R4q:

```text
-mllvm -enable-machine-outliner=always
```

Current CI comparison:

```text
R4q accepted control
R4r live challenger
run 40664239986
```

Do not accept R4r from source inspection. Measure physical footprint and
re-audit MSP if outlining changes call/frame structure.

## Dormant split-metadata work

The branch also contains an inactive experimental switch:

```text
ARGHTOS_SPLIT_META
```

It was initially developed under the R4q label before R4q became the accepted
no-jump-tables experiment. Do not call it R4q.

If tested next, call it **R4s**.

Its hypothesis is:

```text
current unified slot: 20 B/task * 24 = 480 B

split core:
  context + next_time + entry + schedule = 16 B/task
parallel metadata:
  age_credit + status                    =  2 B/task

raw metadata total = 18 B/task
raw opportunity    = 48 B
```

The representation keeps 32-bit fields naturally aligned. It intentionally may
increase address calculations.

The split code is dormant unless `ARGHTOS_SPLIT_META=1`; accepted R4q does
not define that switch.

## Exact next action

1. Finish R4r measurement against same-revision R4q.
2. If R4r is smaller:
   * inspect linked disassembly for outlined helper calls;
   * re-audit the 80 B static MSP lower bound;
   * accept only after MSP remains valid or is updated.
3. If R4r is neutral/larger, reject it.
4. Then test the dormant split-metadata experiment as **R4s** on top of the
   accepted compiler/linker profile:
   * full LTO;
   * `-fomit-frame-pointer`;
   * `-fno-jump-tables`;
   * no unwind index;
   * accepted size-first source switches;
   * `ARGHTOS_SPLIT_META=1`.
5. Keep the accepted profile as same-revision control.
6. Accept R4s only if its 48 B metadata reduction exceeds any code-growth tax.
7. Refresh implementation documentation and this file.

## Important architecture rules

* `src/arghtos_kernel.c` remains portable C11.
* Cortex-M23 mechanisms belong under `port/cortex_m23/`.
* No dynamic allocation.
* Periodic tasks are synchronous RMS, phase 0, D=T, one active instance.
* Async work never outranks READY periodic work.
* Async age credit remains scheduler state.
* Wake/wait retains the one-bit pending-wake handshake.
* Delayed periodic release uses arithmetic catch-up.
* Required 64-byte crash record remains.
* Do not hide scheduler-required state in application-owned exclusions.
* Do not remove internal invariants merely for bytes.

## Useful files

* [agents.md](agents.md)
* this file
* [implementation/R4m-lto.md](implementation/R4m-lto.md)
* [implementation/R4n-omit-frame-pointer.md](implementation/R4n-omit-frame-pointer.md)
* [implementation/R4p-no-exidx.md](implementation/R4p-no-exidx.md)
* [implementation/R4q-no-jump-tables.md](implementation/R4q-no-jump-tables.md)
* [design-elements/D10-build-footprint.md](design-elements/D10-build-footprint.md)
* [proof/P7-bounded-memory-and-work.md](proof/P7-bounded-memory-and-work.md)
* `src/arghtos_internal.h`
* `src/arghtos_kernel.c`
* `Makefile`
* `.github/workflows/ci.yml`
* `footprint/m23_min_no_exidx.ld`
* `tools/footprint_report.py`
