# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-28

Read [agents.md](agents.md) first. Repository state is authoritative.

## Active repository state

```text
repo:   https://github.com/tumblewhead/arghtos
base:   main @ bf44d8495074360f2fb6283322e323ff7f62d24f
branch: implementation/r5-size-specialization-001
phase:  R5 footprint reduction
```

PR #3 was squash-merged to main. R4 is frozen.

## Frozen control

Default capacity:

```text
24 tasks
```

Final R4 control:

```text
R4y complete resident footprint = 2294 B
linked before MSP               = 2214 B
ArghtOS-only MSP                 =   80 B
soft target                      = 1536 B
remaining gap                    =  758 B
```

The final same-revision profiling-enabled R3c reference on the frozen R4 source
was 3104 B.

R4y remains the immutable control for R5 experiments.

## R5 rules

R5 may trade runtime cycles for resident bytes but must preserve:

* portable C11 kernel boundary;
* synchronous periodic RMS semantics;
* async residual-capacity semantics and age credit;
* wake/wait race closure;
* arithmetic delayed-release catch-up;
* required 64-byte crash record;
* no heap;
* bounded work;
* Cortex-M23 task-context ABI unless an owning contract/proof is explicitly
  updated;
* correct first-dispatch MSP reclamation and alignment handling;
* PRIMASK and race-free idle protocol.

Only linked Cortex-M23 evidence counts.

## Current accepted minimum-size profile

R4y uses:

* trusted/static production configuration;
* compact runtime/descriptor/unified slot;
* no runtime profiling in minimum profile;
* size-first selector;
* no dispatch clock read;
* no start epoch;
* split task metadata;
* entry/next-time lifetime overlay;
* full LTO;
* `-fomit-frame-pointer`;
* `-fno-jump-tables`;
* no ARM unwind index;
* compact PendSV R4-R11 save/restore;
* direct SVC branch to saved EXC_RETURN.

## Exact next action: R5a compact PendSV tail/alignment

Keep R4y as same-revision control.

R5a should change only `port/cortex_m23/arghtos_context.S`, behind an
R5a-only assembly define.

### First-dispatch xPSR alignment decode

Current:

```asm
ldr  r1, [r5, #28]
lsrs r1, r1, #9
movs r2, #1
ands r1, r2
lsls r1, r1, #2
```

Experiment:

```asm
ldr  r1, [r5, #28]
lsrs r1, r1, #7
movs r2, #4
ands r1, r2
```

Both produce either 0 or 4 from xPSR bit 9.

### PendSV exception return

Current compact path:

```asm
mov lr, r2
bx  lr
```

R5a:

```asm
bx r2
```

R4y already proved the same direct-EXC_RETURN branch principle in SVC.

### Acceptance

1. Preserve R4y control.
2. Add R5a target only; do not mutate R4y.
3. Run frozen host/sanitizer/portability gates.
4. Link R4y and R5a in the same revision.
5. Inspect disassembly and symbol sizes.
6. Verify:
   * first-dispatch alignment result remains exactly 0 or 4;
   * saved task frame remains unchanged;
   * PSP/MSP ownership is unchanged;
   * PRIMASK/idle behavior is unchanged;
   * MSP bound remains 80 B.
7. Accept only if physical total <2294 B.
8. Record result and refresh this file.

After R5a, the next likely assembly experiment is compact EXC_RETURN bit-2
testing in PendSV/HardFault. Do not combine it with R5a until R5a is measured.
