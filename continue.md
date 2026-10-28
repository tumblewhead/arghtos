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
R5b accepted minimum            = 2286 B
R5d accepted minimum            = 2270 B
R5e accepted minimum            = 2202 B
linked before MSP               = 2122 B
ArghtOS-only MSP                =   80 B
soft target                     = 1536 B
remaining gap                   =  666 B
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

## R5b accepted result

Authoritative CI:

```text
40670635274  success

R5a 2290 B
R5b 2286 B
saved   4 B
```

PendSV shrank 104 -> 102 B and HardFault 28 -> 26 B. MSP remains 80 B.

See
[implementation/R5b-compact-exc-checks.md](implementation/R5b-compact-exc-checks.md).

## R5c rejected result

```text
R5b 2286 B
R5c 2286 B
delta   0 B
```

Compact wake encoding shrank the wake helper by 12 B but enlarged
`arghtos_kernel_service` by 12 B because the encoded value was immediately
decoded back into separate result/dispatch locals.

See
[implementation/R5c-compact-wake-result.md](implementation/R5c-compact-wake-result.md).

## R5d accepted result

```text
CI 40671500777  success

R5b 2286 B
R5d 2270 B
saved 16 B
```

R5d carries one encoded service outcome byte through the entire specialized
service dispatcher.

See
[implementation/R5d-compact-service-outcome.md](implementation/R5d-compact-service-outcome.md).

## R5e accepted result

```text
CI 40671952282  success

R5d 2270 B
R5e 2202 B
saved 68 B
```

R5e overlays async age credit with the async sleep deadline. Task/config state
falls 336 -> 312 B and code also shrinks by 44 B.

See
[implementation/R5e-age-deadline-overlay.md](implementation/R5e-age-deadline-overlay.md).

## R5f rejected result

```text
CI 40672791022  success

R5e 2202 B
R5f 2306 B
delta +104 B
```

The variable-length schedule stream saves 72 B of configuration state but adds
176 B of text. Reject the scan-based decoder.

See
[implementation/R5f-variable-schedule-stream.md](implementation/R5f-variable-schedule-stream.md).

## Exact next action: R5g fixed 16-bit schedule codes

Keep R5e as the accepted control at 2202 B. R5f is not the base.

R5g should remove the 32-bit per-task schedule word exactly as R5f did, but use
an O(1) fixed-width 16-bit schedule table.

Proposed encoding:

```text
0x0001..0x7EFF  direct periodic period
0x8001..0x80FE  direct async priority 1..254
0x7F00..0x7F17  overflow index 0..23
other values    reserved / invalid
```

For an overflow code, an optional exact 32-bit table stores the original
schedule value. The footprint harness periods 1..24 require no overflow table.

R5g footprint data target:

```text
core slots     8 B * 24 = 192 B
status meta    1 B * 24 =  24 B
schedule16     2 B * 24 =  48 B
-------------------------------
total                     264 B
```

versus R5e 312 B, a raw 48 B opportunity.

The accessor must be O(1), preferably small enough to inline cleanly under LTO.
Full legal period/priority semantics must remain representable through the
overflow path.

Acceptance:

1. Preserve R5e same-revision control.
2. Build R5g on accepted R5a+b+d+e features, not R5f.
3. Host tests must exercise:
   * direct periodic code;
   * direct async code;
   * exact overflow periodic code.
4. Cortex-M23 footprint harness may use zero overflow entries.
5. Measure task/config state, accessor/code growth, and total resident bytes.
6. Accept only if total <2202 B.
7. Refresh this handoff.
