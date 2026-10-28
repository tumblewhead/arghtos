#include <arghtos/arghtos.h>
#include <arghtos/arghtos_port.h>
#include "arghtos_internal.h"
#include "arghtos_kernel_port.h"

#include <stdatomic.h>

#ifndef ARGHTOS_TRUSTED_CONFIG
#define ARGHTOS_TRUSTED_CONFIG 0
#endif

#if ARGHTOS_STATIC_CONFIG && !ARGHTOS_TRUSTED_CONFIG
#error "ARGHTOS_STATIC_CONFIG requires ARGHTOS_TRUSTED_CONFIG=1"
#endif

#if ARGHTOS_STATIC_CONFIG
extern const arghtos_task_config_t arghtos_static_tasks[ARGHTOS_STATIC_TASK_COUNT];
extern arghtos_crash_record_t arghtos_static_crash_record;
#define ARGHTOS_TASK_TABLE   (arghtos_static_tasks)
#define ARGHTOS_TASK_COUNT   ((uint8_t)ARGHTOS_STATIC_TASK_COUNT)
#define ARGHTOS_CRASH_RECORD (&arghtos_static_crash_record)
#else
#define ARGHTOS_TASK_TABLE   (kernel.tasks)
#define ARGHTOS_TASK_COUNT   (kernel.task_count)
#define ARGHTOS_CRASH_RECORD (kernel.crash_record)
#endif

static arghtos_task_control_t controls[ARGHTOS_MAX_TASKS];
static arghtos_kernel_state_t kernel;

static bool time_due(arghtos_time_t now, arghtos_time_t t)
{
    return (int32_t)(now - t) >= 0;
}

static bool duration_valid(arghtos_duration_t dt)
{
    return dt < ARGHTOS_TIME_HALF_RANGE;
}

static bool task_id_valid(arghtos_task_id_t id)
{
    return id < ARGHTOS_TASK_COUNT;
}

static bool task_is_periodic(arghtos_task_id_t id)
{
    return ARGHTOS_TASK_TABLE[id].period != 0u;
}

static bool task_is_async(arghtos_task_id_t id)
{
    return ARGHTOS_TASK_TABLE[id].period == 0u;
}

static void control_clear(arghtos_task_control_t *c)
{
    c->context = ARGHTOS_CONTEXT_NONE;
    c->next_time = 0u;
    c->total_cpu_ticks = 0u;
#if ARGHTOS_COMPACT_RUNTIME
    c->max_run_ticks = 0u;
    c->age_credit = 0u;
    c->status = 0u;
    ARGHTOS_CTRL_SET_STATE(c, ARGHTOS_TASK_DORMANT);
#else
    c->max_run_ticks = 0u;
    c->missed_release_count = 0u;
    c->age_credit = 0u;
    c->state = ARGHTOS_TASK_DORMANT;
    c->flags = 0u;
#endif
}

#if !ARGHTOS_TRUSTED_CONFIG
static bool range_end(uintptr_t base, uint32_t size, uintptr_t *end)
{
    if ((uintptr_t)size > UINTPTR_MAX - base) {
        return false;
    }
    *end = base + (uintptr_t)size;
    return true;
}

static bool stacks_overlap(
    const arghtos_task_config_t *a,
    const arghtos_task_config_t *b)
{
    const uintptr_t ab = (uintptr_t)a->stack_base;
    const uintptr_t bb = (uintptr_t)b->stack_base;
    uintptr_t ae;
    uintptr_t be;

    if (!range_end(ab, a->stack_size, &ae) ||
        !range_end(bb, b->stack_size, &be)) {
        return true;
    }

    return (ab < be) && (bb < ae);
}

static arghtos_result_t validate_descriptor(const arghtos_task_config_t *d)
{
    if ((d->entry == NULL) || (d->stack_base == NULL) ||
        (d->stack_size == 0u) || (d->flags != 0u)) {
        return ARGHTOS_ERR_BAD_CONFIG;
    }

    if (!arghtos_port_stack_valid(d->stack_base, d->stack_size)) {
        return ARGHTOS_ERR_BAD_CONFIG;
    }

    if (d->period == 0u) {
        if ((d->async_priority == ARGHTOS_PRIORITY_NONE) ||
            (d->async_priority > ARGHTOS_ASYNC_PRIORITY_MAX) ||
            (d->async_priority == ARGHTOS_PRIORITY_INVALID)) {
            return ARGHTOS_ERR_BAD_CONFIG;
        }
    } else {
        if (!duration_valid(d->period) || (d->async_priority != 0u)) {
            return ARGHTOS_ERR_BAD_CONFIG;
        }
    }

    return ARGHTOS_OK;
}

static arghtos_result_t validate_config(const arghtos_config_t *config)
{
    uint8_t i;
    uint8_t j;

    if ((config == NULL) || (config->tasks == NULL) ||
        (config->crash_record == NULL)) {
        return ARGHTOS_ERR_BAD_ARGUMENT;
    }

    if ((config->task_count == 0u) ||
        (config->task_count > ARGHTOS_MAX_TASKS)) {
        return ARGHTOS_ERR_BAD_CONFIG;
    }

    if (((uintptr_t)config->crash_record %
         _Alignof(arghtos_crash_record_t)) != 0u) {
        return ARGHTOS_ERR_BAD_CONFIG;
    }

    for (i = 0u; i < config->task_count; ++i) {
        const arghtos_result_t r = validate_descriptor(&config->tasks[i]);
        if (r != ARGHTOS_OK) {
            return r;
        }
    }

    for (i = 0u; i < config->task_count; ++i) {
        for (j = (uint8_t)(i + 1u); j < config->task_count; ++j) {
            if (stacks_overlap(&config->tasks[i], &config->tasks[j])) {
                return ARGHTOS_ERR_BAD_CONFIG;
            }
        }
    }

    return ARGHTOS_OK;
}

#endif

static void missed_release_add(arghtos_task_control_t *c, uint32_t count)
{
#if ARGHTOS_COMPACT_RUNTIME
    const uint32_t current = ARGHTOS_CTRL_MISSED(c);
    const uint32_t room = ARGHTOS_CTRL_MISSED_MAX - current;

    if (count >= room) {
        ARGHTOS_CTRL_SET_MISSED(c, ARGHTOS_CTRL_MISSED_MAX);
    } else {
        ARGHTOS_CTRL_SET_MISSED(c, current + count);
    }
#else
    const uint32_t room = (uint32_t)UINT8_MAX - c->missed_release_count;

    if (count >= room) {
        c->missed_release_count = UINT8_MAX;
    } else {
        c->missed_release_count =
            (uint8_t)(c->missed_release_count + (uint8_t)count);
    }
#endif
}

static void timer_consider(
    arghtos_time_t now,
    arghtos_time_t t,
    bool *have,
    arghtos_time_t *best,
    uint32_t *best_delta)
{
    const uint32_t delta = t - now;

    if ((!*have) || (delta < *best_delta)) {
        *have = true;
        *best = t;
        *best_delta = delta;
    }
}

static bool process_time_locked(arghtos_time_t now)
{
    bool changed = false;
    bool have_timer = false;
    arghtos_time_t best_time = 0u;
    uint32_t best_delta = 0u;
    uint8_t id;

    for (id = 0u; id < ARGHTOS_TASK_COUNT; ++id) {
        const arghtos_task_config_t *d = &ARGHTOS_TASK_TABLE[id];
        arghtos_task_control_t *c = &controls[id];

        if (d->period != 0u) {
            if (time_due(now, c->next_time)) {
                const uint32_t late = now - c->next_time;
                const uint32_t releases = 1u + (late / d->period);
                uint32_t missed;

                c->next_time += releases * d->period;

                if (ARGHTOS_CTRL_STATE(c) == ARGHTOS_TASK_DORMANT) {
                    ARGHTOS_CTRL_SET_STATE(c, ARGHTOS_TASK_READY);
                    changed = true;
                    missed = releases - 1u;
                } else if ((ARGHTOS_CTRL_STATE(c) == ARGHTOS_TASK_READY) ||
                           (ARGHTOS_CTRL_STATE(c) == ARGHTOS_TASK_RUNNING)) {
                    missed = releases;
                } else {
                    arghtos_fatal(ARGHTOS_FAULT_KERNEL_INVARIANT, NULL);
                }

                if (missed != 0u) {
                    missed_release_add(c, missed);
                }
            }

            timer_consider(
                now, c->next_time, &have_timer, &best_time, &best_delta);
        } else if (ARGHTOS_CTRL_STATE(c) == ARGHTOS_TASK_SLEEPING) {
            if (time_due(now, c->next_time)) {
                ARGHTOS_CTRL_SET_STATE(c, ARGHTOS_TASK_READY);
                c->age_credit = 0u;
                changed = true;
            } else {
                timer_consider(
                    now, c->next_time, &have_timer, &best_time, &best_delta);
            }
        }
    }

    if (have_timer) {
        arghtos_port_timer_arm(best_time);
    } else {
        arghtos_port_timer_disarm();
    }

    return changed;
}

static bool wake_locked(arghtos_task_id_t id, arghtos_result_t *result)
{
    arghtos_task_control_t *c;

    if (!task_id_valid(id)) {
        *result = ARGHTOS_ERR_BAD_TASK_ID;
        return false;
    }

    if (!task_is_async(id)) {
        *result = ARGHTOS_ERR_BAD_TASK_CLASS;
        return false;
    }

    c = &controls[id];

    switch (ARGHTOS_CTRL_STATE(c)) {
    case ARGHTOS_TASK_DORMANT:
    case ARGHTOS_TASK_WAITING:
        ARGHTOS_CTRL_SET_STATE(c, ARGHTOS_TASK_READY);
        c->age_credit = 0u;
        ARGHTOS_CTRL_CLEAR_WAKE(c);
        *result = ARGHTOS_OK;
        return true;

    case ARGHTOS_TASK_READY:
        *result = ARGHTOS_OK;
        return false;

    case ARGHTOS_TASK_RUNNING:
    case ARGHTOS_TASK_SLEEPING:
        ARGHTOS_CTRL_SET_WAKE(c);
        *result = ARGHTOS_OK;
        return false;

    case ARGHTOS_TASK_FAULTED:
        *result = ARGHTOS_ERR_BAD_STATE;
        return false;

    default:
        arghtos_fatal(ARGHTOS_FAULT_KERNEL_INVARIANT, NULL);
    }
}

static bool current_running(arghtos_task_id_t *id)
{
    if ((kernel.current_task == ARGHTOS_TASK_ID_INVALID) ||
        !task_id_valid(kernel.current_task) ||
        (ARGHTOS_CTRL_STATE(&controls[kernel.current_task]) != ARGHTOS_TASK_RUNNING)) {
        return false;
    }

    *id = kernel.current_task;
    return true;
}

arghtos_result_t arghtos_init(const arghtos_config_t *config)
{
    arghtos_result_t r;
    uint8_t id;
    const arghtos_task_config_t *init_tasks;
    arghtos_crash_record_t *init_crash;
    uint8_t init_count;

    if ((kernel.flags & ARGHTOS_KF_INITIALIZED) != 0u) {
        return ARGHTOS_ERR_BAD_STATE;
    }

#if ARGHTOS_STATIC_CONFIG
    (void)config;
    init_tasks = arghtos_static_tasks;
    init_crash = &arghtos_static_crash_record;
    init_count = (uint8_t)ARGHTOS_STATIC_TASK_COUNT;
#else
#if !ARGHTOS_TRUSTED_CONFIG
    r = validate_config(config);
    if (r != ARGHTOS_OK) {
        return r;
    }
#endif
    init_tasks = config->tasks;
    init_crash = config->crash_record;
    init_count = config->task_count;
#endif

    r = arghtos_port_init();
    if (r != ARGHTOS_OK) {
        return r;
    }

    for (id = 0u; id < init_count; ++id) {
        control_clear(&controls[id]);
        controls[id].context = arghtos_port_build_initial_context(
            init_tasks[id].stack_base,
            init_tasks[id].stack_size,
            arghtos_task_start_trampoline,
            arghtos_task_return_guard);

        if (controls[id].context == ARGHTOS_CONTEXT_NONE) {
            return ARGHTOS_ERR_BAD_CONFIG;
        }
    }

    for (; id < ARGHTOS_MAX_TASKS; ++id) {
        control_clear(&controls[id]);
    }

#if !ARGHTOS_STATIC_CONFIG
    kernel.tasks = init_tasks;
    kernel.crash_record = init_crash;
    kernel.task_count = init_count;
#else
    (void)init_crash;
#endif
    kernel.start_time = 0u;
    kernel.run_start = 0u;
    kernel.current_task = ARGHTOS_TASK_ID_INVALID;
    kernel.flags = ARGHTOS_KF_INITIALIZED;
#if ARGHTOS_STATIC_CONFIG
    kernel.reserved[0] = 0u;
    kernel.reserved[1] = 0u;
#else
    kernel.reserved = 0u;
#endif

    arghtos_port_timer_disarm();
    return ARGHTOS_OK;
}

ARGHTOS_NORETURN void arghtos_start(void)
{
    arghtos_time_t start;
    arghtos_irq_key_t key;
    bool have_timer = false;
    arghtos_time_t earliest = 0u;
    uint32_t earliest_delta = 0u;
    uint8_t id;

    if (((kernel.flags & ARGHTOS_KF_INITIALIZED) == 0u) ||
        ((kernel.flags & ARGHTOS_KF_STARTED) != 0u)) {
        arghtos_fatal(ARGHTOS_FAULT_KERNEL_INVARIANT, NULL);
    }

    key = arghtos_port_critical_enter();
    start = arghtos_port_time_now();
    kernel.start_time = start;

    for (id = 0u; id < ARGHTOS_TASK_COUNT; ++id) {
        if (task_is_periodic(id)) {
            ARGHTOS_CTRL_SET_STATE(&controls[id], ARGHTOS_TASK_READY);
            controls[id].next_time = start + ARGHTOS_TASK_TABLE[id].period;
            timer_consider(start, controls[id].next_time,
                           &have_timer, &earliest, &earliest_delta);
        }
    }

    kernel.flags |= ARGHTOS_KF_STARTED;

    if (have_timer) {
        arghtos_port_timer_arm(earliest);
    } else {
        arghtos_port_timer_disarm();
    }

    arghtos_port_critical_exit(key);
    arghtos_port_start();
}

arghtos_result_t arghtos_yield(void)
{
    if (!arghtos_port_is_task_context()) {
        return ARGHTOS_ERR_BAD_CONTEXT;
    }
    return arghtos_port_service_call(ARGHTOS_SERVICE_YIELD, 0u);
}

arghtos_result_t arghtos_wait(void)
{
    if (!arghtos_port_is_task_context()) {
        return ARGHTOS_ERR_BAD_CONTEXT;
    }
    return arghtos_port_service_call(ARGHTOS_SERVICE_WAIT, 0u);
}

arghtos_result_t arghtos_sleep_until(arghtos_time_t absolute_time)
{
    if (!arghtos_port_is_task_context()) {
        return ARGHTOS_ERR_BAD_CONTEXT;
    }
    return arghtos_port_service_call(
        ARGHTOS_SERVICE_SLEEP_UNTIL, absolute_time);
}

arghtos_result_t arghtos_sleep_for(arghtos_duration_t duration)
{
    arghtos_time_t now;

    if (!arghtos_port_is_task_context()) {
        return ARGHTOS_ERR_BAD_CONTEXT;
    }
    if (!duration_valid(duration)) {
        return ARGHTOS_ERR_BAD_TIME;
    }
    if (duration == 0u) {
        return ARGHTOS_OK;
    }

    now = arghtos_port_time_now();
    return arghtos_port_service_call(
        ARGHTOS_SERVICE_SLEEP_UNTIL, now + duration);
}

arghtos_result_t arghtos_wake(arghtos_task_id_t task_id)
{
    if (!arghtos_port_is_task_context()) {
        return ARGHTOS_ERR_BAD_CONTEXT;
    }
    return arghtos_port_service_call(ARGHTOS_SERVICE_WAKE, task_id);
}

arghtos_result_t arghtos_wake_from_interrupt(arghtos_task_id_t task_id)
{
    arghtos_irq_key_t key;
    arghtos_result_t result;
    bool dispatch;

    if (!arghtos_port_is_handler_context()) {
        return ARGHTOS_ERR_BAD_CONTEXT;
    }

    key = arghtos_port_critical_enter();

    if ((kernel.flags & ARGHTOS_KF_STARTED) == 0u) {
        arghtos_port_critical_exit(key);
        return ARGHTOS_ERR_BAD_STATE;
    }

    dispatch = wake_locked(task_id, &result);
    arghtos_port_critical_exit(key);

    if (dispatch) {
        arghtos_port_request_dispatch();
    }

    return result;
}

arghtos_result_t arghtos_profile_get(
    arghtos_task_id_t task_id,
    arghtos_task_profile_t *out)
{
    arghtos_irq_key_t key;
    const arghtos_task_control_t *c;

    if (!arghtos_port_is_task_context()) {
        return ARGHTOS_ERR_BAD_CONTEXT;
    }
    if (out == NULL) {
        return ARGHTOS_ERR_BAD_ARGUMENT;
    }
    if (!task_id_valid(task_id)) {
        return ARGHTOS_ERR_BAD_TASK_ID;
    }

    key = arghtos_port_critical_enter();
    c = &controls[task_id];
    out->total_cpu_ticks = c->total_cpu_ticks;
    out->max_run_ticks = ARGHTOS_CTRL_MAX_RUN(c);
    out->missed_release_count = ARGHTOS_CTRL_MISSED(c);
    out->age_credit = c->age_credit;
    out->state = ARGHTOS_CTRL_STATE(c);
    out->flags = ARGHTOS_CTRL_FLAGS(c);
    arghtos_port_critical_exit(key);

    return ARGHTOS_OK;
}

arghtos_result_t arghtos_kernel_service(uint8_t service_id, uint32_t arg0)
{
    arghtos_irq_key_t key;
    arghtos_task_id_t self;
    arghtos_result_t result = ARGHTOS_OK;
    bool dispatch = false;

    if (!current_running(&self)) {
        arghtos_fatal(ARGHTOS_FAULT_BAD_SERVICE, NULL);
    }

    key = arghtos_port_critical_enter();

    switch (service_id) {
    case ARGHTOS_SERVICE_YIELD:
        ARGHTOS_CTRL_SET_STATE(&controls[self], ARGHTOS_TASK_READY);
        if (task_is_async(self)) {
            controls[self].age_credit = 0u;
        }
        dispatch = true;
        break;

    case ARGHTOS_SERVICE_WAIT:
        if (!task_is_async(self)) {
            result = ARGHTOS_ERR_BAD_TASK_CLASS;
            break;
        }
        if (ARGHTOS_CTRL_WAKE_PENDING(&controls[self])) {
            ARGHTOS_CTRL_CLEAR_WAKE(&controls[self]);
        } else {
            ARGHTOS_CTRL_SET_STATE(&controls[self], ARGHTOS_TASK_WAITING);
            controls[self].age_credit = 0u;
            dispatch = true;
        }
        break;

    case ARGHTOS_SERVICE_SLEEP_UNTIL:
        if (!task_is_async(self)) {
            result = ARGHTOS_ERR_BAD_TASK_CLASS;
            break;
        } else {
            const arghtos_time_t now = arghtos_port_time_now();
            const arghtos_time_t deadline = (arghtos_time_t)arg0;
            const uint32_t delta = deadline - now;

            if (time_due(now, deadline)) {
                break;
            }
            if (!duration_valid(delta)) {
                result = ARGHTOS_ERR_BAD_TIME;
                break;
            }

            controls[self].next_time = deadline;
            ARGHTOS_CTRL_SET_STATE(&controls[self], ARGHTOS_TASK_SLEEPING);
            controls[self].age_credit = 0u;
            (void)process_time_locked(now);
            dispatch = true;
        }
        break;

    case ARGHTOS_SERVICE_WAKE:
        dispatch = wake_locked((arghtos_task_id_t)arg0, &result);
        break;

    case ARGHTOS_SERVICE_TASK_RETURN:
        if (!task_is_periodic(self)) {
            result = ARGHTOS_ERR_BAD_TASK_CLASS;
            break;
        }
        ARGHTOS_CTRL_SET_STATE(&controls[self], ARGHTOS_TASK_DORMANT);
        dispatch = true;
        break;

    default:
        arghtos_port_critical_exit(key);
        arghtos_fatal(ARGHTOS_FAULT_BAD_SERVICE, NULL);
    }

    arghtos_port_critical_exit(key);

    if (dispatch) {
        arghtos_port_request_dispatch();
    }

    return result;
}

arghtos_task_id_t arghtos_select_next(void)
{
    arghtos_task_id_t best = ARGHTOS_TASK_ID_INVALID;
    uint8_t id;

    for (id = 0u; id < ARGHTOS_TASK_COUNT; ++id) {
        if (task_is_periodic(id) &&
            (ARGHTOS_CTRL_STATE(&controls[id]) == ARGHTOS_TASK_READY)) {
            if ((best == ARGHTOS_TASK_ID_INVALID) ||
                (ARGHTOS_TASK_TABLE[id].period < ARGHTOS_TASK_TABLE[best].period) ||
                ((ARGHTOS_TASK_TABLE[id].period == ARGHTOS_TASK_TABLE[best].period) &&
                 (id < best))) {
                best = id;
            }
        }
    }

    if (best != ARGHTOS_TASK_ID_INVALID) {
        return best;
    }

    for (id = 0u; id < ARGHTOS_TASK_COUNT; ++id) {
        if (task_is_async(id) &&
            (ARGHTOS_CTRL_STATE(&controls[id]) == ARGHTOS_TASK_READY)) {
            const uint8_t ceiling =
                (uint8_t)ARGHTOS_ASYNC_PRIORITY_MAX;
            uint16_t raised;
            uint8_t effective;

            if (controls[id].age_credit != UINT8_MAX) {
                controls[id].age_credit++;
            }

            raised = (uint16_t)ARGHTOS_TASK_TABLE[id].async_priority +
                     (uint16_t)controls[id].age_credit;
            effective = (raised > ceiling) ? ceiling : (uint8_t)raised;

            if (best == ARGHTOS_TASK_ID_INVALID) {
                best = id;
            } else {
                const uint8_t best_ceiling =
                    (uint8_t)ARGHTOS_ASYNC_PRIORITY_MAX;
                const uint16_t best_raised =
                    (uint16_t)ARGHTOS_TASK_TABLE[best].async_priority +
                    (uint16_t)controls[best].age_credit;
                const uint8_t best_effective =
                    (best_raised > best_ceiling) ?
                    best_ceiling : (uint8_t)best_raised;

                if ((effective > best_effective) ||
                    ((effective == best_effective) &&
                     (controls[id].age_credit > controls[best].age_credit)) ||
                    ((effective == best_effective) &&
                     (controls[id].age_credit == controls[best].age_credit) &&
                     (id < best))) {
                    best = id;
                }
            }
        }
    }

    return best;
}

arghtos_context_t arghtos_kernel_dispatch(arghtos_context_t outgoing_context)
{
    const arghtos_time_t now = arghtos_port_time_now();
    arghtos_task_id_t next;

    if ((kernel.flags & ARGHTOS_KF_STARTED) == 0u) {
        arghtos_fatal(ARGHTOS_FAULT_BAD_DISPATCH, NULL);
    }

    if (outgoing_context != ARGHTOS_CONTEXT_NONE) {
        arghtos_task_control_t *c;
        uint32_t elapsed;

        if ((kernel.current_task == ARGHTOS_TASK_ID_INVALID) ||
            !task_id_valid(kernel.current_task)) {
            arghtos_fatal(ARGHTOS_FAULT_BAD_DISPATCH, NULL);
        }

        c = &controls[kernel.current_task];
        c->context = outgoing_context;

        elapsed = now - kernel.run_start;
        c->total_cpu_ticks += elapsed;
#if ARGHTOS_COMPACT_RUNTIME
        if (elapsed > (uint32_t)c->max_run_ticks) {
            c->max_run_ticks =
                (elapsed > UINT16_MAX) ? UINT16_MAX : (uint16_t)elapsed;
        }
#else
        if (elapsed > c->max_run_ticks) {
            c->max_run_ticks = elapsed;
        }
#endif

        if (ARGHTOS_CTRL_STATE(c) == ARGHTOS_TASK_RUNNING) {
            ARGHTOS_CTRL_SET_STATE(c, ARGHTOS_TASK_READY);
            if (task_is_async(kernel.current_task)) {
                c->age_credit = 0u;
            }
        }
    } else if (kernel.current_task != ARGHTOS_TASK_ID_INVALID) {
        arghtos_fatal(ARGHTOS_FAULT_BAD_DISPATCH, NULL);
    }

    next = arghtos_select_next();

    if (next == ARGHTOS_TASK_ID_INVALID) {
        kernel.current_task = ARGHTOS_TASK_ID_INVALID;
        return ARGHTOS_CONTEXT_NONE;
    }

    if ((ARGHTOS_CTRL_STATE(&controls[next]) != ARGHTOS_TASK_READY) ||
        (controls[next].context == ARGHTOS_CONTEXT_NONE)) {
        arghtos_fatal(ARGHTOS_FAULT_BAD_DISPATCH, NULL);
    }

    ARGHTOS_CTRL_SET_STATE(&controls[next], ARGHTOS_TASK_RUNNING);
    if (task_is_async(next)) {
        controls[next].age_credit = 0u;
    }

    kernel.current_task = next;
    kernel.run_start = now;
    return controls[next].context;
}

void arghtos_timer_interrupt(void)
{
    arghtos_irq_key_t key;
    bool changed;

    if ((kernel.flags & ARGHTOS_KF_STARTED) == 0u) {
        arghtos_port_timer_disarm();
        return;
    }

    key = arghtos_port_critical_enter();
    changed = process_time_locked(arghtos_port_time_now());
    arghtos_port_critical_exit(key);

    if (changed) {
        arghtos_port_request_dispatch();
    }
}

static bool task_fault_attributed(arghtos_fault_reason_t reason)
{
    return (reason == ARGHTOS_FAULT_ASYNC_RETURNED) ||
           (reason == ARGHTOS_FAULT_STACK_RANGE) ||
           (reason == ARGHTOS_FAULT_CONTEXT_CORRUPT) ||
           (reason == ARGHTOS_FAULT_CPU_EXCEPTION);
}

ARGHTOS_NORETURN void arghtos_fatal(
    arghtos_fault_reason_t reason,
    const void *port_fault_context)
{
    arghtos_crash_record_t *r;
    uint8_t i;

    if ((kernel.flags & ARGHTOS_KF_FAULT_ACTIVE) != 0u) {
        arghtos_port_fault_terminal();
    }

    kernel.flags |= ARGHTOS_KF_FAULT_ACTIVE;

    if (task_fault_attributed(reason) &&
        task_id_valid(kernel.current_task)) {
        ARGHTOS_CTRL_SET_STATE(&controls[kernel.current_task], ARGHTOS_TASK_FAULTED);
    }

    r = ARGHTOS_CRASH_RECORD;
    if (r != NULL) {
        r->commit_magic = 0u;
        r->magic = ARGHTOS_CRASH_MAGIC;
        r->version = ARGHTOS_CRASH_VERSION;
        r->size = (uint16_t)sizeof(*r);
        r->reason = reason;
        r->task_id = ARGHTOS_TASK_ID_INVALID;
        r->flags = 0u;
        r->timestamp = arghtos_port_time_now();
        r->port_format = 0u;
        r->port_words = 0u;
        r->reserved = 0u;

        if (task_id_valid(kernel.current_task)) {
            r->task_id = kernel.current_task;
            r->flags |= ARGHTOS_CRASH_F_TASK_VALID;
        }

        for (i = 0u; i < 10u; ++i) {
            r->port_state[i] = 0u;
        }

        arghtos_port_crash_fill(r, port_fault_context);
        if (r->port_words > 10u) {
            r->port_words = 10u;
        }
        if (r->port_words != 0u) {
            r->flags |= ARGHTOS_CRASH_F_PORT_STATE_VALID;
        }

        atomic_thread_fence(memory_order_release);
        r->commit_magic = ARGHTOS_CRASH_COMMIT_MAGIC;
    }

    arghtos_port_fault_terminal();
}

ARGHTOS_NORETURN void arghtos_task_start_trampoline(void)
{
    arghtos_task_id_t id = kernel.current_task;
    const arghtos_task_config_t *d;

    if (!task_id_valid(id)) {
        arghtos_fatal(ARGHTOS_FAULT_BAD_DISPATCH, NULL);
    }

    d = &ARGHTOS_TASK_TABLE[id];

    if (d->period != 0u) {
        for (;;) {
            arghtos_result_t r;
            d->entry();
            r = arghtos_port_service_call(ARGHTOS_SERVICE_TASK_RETURN, 0u);
            if (r != ARGHTOS_OK) {
                arghtos_fatal(ARGHTOS_FAULT_BAD_SERVICE, NULL);
            }
        }
    }

    d->entry();
    arghtos_fatal(ARGHTOS_FAULT_ASYNC_RETURNED, NULL);
}

ARGHTOS_NORETURN void arghtos_task_return_guard(void)
{
    arghtos_fatal(ARGHTOS_FAULT_KERNEL_INVARIANT, NULL);
}

#ifdef ARGHTOS_TEST
const arghtos_task_control_t *arghtos_test_controls(void)
{
    return controls;
}

const arghtos_kernel_state_t *arghtos_test_kernel(void)
{
    return &kernel;
}

void arghtos_test_reset(void)
{
    uint8_t id;

    for (id = 0u; id < ARGHTOS_MAX_TASKS; ++id) {
        control_clear(&controls[id]);
    }

    kernel.start_time = 0u;
    kernel.run_start = 0u;
    kernel.current_task = ARGHTOS_TASK_ID_INVALID;
    kernel.flags = 0u;
#if ARGHTOS_STATIC_CONFIG
    kernel.reserved[0] = 0u;
    kernel.reserved[1] = 0u;
#else
    kernel.tasks = NULL;
    kernel.crash_record = NULL;
    kernel.task_count = 0u;
    kernel.reserved = 0u;
#endif
}
#endif
