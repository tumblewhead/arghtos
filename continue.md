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
R4y frozen control             = 2294 B
R5a accepted minimum            = 2290 B
linked before MSP               = 2210 B
ArghtOS-only MSP                =   80 B
soft target                     = 1536 B
remaining gap                   =  754 B
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

## R5a accepted result

Authoritative CI:

```text
40670407899  success

R4y 2294 B
R5a 2290 B
saved   4 B
```

`PendSV_Handler` shrank from 108 B to 104 B. All measured C symbols are
unchanged and the 80 B MSP bound remains valid.

See
[implementation/R5a-compact-pendsv-tail.md](implementation/R5a-compact-pendsv-tail.md).

## Exact next action: R5b compact EXC_RETURN bit tests

Keep R5a as same-revision control.

R5b should add one assembly-only define on top of R5a.

Current PendSV entry test:

```asm
mov  r3, lr
movs r2, #4
tst  r3, r2
beq  1f
```

R5b candidate:

```asm
mov  r3, lr
lsrs r3, r3, #3
bcc  1f
```

LSR #3 places original EXC_RETURN bit 2 in carry. Carry clear means bit 2 was
zero, selecting the MSP/first-dispatch path. R3 is scratch after the test.

Current HardFault stack-selection test:

```asm
mov  r0, lr
mrs  r2, msp
mrs  r3, psp
movs r1, #4
tst  r0, r1
beq  1f
```

R5b candidate:

```asm
mov  r0, lr
mrs  r2, msp
mrs  r3, psp
lsrs r1, r0, #3
bcc  1f
```

R0 retains the original EXC_RETURN value passed to the C hard-fault bridge.

Acceptance:

1. R5a remains control.
2. R5b includes accepted R5a compaction plus only these bit-test changes.
3. Run frozen host/sanitizer/portability gates.
4. Link R5a/R5b same revision.
5. Inspect PendSV/HardFault disassembly.
6. Verify stack-selection semantics and MSP bound.
7. Accept only if physical total <2290 B.
8. Record result and refresh this file.

