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
linked before MSP               = 2206 B
ArghtOS-only MSP                =   80 B
soft target                     = 1536 B
remaining gap                   =  750 B
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

## Exact next action: R5d compact service outcome

Keep R5b as accepted control.

R5d should use one internal byte throughout the specialized minimum-profile
`arghtos_kernel_service()`:

```text
bits 0..2  public result
bit 3      request dispatch
```

Yield, wait, sleep, wake, and task-return should write this one byte directly.

The wake path should reuse the R5c compact wake helper, but **must not decode it
back into separate locals inside the service switch**.

After the critical section:

```text
if dispatch bit set -> request PendSV
return low three result bits
```

Acceptance:

1. Preserve R5b same-revision control.
2. Add dedicated R5d host profile and Cortex-M23 target.
3. Retain all public results and wake/state semantics.
4. Run frozen host/sanitizer/portability gates.
5. Compare service, wake helper, wake interrupt wrapper, and total linked size.
6. Re-audit MSP if call frames change.
7. Accept only if total <2286 B.
8. Record and refresh this file.
