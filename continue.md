# ArghtOS live continuation checkpoint

Last refreshed: 2026-10-28

Read [agents.md](agents.md) first. Repository state is authoritative.

## Active repository state

```text
repo:   https://github.com/tumblewhead/arghtos
base:   main @ dcf188aae8765d0d65d729af706da4bdaa922474
branch: implementation/r4-feature-specialization-001
PR:     #3 (draft)
```

Soft target:

```text
1536 B complete ArghtOS resident footprint
24-task default
```

Physical linker output-section occupancy is authoritative. All ArghtOS code,
constants, data, generated scheduler state, crash state, port code, assembly,
alignment/linker metadata, and ArghtOS-only MSP count.

## Minimum-size policy

Runtime profiling is optional in the minimum-size production profile and
enabled by default elsewhere.

Minimum-size work may trade cycles for bytes, but must retain scheduler
semantics, C11 defined behavior, portability boundaries, bounded work, crash
integrity, wake/wait correctness, and required invariants.

## Accepted footprint chain

```text
physical R3c                           3080 B
R4h corrected physical baseline       2778 B
R4m full LTO                           2530 B
R4n omit frame pointer                 2450 B
R4p no ARM unwind index                2434 B
R4q no jump tables                     2410 B
R4s split task metadata                2402 B
R4u entry/next-time lifetime overlay   2322 B
---------------------------------------------
current accepted minimum               2322 B
soft target                            1536 B
remaining gap                           786 B
```

R4u:

```text
linked before MSP  2242 B
MSP                  80 B
total               2322 B
```

Authoritative R4u CI:

```text
40665626084  success
```

R4u changes task metadata:

```text
R4s 432 B
R4u 336 B
raw saving 96 B
code-growth tax 16 B
net saving 80 B
```

The overlaid word contains task entry before init and runtime `next_time`
after init. The Cortex-M23 initial frame carries entry in stacked R0. The
trampoline retains entry in R4, already covered by the normal R4-R11 context,
so there is no extra PSP context/frame storage.

See [implementation/R4u-entry-next-overlay.md](implementation/R4u-entry-next-overlay.md).

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
R4t ID-native metadata indexing     0 B
```

R4t CI: 40665299726, success, R4s=R4t=2402 B.

## Exact next action: R4v compact PendSV assembly

R4u retains the original 130-byte Cortex-M23 PendSV handler.

R4v should preserve the exact C4 frame and execution protocol while using
denser Thumb-1 save/restore sequences:

Outgoing task:

```text
save R4-R7 with STMIA
reuse R4-R7 as temporaries for R8-R11
save those four with a second STMIA
```

Incoming task:

```text
restore R4-R7 with LDMIA
restore high-register values in two LDMIA pairs
move them into R8-R11
```

Also test replacing the literal load of EXC_RETURN 0xFFFFFFFD with:

```asm
movs r2, #2
mvns r2, r2
```

Requirements:

1. Keep R4u as same-revision control.
2. Put compact PendSV behind an assembly build define for R4v only.
3. Preserve saved context exactly:
   * 32 B software R4-R11;
   * existing hardware frame;
   * outgoing context token still points at saved R4.
4. Preserve first-dispatch MSP frame reclamation and xPSR alignment handling.
5. Preserve PRIMASK save/restore and race-free idle loop.
6. Run all host/sanitizer/portability gates plus Cortex-M23 link.
7. Inspect R4u/R4v disassembly and symbol sizes.
8. Re-audit normal MSP; assembly compaction itself must not increase C stack.
9. Accept only if total physical footprint <2322 B.
10. Refresh implementation docs and this file.

## Architecture rules

* portable kernel remains target-independent C11;
* Cortex-M23 mechanics remain under `port/cortex_m23/`;
* no dynamic allocation;
* periodic synchronous RMS semantics remain;
* async age/wake semantics remain;
* arithmetic missed-release catch-up remains;
* required 64-byte crash record remains;
* no hidden application-owned exclusions;
* do not remove invariants merely for bytes.

## Useful files

* [agents.md](agents.md)
* this file
* [implementation/R4u-entry-next-overlay.md](implementation/R4u-entry-next-overlay.md)
* [architecture/contracts/C4-context-switch-port.md](architecture/contracts/C4-context-switch-port.md)
* [design-elements/D06-cortex-m23-port.md](design-elements/D06-cortex-m23-port.md)
* [proof/P2-context-stack-integrity.md](proof/P2-context-stack-integrity.md)
* `port/cortex_m23/arghtos_context.S`
* `Makefile`
* `.github/workflows/ci.yml`
