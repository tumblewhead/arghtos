---
type: CodeModuleDesign
title: "D05 - Time, sleep, and periodic release engine"
description: "Design of the bounded timed-task scan, periodic catch-up arithmetic, sleep deadlines, wrap-safe earliest-deadline selection, and timer re-arming."
tags: [design, timer, tickless, periodic, sleep, wraparound]
status: draft
---

# Goal

Own all scheduler-visible timed eligibility so no lifecycle or port code invents a second timer policy.

# Logical module

```text
TIME_RELEASE
```

# Physical implementation

Portable release logic in:

```text
src/arghtos_kernel.c
```

Platform time observation / one-shot timer realization in D06.

# External kernel entry

```c
void arghtos_timer_interrupt(void);
```

Called from the configured platform timer handler.

# Private interfaces

Proposed logical helpers:

```c
static bool time_due(arghtos_time_t now, arghtos_time_t t);

static void time_process_due_locked(
    arghtos_time_t now,
    bool *readiness_changed);

static void time_rearm_locked(arghtos_time_t now);

static void time_set_sleep_locked(
    arghtos_task_id_t id,
    arghtos_time_t deadline);

static void missed_release_add(
    arghtos_task_control_t *c,
    uint32_t count);
```

The implementation may combine the process/rearm scans to minimize code.

# Timed-task representation

No heap, timer queue, or timing wheel exists.

Timed state is already in C8:

```text
periodic task:
    next_time = next ideal periodic release

async SLEEPING:
    next_time = sleep deadline
```

At most one timed deadline exists per task.

# Timer handler algorithm

Reference logic:

```text
enter scheduler serialization
now = port_time_now()
changed = false
earliest_delta = none

for id in [0, task_count):
    d = descriptor[id]
    c = control[id]

    if d.period != 0:
        if due(now, c.next_time):
            late = now - c.next_time
            releases = 1 + late / d.period
            c.next_time += releases * d.period

            if c.state == DORMANT:
                c.state = READY
                changed = true
                missed = releases - 1
            else:
                missed = releases

            sat_add(c.missed_release_count, missed)

        consider c.next_time for earliest future timer

    else if c.state == SLEEPING:
        if due(now, c.next_time):
            c.state = READY
            c.age_credit = 0
            changed = true
        else:
            consider c.next_time

arm earliest future timer or disarm
exit serialization

if changed:
    request dispatch
```

No loop is proportional to `releases`.

# Saturating missed-release add

`releases` can be much larger than 255.

Do not truncate before saturation.

Safe logical rule:

```c
uint32_t room = UINT8_MAX - c->missed_release_count;
if (count >= room)
    c->missed_release_count = UINT8_MAX;
else
    c->missed_release_count += (uint8_t)count;
```

Exact boundary handling must be tested for `room=0`.

# Future-deadline comparison

When a timestamp `t` is known not due:

```c
uint32_t delta = t - now;
```

By contract:

```text
0 < delta < ARGHTOS_TIME_HALF_RANGE
```

Choose the candidate with the smallest unsigned `delta`.

This is simpler and safer than directly comparing wrapped absolute timestamps.

# Sleep insertion

When D03 commits RUNNING -> SLEEPING:

```text
control.next_time = deadline
control.state = SLEEPING
```

Then D05 must ensure the hardware timer still targets the earliest timed event.

Because v1 stores no global "currently armed deadline" field, the reference implementation performs one bounded rearm scan after a new sleep deadline is installed.

This costs O(N) but saves global state and avoids stale-timer bugs.

# Start integration

D02 sets every periodic:

```text
next_time = start_time + period
state = READY
```

Then D05/ D02 common earliest-deadline logic arms the minimum next periodic release.

The release-0 job is already READY; the timer is for release 1.

# Periodic return interaction

A periodic task returning before its next release becomes DORMANT.

D05 does not change `next_time` on return.

When the next ideal epoch arrives, D05 performs DORMANT -> READY.

# Delayed service example

Suppose:

```text
period = 10
next_time = 100
now = 137
```

Then:

```text
late = 37
releases = 1 + 37/10 = 4
next_time = 100 + 4*10 = 140
```

If task was DORMANT:

* release at 100 becomes the one active READY job;
* 110,120,130 are three missed releases;
* missed_release_count += 3.

If task was already active:

* all four releases are missed;
* no new instance is created.

# Key goals

* O(N) runtime timer service.
* No periodic drift.
* No timer-event queue.
* Wrap-safe comparisons.
* One active periodic instance.
* Sleep remains timer-only.

# Dependencies

* C1, C3, C8, C9.
* P4, P6, P7.
* D02, D03, D06.

# Dangerous code sections and tester guidance

## 1. Looping once per missed release

**Bad implementation:**

```c
while (time_due(now, c->next_time)) {
    c->next_time += period;
    c->missed_release_count++;
}
```

After a long delay and a 1-tick period, this can execute billions of iterations.

**Tester should:**

* use period=1 and lateness near `0x7FFFFFFF`;
* instrument loop/operation count;
* require timer service task inspections to remain proportional to N, not lateness;
* static-review that no `while(due)` catch-up loop exists.

## 2. Truncating releases before saturation

**Bad implementation:**

```c
uint8_t missed = releases - 1;
```

If releases=257, missed becomes 0.

**Tester should:**

* test releases_due values 1,2,254,255,256,257,65536;
* assert the counter saturates correctly;
* start from counter values 0,254,255.

## 3. Direct comparison of wrapped absolute timestamps

**Bad implementation:**

```c
if (candidate < earliest)
```

This fails when `now` is near `UINT32_MAX` and one deadline wraps to a small number.

**Tester should:**

* place now at `0xFFFFFFF0`;
* use deadlines `0xFFFFFFF8` and `0x00000010`;
* assert +8 ticks is selected before +32 ticks;
* property-test relative deltas across wrap.

## 4. Sleep installs a new earlier deadline but timer is not rearmed

**Failure scenario:**

* timer currently armed for periodic deadline +1000;
* current async task sleeps until +20;
* kernel blocks it but leaves timer at +1000.

The task oversleeps by 980 ticks.

**Tester should:**

* mock the port and record every arm;
* insert sleep deadlines earlier/later than current earliest;
* assert final armed target equals minimum future deadline.

## 5. Timer handler changes a READY periodic task back to READY and loses missed count

A due release for a task already READY/RUNNING is a missed release, not a new job.

**Tester should:**

* test periodic states DORMANT, READY, RUNNING;
* for each, process 1 and multiple due releases;
* assert active-instance count remains one;
* verify missed-release count formula exactly.

## 6. Release drift from `now + period`

**Bad implementation:**

```c
next_time = now + period;
```

Handler latency permanently shifts the phase.

**Tester should:**

* inject varying timer lateness across 1000 periods;
* assert ideal epochs remain integer multiples of the original start epoch;
* compare against a golden arithmetic model.

## 7. Half-range ambiguity is accepted

A sleep deadline or timer service more than half-range away makes signed due comparison ambiguous.

**Tester should:**

* test relative sleep values `0x7FFFFFFF` legal and `0x80000000` illegal;
* model timer service delayed to exactly half-range and require an integration failure/assert rather than ambiguous scheduling.

# Test seam

Host port provides:

* controllable `now`;
* captured arm/disarm target;
* dispatch-request counter.

Property tests should compare D05 against a Python/C reference model for randomized task tables and timestamps around wrap.

# Boundary specification

D05 owns timed eligibility and hardware timer target selection. It does not choose the CPU winner or change processor context.
