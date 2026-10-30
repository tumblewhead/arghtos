---
type: ImplementationCheckpoint
title: "R5 final target crossing - 1534-byte Cortex-M23 minimum"
description: "Late-stage accepted/rejected experiments that cross the 1536-byte complete resident-footprint soft target."
tags: [implementation, footprint, cortex-m23, target-met, accepted]
status: accepted
---

# Baseline

R5y was the last previously documented accepted checkpoint:

```text
R5y  1646 B
soft target 1536 B
gap          110 B
```

All figures below are complete physical ArghtOS resident footprint, including
the accepted ArghtOS-only MSP accounting.

# Late-stage chain

```text
R5y   1646 B
R5z   1618 B   -28 B   raw class-partitioned schedule values
R5aa  1618 B     0 B   delta-only timer experiment, neutral
R5ab  1590 B   -28 B   single-async generated selector specialization
R5ac  1550 B   -40 B   terminal-only fault-state mutation elision
R5ad  1542 B    -8 B   compact Cortex-M23 public service veneers + handler cuts
R5ae  1542 B     0 B   explicit local service-veneer section, neutral
R5af  1538 B    -4 B   assembly hard-fault context bridge
R5ag  1534 B    -4 B   link-time terminal-hook alias
--------------------------------
soft target 1536 B
margin          2 B
```

# R5z

The generated zero-overflow schedule8 profile already proves task class from the
R5p ID partition. R5z therefore stores raw period bytes in the periodic prefix
and raw async-priority bytes in the async suffix, eliminating redundant class
tag decoding.

Authoritative comparison is included in CI run 40807843504.

# R5aa

R5aa retained only a timer delta rather than both absolute time and delta.
Linked size was unchanged at 1618 B.

Decision: reject as neutral.

# R5ab

The target fixture and generated profile contain exactly one async task.
R5ab specializes residual-capacity selection for that single candidate while
preserving the required age-credit behavior.

Measured result:

```text
R5z   1618 B
R5ab  1590 B
saved   28 B
```

Authoritative CI: 40807843504.

# R5ac

For the trusted terminal fault path, mutating the current task state to FAULTED
has no subsequent scheduler-observable effect because fatal handling never
returns and recursive-fatal protection remains active.

R5ac removes only that terminal-only state mutation in the specialized profile.

Measured result:

```text
R5ab  1590 B
R5ac  1550 B
saved   40 B
```

Authoritative CI: 40807843504.

# R5ad

R5ad keeps the portable C API in generic/host builds but lets the minimum
Cortex-M23 target provide equivalent YIELD, WAIT, SLEEP_UNTIL, and WAKE veneers
in assembly.

It also:

* uses a tighter first-dispatch xPSR-alignment decode in PendSV;
* shares the final HardFault branch;
* leaves R1 unspecified for YIELD/WAIT because those services never read
  `arg0`.

Measured result:

```text
R5ac  1550 B
R5ad  1542 B
saved    8 B
```

Authoritative CI: 40811343656.

# R5ae

R5ae attempted to force the service call and veneers into one explicit assembly
section so short branches could be selected at assembly time.

The linked result remained 1542 B.

Decision: reject as neutral.

Authoritative CI: 40811655572.

# R5af

The former C hard-fault bridge built this 16-byte port context with a stack
allocation and four scalar stores:

```text
msp
psp
exc_return
basic_frame
```

R5af uses two Thumb PUSH instructions:

```asm
push {r0, r1}
push {r2, r3}
```

Because PUSH stores registers in ascending register order while decrementing SP,
the resulting memory layout is exactly:

```text
[sp+0]  r2 = msp
[sp+4]  r3 = psp
[sp+8]  r0 = exc_return
[sp+12] r1 = basic_frame
```

The bridge symbol shrinks from 20 B to 14 B. Linked physical footprint improves
by 4 B after layout effects.

```text
R5ad  1542 B
R5af  1538 B
saved    4 B
```

The bridge still consumes the same 16 B of MSP, so the accepted 64 B
ArghtOS-only MSP bound remains valid.

Authoritative CI: 40812166244.

# R5ag

The Cortex-M23 port wrapper:

```c
arghtos_port_fault_terminal()
```

contained only a call to the board terminal routine. R5ag removes that 4-byte
wrapper from the minimum target build and resolves the abstract port symbol
directly at link time:

```text
arghtos_port_fault_terminal = arghtos_m23_board_fault_terminal
```

Portable kernel C continues to reference the abstract port hook. Generic and
non-minimum target builds keep the ordinary wrapper.

Authoritative CI:

```text
40812507607  success
```

Final measured result:

```text
linked before MSP  1470 B
ArghtOS-only MSP      64 B
-------------------------
R5ag total          1534 B
soft target         1536 B
margin                 2 B
target_met           true
```

The required crash record remains 64 B and task/config state remains 240 B.

# Decision

Accept R5ag as the first verified ArghtOS minimum-size Cortex-M23 profile to
meet the 1536-byte complete resident-footprint soft target.

The 2-byte margin is real but small. Any cleanup, linker-script change,
toolchain update, or source reorganization must re-link R5ag and re-check the
physical footprint before claiming the target remains met.

# Post-freeze semantic erratum

R6ai later discovered that R5z raw schedule encoding made the shared task-start
trampoline misclassify the raw async-priority byte as a nonzero period.

Accordingly, the historical R5ag 1534 B result remains authoritative footprint
evidence, but the merged minimum profile does **not** by itself establish the
claimed async-return-fatal behavior. A generated-context repair is required
before this size lineage can again be described as preserving that invariant.

See [R5z raw schedule erratum](R5z-raw-class-schedule.md).
