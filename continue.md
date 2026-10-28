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
R5h accepted minimum            = 1970 B
R5i accepted minimum            = 1914 B
R5j accepted minimum            = 1906 B
R5l accepted minimum            = 1822 B
linked before MSP               = 1750 B
ArghtOS-only MSP                =   72 B
soft target                     = 1536 B
remaining gap                   =  286 B
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

## R5g rejected result

```text
CI 40673681141  success

R5e 2202 B
R5g 2218 B
delta +16 B
```

R5g saved 48 B of configuration state but its general 32-bit schedule
reconstruction path added 64 B of code.

See [implementation/R5g-schedule16.md](implementation/R5g-schedule16.md).

## R5h accepted result

```text
CI 40674018494  success

R5e 2202 B
R5h 1970 B
saved 232 B
```

R5h keeps the fixed 16-bit representation but specializes zero-overflow
generated configurations to direct halfword access.

```text
.text          1910 -> 1726 B
.arghtos_cfg    312 ->  264 B
```

See
[implementation/R5h-direct-schedule16.md](implementation/R5h-direct-schedule16.md).

## R5i accepted result

```text
CI 40718436840  success

R5h 1970 B
R5i 1914 B
saved  56 B
```

R5i replaces the fixed 16-bit schedule table with O(1) fixed 8-bit schedule
codes plus exact overflow fallback. The target fixture has zero overflow
entries.

```text
.arghtos_cfg          264 -> 240 B
process_time_locked   208 -> 188 B
kernel_dispatch       188 -> 180 B
arghtos_start         100 ->  96 B
```

See [implementation/R5i-schedule8.md](implementation/R5i-schedule8.md).

## R5j accepted result

```text
CI 40719129049  success

R5i 1914 B
R5j 1906 B
saved   8 B
```

R5j is an accounting-only requalification of the exact R5i linked image.
The deepest linked normal ArghtOS-owned MSP chain is now:

```text
SVC explicit push               8 B
kernel_service frame           16 B
process_time_locked frame      48 B
-----------------------------------
total                          72 B
```

See [implementation/R5j-msp-72.md](implementation/R5j-msp-72.md).

## R5k rejected result

```text
CI 40719949754  success

R5j 1906 B
R5k 1918 B
delta +12 B
```

Moving Cortex-M23 crash payload extraction into a leaf Thumb helper shrank
`arghtos_fatal` 228 -> 160 B, but the new helper cost 78 B. Reject the exact
split.

See
[implementation/R5k-asm-crash-fill.md](implementation/R5k-asm-crash-fill.md).

## R5l accepted result

```text
CI 40721286853  success

R5j 1906 B
R5l 1822 B
saved  84 B
```

R5l moves initial Cortex-M23 task-frame construction into the trusted generated
application stack image.

```text
arghtos_init 156 -> 72 B
```

Task-stack SRAM is unchanged and remains application-owned. The trade is that
the generated initial frame is load-initialized instead of `NOLOAD`, increasing
load-image/startup initialization bytes.

C0, C4, D06, and P2 now record the generated-frame equivalence obligation.

See
[implementation/R5l-prebuilt-context.md](implementation/R5l-prebuilt-context.md).

## Exact next action: R5m pristine generated init

Keep R5l as accepted control at 1822 B.

R5l still runs a bounded per-task initialization loop that writes:

* `next_time = 0`;
* packed status = 0;
* then verifies each prebuilt context token is nonzero.

For the trusted generated profile these are already build/startup invariants:

* `arghtos_init()` is called exactly once;
* active context tokens are valid by generator evidence;
* mutable task fields are emitted in their required zero state;
* unused slots are zero/inactive;
* the startup image is materialized before `arghtos_init()`.

C4 also guarantees successful `arghtos_port_init()` leaves the scheduler timer
disarmed, so the final portable `arghtos_port_timer_disarm()` is redundant in
this profile.

R5m should specialize only the trusted generated prebuilt-context profile so
`arghtos_init()` performs:

```text
port_init()
if failure: return it
current_task = INVALID
flags = INITIALIZED
return OK
```

Generic/debug and non-R5m initialization remain unchanged.

Acceptance:

1. Preserve R5l same-revision control.
2. Add an R5m-only switch requiring R5l + trusted/minimal static config.
3. Host tests must prove valid generated startup semantics and existing
   post-init states.
4. Keep port-init failure propagation.
5. Do not change `arghtos_start()` or scheduler semantics.
6. Run host/sanitizer/portability + Cortex-M23 comparison.
7. Re-audit MSP only if call/frame structure changes.
8. Accept only if total <1822 B.
9. Promote the pristine-startup invariant into C0 if accepted.
10. Refresh this handoff.

