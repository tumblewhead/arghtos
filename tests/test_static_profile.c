#include <arghtos/arghtos.h>
#include "../src/arghtos_kernel_port.h"
#include "../port/host/arghtos_host_port.h"
#include "arghtos_test.h"

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

extern ARGHTOS_NORETURN void arghtos_task_return_guard(void);

#if ARGHTOS_R5V_UNIT_PERIOD_TIMER
#define ARGHTOS_TEST_PERIOD 1u
#else
#define ARGHTOS_TEST_PERIOD 10u
#endif

#if (ARGHTOS_R5G_SCHEDULE16 && !ARGHTOS_R5H_DIRECT_SCHEDULE16) || \
    (ARGHTOS_R5I_SCHEDULE8 && ARGHTOS_R5I_SCHEDULE_OVERFLOW_COUNT > 0u)
#define ARGHTOS_TEST_THREE_TASKS 1
#else
#define ARGHTOS_TEST_THREE_TASKS 0
#endif

static _Alignas(8) uint8_t stack0[256];
static _Alignas(8) uint8_t stack1[256];
#if ARGHTOS_TEST_THREE_TASKS
static _Alignas(8) uint8_t stack2[256];
#endif

static void periodic_task(void) {}
static void async_task(void) {}

#if ARGHTOS_UNIFIED_SLOT
arghtos_task_control_t arghtos_static_slots[ARGHTOS_MAX_TASKS] = {
    {
#if ARGHTOS_R5L_PREBUILT_CONTEXT
        .context = (arghtos_context_t)(uintptr_t)&stack0[sizeof(stack0) - 64u],
#if ARGHTOS_R6Q_PRESEEDED_PERIOD_OFFSETS
        .entry_or_next.next_time = ARGHTOS_TEST_PERIOD,
#else
        .entry_or_next.next_time = 0u,
#endif
#else
        .context = (arghtos_context_t)(uintptr_t)&stack0[sizeof(stack0)],
#if ARGHTOS_ENTRY_NEXT_OVERLAY
        .entry_or_next.entry = periodic_task,
#else
        .entry = periodic_task,
#endif
#endif
#if !ARGHTOS_R5F_COMPRESSED_SCHEDULE && !ARGHTOS_R5G_SCHEDULE16 && !ARGHTOS_R5I_SCHEDULE8
        .schedule = ARGHTOS_STATIC_PERIOD(10u)
#endif
    },
    {
#if ARGHTOS_R5L_PREBUILT_CONTEXT
        .context = (arghtos_context_t)(uintptr_t)&stack1[sizeof(stack1) - 64u],
        .entry_or_next.next_time = 0u,
#else
        .context = (arghtos_context_t)(uintptr_t)&stack1[sizeof(stack1)],
#if ARGHTOS_ENTRY_NEXT_OVERLAY
        .entry_or_next.entry = async_task,
#else
        .entry = async_task,
#endif
#endif
#if !ARGHTOS_R5F_COMPRESSED_SCHEDULE && !ARGHTOS_R5G_SCHEDULE16 && !ARGHTOS_R5I_SCHEDULE8
        .schedule = ARGHTOS_STATIC_ASYNC(3u)
#endif
    }
#if ARGHTOS_TEST_THREE_TASKS
    ,{
#if ARGHTOS_R5L_PREBUILT_CONTEXT
        .context = (arghtos_context_t)(uintptr_t)&stack2[sizeof(stack2) - 64u],
        .entry_or_next.next_time = 0u,
#else
        .context = (arghtos_context_t)(uintptr_t)&stack2[sizeof(stack2)],
#if ARGHTOS_ENTRY_NEXT_OVERLAY
        .entry_or_next.entry = periodic_task,
#else
        .entry = periodic_task,
#endif
#endif
    }
#endif
};
#if ARGHTOS_R5F_COMPRESSED_SCHEDULE
/*
 * Exercise both decoder forms while preserving the existing test semantics:
 * task 0 uses escape + exact 32-bit period 10, task 1 uses one-byte async 3.
 */
uint8_t arghtos_static_schedule_stream[] = {
    ARGHTOS_R5F_SCHEDULE_ESCAPE, 10u, 0u, 0u, 0u,
    0x83u
};
#endif
#if ARGHTOS_R5G_SCHEDULE16
#if ARGHTOS_R5H_DIRECT_SCHEDULE16
uint16_t arghtos_static_schedule16[ARGHTOS_MAX_TASKS] = {
    10u,
    ARGHTOS_R5G_ASYNC_BASE | 3u
};
#else
/*
 * task 0: overflow index 0 -> exact period 10
 * task 1: direct async priority 3
 * task 2: direct periodic period 11
 */
uint16_t arghtos_static_schedule16[ARGHTOS_MAX_TASKS] = {
    ARGHTOS_R5G_OVERFLOW_BASE | 0u,
    ARGHTOS_R5G_ASYNC_BASE | 3u,
    11u
};
uint32_t arghtos_static_schedule_overflow[ARGHTOS_R5G_SCHEDULE_OVERFLOW_COUNT] = {
    ARGHTOS_STATIC_PERIOD(10u)
};
#endif
#endif
#if ARGHTOS_R5I_SCHEDULE8
#if ARGHTOS_R5I_SCHEDULE_OVERFLOW_COUNT > 0u
uint8_t arghtos_static_schedule8[ARGHTOS_MAX_TASKS] = {
    ARGHTOS_R5I_OVERFLOW_BASE | 0u,
    ARGHTOS_R5I_ASYNC_BASE | 3u,
    11u
};
uint32_t arghtos_static_schedule8_overflow[ARGHTOS_R5I_SCHEDULE_OVERFLOW_COUNT] = {
    ARGHTOS_STATIC_PERIOD(10u)
};
#else
uint8_t arghtos_static_schedule8[ARGHTOS_MAX_TASKS] = {
    ARGHTOS_TEST_PERIOD,
#if ARGHTOS_R5Z_RAW_CLASS_SCHEDULE
    3u
#else
    ARGHTOS_R5I_ASYNC_BASE | 3u
#endif
};
#endif
#endif
#if ARGHTOS_SPLIT_META
#if ARGHTOS_R6K_PRESEEDED_PERIODIC_READY
arghtos_task_meta_t arghtos_static_meta[ARGHTOS_MAX_TASKS] = {
    { .status = ARGHTOS_TASK_READY }
};
#else
arghtos_task_meta_t arghtos_static_meta[ARGHTOS_MAX_TASKS];
#endif
#endif
#elif ARGHTOS_COMPACT_DESCRIPTOR
const arghtos_static_task_t arghtos_static_tasks[2] = {
    { periodic_task, &stack0[sizeof(stack0)], ARGHTOS_STATIC_PERIOD(10u) },
    { async_task,    &stack1[sizeof(stack1)], ARGHTOS_STATIC_ASYNC(3u) }
};
#else
const arghtos_task_config_t arghtos_static_tasks[2] = {
    { periodic_task, stack0, 10u, sizeof(stack0), 0u, 0u },
    { async_task,    stack1, 0u,  sizeof(stack1), 3u, 0u }
};
#endif

#if ARGHTOS_R5S_PRESEEDED_CRASH
arghtos_crash_record_t arghtos_static_crash_record = {
    .magic = ARGHTOS_CRASH_MAGIC,
    .version = ARGHTOS_CRASH_VERSION,
    .size = (uint16_t)sizeof(arghtos_crash_record_t),
    .task_id = ARGHTOS_TASK_ID_INVALID
#if ARGHTOS_R5W_TRUSTED_CRASH_PORT || ARGHTOS_R6Z_TRUSTED_CRASH_PAYLOAD
    ,.flags = ARGHTOS_CRASH_F_PORT_STATE_VALID
#endif
#if ARGHTOS_R6AC_PRESEEDED_PORT_FORMAT
    ,.port_format = (uint16_t)ARGHTOS_R6AC_PORT_FORMAT_VALUE
#endif
};
#else
arghtos_crash_record_t arghtos_static_crash_record;
#endif

static void reset_all(void)
{
#if ARGHTOS_R5L_PREBUILT_CONTEXT
    /* Entry pointers live in the generated target stack image in this profile. */
    (void)periodic_task;
    (void)async_task;
#endif
    arghtos_test_reset();
#if ARGHTOS_R6K_PRESEEDED_PERIODIC_READY
    arghtos_static_meta[0].status = ARGHTOS_TASK_READY;
#endif
#if ARGHTOS_R6Q_PRESEEDED_PERIOD_OFFSETS
    ARGHTOS_CTRL_NEXT(&arghtos_static_slots[0]) = ARGHTOS_TEST_PERIOD;
    ARGHTOS_CTRL_NEXT(&arghtos_static_slots[1]) = 0u;
#endif
#if ARGHTOS_UNIFIED_SLOT
#if ARGHTOS_R5L_PREBUILT_CONTEXT
    arghtos_static_slots[0].context =
        (arghtos_context_t)(uintptr_t)&stack0[sizeof(stack0) - 64u];
    arghtos_static_slots[1].context =
        (arghtos_context_t)(uintptr_t)&stack1[sizeof(stack1) - 64u];
#else
    arghtos_static_slots[0].context =
        (arghtos_context_t)(uintptr_t)&stack0[sizeof(stack0)];
    arghtos_static_slots[1].context =
        (arghtos_context_t)(uintptr_t)&stack1[sizeof(stack1)];
#endif
#if ARGHTOS_TEST_THREE_TASKS
#if ARGHTOS_R5L_PREBUILT_CONTEXT
    arghtos_static_slots[2].context =
        (arghtos_context_t)(uintptr_t)&stack2[sizeof(stack2) - 64u];
#else
    arghtos_static_slots[2].context =
        (arghtos_context_t)(uintptr_t)&stack2[sizeof(stack2)];
#endif
#endif
#if ARGHTOS_ENTRY_NEXT_OVERLAY && !ARGHTOS_R5L_PREBUILT_CONTEXT
    ARGHTOS_CTRL_ENTRY(&arghtos_static_slots[0]) = periodic_task;
    ARGHTOS_CTRL_ENTRY(&arghtos_static_slots[1]) = async_task;
#if ARGHTOS_TEST_THREE_TASKS
    ARGHTOS_CTRL_ENTRY(&arghtos_static_slots[2]) = periodic_task;
#endif
#endif
#endif
    arghtos_host_reset();
#if ARGHTOS_R5S_PRESEEDED_CRASH
    memset(&arghtos_static_crash_record, 0, sizeof(arghtos_static_crash_record));
    arghtos_static_crash_record.magic = ARGHTOS_CRASH_MAGIC;
    arghtos_static_crash_record.version = ARGHTOS_CRASH_VERSION;
    arghtos_static_crash_record.size = (uint16_t)sizeof(arghtos_crash_record_t);
    arghtos_static_crash_record.task_id = ARGHTOS_TASK_ID_INVALID;
#if ARGHTOS_R5W_TRUSTED_CRASH_PORT || ARGHTOS_R6Z_TRUSTED_CRASH_PAYLOAD
    arghtos_static_crash_record.flags = ARGHTOS_CRASH_F_PORT_STATE_VALID;
#endif
#if ARGHTOS_R6AC_PRESEEDED_PORT_FORMAT
    arghtos_static_crash_record.port_format =
        (uint16_t)ARGHTOS_R6AC_PORT_FORMAT_VALUE;
#endif
#else
    memset(&arghtos_static_crash_record, 0, sizeof(arghtos_static_crash_record));
#endif
}

static void start_and_escape(void)
{
    jmp_buf env;

#if ARGHTOS_R6L_PREMASKED_START
    (void)arghtos_port_critical_enter();
    assert(arghtos_host_interrupts_masked());
#endif
    arghtos_host_set_start_jmp(&env);
    if (setjmp(env) == 0) {
        arghtos_start();
        assert(!"arghtos_start returned");
    }
    arghtos_host_set_start_jmp(NULL);
#if ARGHTOS_R6L_PREMASKED_START
    assert(!arghtos_host_interrupts_masked());
#endif
}

static void test_static_init_ignores_runtime_config(void)
{
    const arghtos_task_control_t *c;

    reset_all();
    arghtos_host_set_time(100u);

    assert(arghtos_init(NULL) == ARGHTOS_OK);
#if ARGHTOS_R6K_PRESEEDED_PERIODIC_READY
    c = arghtos_test_controls();
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_READY);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_DORMANT);
#endif
#if ARGHTOS_R6Q_PRESEEDED_PERIOD_OFFSETS
    assert(ARGHTOS_CTRL_NEXT(&c[0]) == ARGHTOS_TEST_PERIOD);
    assert(ARGHTOS_CTRL_NEXT(&c[1]) == 0u);
#endif
#if ARGHTOS_ENTRY_NEXT_OVERLAY && !ARGHTOS_R5L_PREBUILT_CONTEXT
#if ARGHTOS_TEST_THREE_TASKS
    assert(arghtos_host_initial_arg() == (uintptr_t)periodic_task);
#else
    assert(arghtos_host_initial_arg() == (uintptr_t)async_task);
#endif
#endif
    start_and_escape();

    c = arghtos_test_controls();
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_READY);
    assert(ARGHTOS_CTRL_NEXT(&c[0]) == 100u + ARGHTOS_TEST_PERIOD);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_DORMANT);
    assert(arghtos_host_timer_is_armed());
    assert(arghtos_host_timer_deadline() == 100u + ARGHTOS_TEST_PERIOD);
}

static void test_static_dispatch_and_async_wake(void)
{
    const arghtos_task_control_t *c;
    arghtos_context_t periodic_context;

    reset_all();
    assert(arghtos_init(NULL) == ARGHTOS_OK);
    start_and_escape();

    c = arghtos_test_controls();
    periodic_context = arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE);
    assert(periodic_context == c[0].context);
    assert(arghtos_test_kernel()->current_task == 0u);

    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
#if ARGHTOS_R6AD_ALWAYS_DISPATCH_WAKE
    arghtos_host_clear_dispatch_requests();
#endif
    assert(arghtos_wake_from_interrupt(1u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_READY);
    assert(ARGHTOS_CTRL_AGE(&c[1]) == 0u);
#if ARGHTOS_R6AD_ALWAYS_DISPATCH_WAKE
    assert(arghtos_host_dispatch_request_count() == 1u);

    /* Repeated READY wake changes no state but still requests reevaluation. */
    arghtos_host_clear_dispatch_requests();
    assert(arghtos_wake_from_interrupt(1u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_READY);
    assert(!ARGHTOS_CTRL_WAKE_PENDING(&c[1]));
    assert(arghtos_host_dispatch_request_count() == 1u);
#endif

    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
#if ARGHTOS_R6AD_ALWAYS_DISPATCH_WAKE
    /* Task-context WAKE uses the same unconditional dispatch policy. */
    arghtos_host_clear_dispatch_requests();
    assert(arghtos_port_service_call(ARGHTOS_SERVICE_WAKE, 1u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_READY);
    assert(arghtos_host_dispatch_request_count() == 1u);
#endif
#if ARGHTOS_R6AH_DEFAULT_DISPATCH_SERVICES
    arghtos_host_clear_dispatch_requests();
#endif
    assert(arghtos_port_service_call(
        ARGHTOS_SERVICE_TASK_RETURN, 0u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_DORMANT);
#if ARGHTOS_R6AH_DEFAULT_DISPATCH_SERVICES
    assert(arghtos_host_dispatch_request_count() == 1u);
#endif

#if ARGHTOS_TEST_THREE_TASKS
    {
        const arghtos_context_t second_periodic_context =
            arghtos_kernel_dispatch(periodic_context);

        assert(second_periodic_context == c[2].context);
        assert(arghtos_test_kernel()->current_task == 2u);

        assert(arghtos_port_service_call(
            ARGHTOS_SERVICE_TASK_RETURN, 0u) == ARGHTOS_OK);
        assert(ARGHTOS_CTRL_STATE(&c[2]) == ARGHTOS_TASK_DORMANT);

        assert(arghtos_kernel_dispatch(second_periodic_context) == c[1].context);
        assert(arghtos_test_kernel()->current_task == 1u);
    }
#else
    assert(arghtos_kernel_dispatch(periodic_context) == c[1].context);
    assert(arghtos_test_kernel()->current_task == 1u);
#endif
    assert(ARGHTOS_CTRL_AGE(&c[1]) == 0u);

    arghtos_host_clear_dispatch_requests();
    assert(arghtos_sleep_for(0u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_RUNNING);
    assert(arghtos_host_dispatch_request_count() == 0u);

#if ARGHTOS_R6AD_ALWAYS_DISPATCH_WAKE
    /* RUNNING wake preserves RUNNING, latches wake, and requests dispatch. */
    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    arghtos_host_clear_dispatch_requests();
    assert(arghtos_wake_from_interrupt(1u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_RUNNING);
    assert(ARGHTOS_CTRL_WAKE_PENDING(&c[1]));
    assert(arghtos_host_dispatch_request_count() == 1u);

    /* Consume the latch without blocking, then enter a real timed sleep. */
    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    arghtos_host_clear_dispatch_requests();
    assert(arghtos_port_service_call(ARGHTOS_SERVICE_WAIT, 0u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_RUNNING);
    assert(!ARGHTOS_CTRL_WAKE_PENDING(&c[1]));
#if ARGHTOS_R6AH_DEFAULT_DISPATCH_SERVICES
    assert(arghtos_host_dispatch_request_count() == 0u);
#endif

    arghtos_host_set_time(10u);
    assert(arghtos_sleep_until(20u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_SLEEPING);

    /* SLEEPING wake does not end sleep; it only latches + dispatches. */
    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    arghtos_host_clear_dispatch_requests();
    assert(arghtos_wake_from_interrupt(1u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_SLEEPING);
    assert(ARGHTOS_CTRL_WAKE_PENDING(&c[1]));
    assert(arghtos_host_dispatch_request_count() == 1u);
#endif
}

static void test_static_crash_record_symbol_is_used(void)
{
    jmp_buf env;

    reset_all();
#if ARGHTOS_R5S_PRESEEDED_CRASH
    assert(arghtos_static_crash_record.magic == ARGHTOS_CRASH_MAGIC);
    assert(arghtos_static_crash_record.version == ARGHTOS_CRASH_VERSION);
    assert(arghtos_static_crash_record.size == sizeof(arghtos_crash_record_t));
    assert(arghtos_static_crash_record.task_id == ARGHTOS_TASK_ID_INVALID);
#if ARGHTOS_R5W_TRUSTED_CRASH_PORT || ARGHTOS_R6Z_TRUSTED_CRASH_PAYLOAD
    assert(arghtos_static_crash_record.flags == ARGHTOS_CRASH_F_PORT_STATE_VALID);
#else
    assert(arghtos_static_crash_record.flags == 0u);
#endif
#if ARGHTOS_R6AC_PRESEEDED_PORT_FORMAT
    assert(arghtos_static_crash_record.port_format ==
           (uint16_t)ARGHTOS_R6AC_PORT_FORMAT_VALUE);
#endif
    assert(arghtos_static_crash_record.port_words == 0u);
    assert(arghtos_static_crash_record.commit_magic == 0u);
#endif
    assert(arghtos_init(NULL) == ARGHTOS_OK);
    start_and_escape();

    arghtos_host_set_fault_jmp(&env);
    if (setjmp(env) == 0) {
        arghtos_fatal(ARGHTOS_FAULT_KERNEL_INVARIANT, NULL);
        assert(!"fatal returned");
    }
    arghtos_host_set_fault_jmp(NULL);

    assert(arghtos_static_crash_record.magic == ARGHTOS_CRASH_MAGIC);
    assert(arghtos_static_crash_record.reason ==
           ARGHTOS_FAULT_KERNEL_INVARIANT);
    assert(arghtos_static_crash_record.commit_magic ==
           ARGHTOS_CRASH_COMMIT_MAGIC);
#if ARGHTOS_R6AC_PRESEEDED_PORT_FORMAT
    assert(arghtos_static_crash_record.port_format ==
           (uint16_t)ARGHTOS_R6AC_PORT_FORMAT_VALUE);
#endif
#if ARGHTOS_R5W_TRUSTED_CRASH_PORT || ARGHTOS_R6Z_TRUSTED_CRASH_PAYLOAD
    assert(arghtos_static_crash_record.port_words >= 1u);
    assert(arghtos_static_crash_record.port_words <= 10u);
    assert((arghtos_static_crash_record.flags &
            ARGHTOS_CRASH_F_PORT_STATE_VALID) != 0u);
#endif
#if ARGHTOS_R6AF_UNCONDITIONAL_CRASH_TASK_ID
    assert(arghtos_static_crash_record.task_id == ARGHTOS_TASK_ID_INVALID);
    assert((arghtos_static_crash_record.flags &
            ARGHTOS_CRASH_F_TASK_VALID) == 0u);
#endif
}

#if ARGHTOS_R6AF_UNCONDITIONAL_CRASH_TASK_ID
static void test_crash_attribution_with_current_task(void)
{
    const arghtos_task_control_t *ctrl;
    jmp_buf env;

    reset_all();
    assert(arghtos_init(NULL) == ARGHTOS_OK);
    start_and_escape();

    ctrl = arghtos_test_controls();
    assert(arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE) == ctrl[0].context);
    assert(arghtos_test_kernel()->current_task == 0u);

    arghtos_host_set_fault_jmp(&env);
    if (setjmp(env) == 0) {
        arghtos_fatal(ARGHTOS_FAULT_KERNEL_INVARIANT, NULL);
        assert(!"fatal returned");
    }
    arghtos_host_set_fault_jmp(NULL);

    assert(arghtos_static_crash_record.commit_magic ==
           ARGHTOS_CRASH_COMMIT_MAGIC);
    assert(arghtos_static_crash_record.task_id == 0u);
    assert((arghtos_static_crash_record.flags &
            ARGHTOS_CRASH_F_TASK_VALID) != 0u);
#if ARGHTOS_R6Z_TRUSTED_CRASH_PAYLOAD
    assert((arghtos_static_crash_record.flags &
            ARGHTOS_CRASH_F_PORT_STATE_VALID) != 0u);
#endif
}
#endif

#if ARGHTOS_COMPACT_RUNTIME
static void test_compact_runtime_saturation(void)
{
    const arghtos_task_control_t *ctrl;
#if ARGHTOS_RUNTIME_PROFILING
    arghtos_task_profile_t p;
#endif
    arghtos_context_t ctx;

    reset_all();
    arghtos_host_set_time(0u);
    assert(arghtos_init(NULL) == ARGHTOS_OK);
    start_and_escape();

    ctrl = arghtos_test_controls();
    ctx = arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE);
    assert(ctx == ctrl[0].context);

    arghtos_host_set_time(70000u);
    ctx = arghtos_kernel_dispatch(ctx);
    assert(ctx == ctrl[0].context);

#if ARGHTOS_RUNTIME_PROFILING
    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    assert(arghtos_profile_get(0u, &p) == ARGHTOS_OK);
    assert(p.max_run_ticks == UINT16_MAX);
#endif

    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
#if ARGHTOS_R6X_BOUNDED_MISSED_LOOP || ARGHTOS_R6Y_PACKED_MISSED_ADD
    arghtos_host_set_time(3u);
    arghtos_host_clear_dispatch_requests();
    arghtos_timer_interrupt();
    assert(ARGHTOS_CTRL_MISSED(&ctrl[0]) == 3u);
    assert(ARGHTOS_CTRL_STATE(&ctrl[0]) == ARGHTOS_TASK_RUNNING);
    assert(arghtos_host_dispatch_request_count() == 1u);
#endif
    arghtos_host_set_time(1000u);
#if ARGHTOS_R6V_ALWAYS_DISPATCH_TICK
    arghtos_host_clear_dispatch_requests();
#endif
    arghtos_timer_interrupt();
    assert(ARGHTOS_CTRL_MISSED(&ctrl[0]) == ARGHTOS_CTRL_MISSED_MAX);
#if ARGHTOS_R6V_ALWAYS_DISPATCH_TICK
    assert(arghtos_host_dispatch_request_count() == 1u);
#if ARGHTOS_R6W_PERIODIC_READY_NORMALIZE
    assert(ARGHTOS_CTRL_STATE(&ctrl[0]) == ARGHTOS_TASK_READY);
    ctx = arghtos_kernel_dispatch(ctx);
    assert(ctx == ctrl[0].context);
    assert(ARGHTOS_CTRL_STATE(&ctrl[0]) == ARGHTOS_TASK_RUNNING);
#else
    assert(ARGHTOS_CTRL_STATE(&ctrl[0]) == ARGHTOS_TASK_RUNNING);
#endif
#endif
}
#endif

#if ARGHTOS_R6AE_DISPATCH_OWNS_YIELD_READY
static void test_deferred_yield_ready_owned_by_dispatch(void)
{
    const arghtos_task_control_t *c;
    arghtos_context_t ctx;

    reset_all();
    assert(arghtos_init(NULL) == ARGHTOS_OK);
    start_and_escape();

    c = arghtos_test_controls();
    ctx = arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE);
    assert(ctx == c[0].context);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_RUNNING);

    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    arghtos_host_clear_dispatch_requests();
    assert(arghtos_port_service_call(ARGHTOS_SERVICE_YIELD, 0u) == ARGHTOS_OK);

    /*
     * The host seam returns from the service directly instead of Cortex-M23
     * exception tail-chaining. R6ae intentionally leaves the task RUNNING
     * until the immediately-owned PendSV dispatcher saves its context.
     */
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_RUNNING);
    assert(arghtos_host_dispatch_request_count() == 1u);

    ctx = arghtos_kernel_dispatch(ctx);
    assert(ctx == c[0].context);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_RUNNING);
    assert(arghtos_test_kernel()->current_task == 0u);
}
#endif

#if ARGHTOS_R6AG_TIMER_OWNS_SLEEP_RELEASE
static void test_sleep_time_processing_owned_by_timer(void)
{
    const arghtos_task_control_t *ctrl;
    arghtos_context_t periodic_ctx;
    arghtos_context_t async_ctx;
    arghtos_time_t old_periodic_next;
    uint32_t old_arm_count;

    reset_all();
    arghtos_host_set_time(0u);
    assert(arghtos_init(NULL) == ARGHTOS_OK);
    start_and_escape();

    ctrl = arghtos_test_controls();
    periodic_ctx = arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE);
    assert(periodic_ctx == ctrl[0].context);
    assert(ARGHTOS_CTRL_STATE(&ctrl[0]) == ARGHTOS_TASK_RUNNING);

    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    assert(arghtos_wake_from_interrupt(1u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&ctrl[1]) == ARGHTOS_TASK_READY);

    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    assert(arghtos_port_service_call(
        ARGHTOS_SERVICE_TASK_RETURN, 0u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&ctrl[0]) == ARGHTOS_TASK_DORMANT);

    async_ctx = arghtos_kernel_dispatch(periodic_ctx);
    assert(async_ctx == ctrl[1].context);
    assert(arghtos_test_kernel()->current_task == 1u);
    assert(ARGHTOS_CTRL_STATE(&ctrl[1]) == ARGHTOS_TASK_RUNNING);

    old_periodic_next = ARGHTOS_CTRL_NEXT(&ctrl[0]);
    old_arm_count = arghtos_host_timer_arm_count();

    /*
     * Simulate scheduler-timer delivery lateness. R6ag must not opportunistically
     * process the overdue periodic release from SVC.
     */
    arghtos_host_set_time(10u);
    arghtos_host_clear_dispatch_requests();
    assert(arghtos_sleep_until(20u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&ctrl[1]) == ARGHTOS_TASK_SLEEPING);
    assert(ARGHTOS_CTRL_NEXT(&ctrl[1]) == 20u);
    assert(ARGHTOS_CTRL_STATE(&ctrl[0]) == ARGHTOS_TASK_DORMANT);
    assert(ARGHTOS_CTRL_NEXT(&ctrl[0]) == old_periodic_next);
    assert(arghtos_host_timer_arm_count() == old_arm_count);
    assert(arghtos_host_dispatch_request_count() == 1u);

    /* The timer interrupt now owns delayed periodic catch-up. */
    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    arghtos_host_clear_dispatch_requests();
    arghtos_timer_interrupt();
    assert(ARGHTOS_CTRL_STATE(&ctrl[0]) == ARGHTOS_TASK_READY);
    assert(ARGHTOS_CTRL_NEXT(&ctrl[0]) == 11u);
    assert(ARGHTOS_CTRL_STATE(&ctrl[1]) == ARGHTOS_TASK_SLEEPING);
    assert(arghtos_host_timer_deadline() == 11u);
    assert(arghtos_host_dispatch_request_count() == 1u);

    /* A delayed later timer service still expires the absolute sleep deadline. */
    arghtos_host_set_time(20u);
    arghtos_host_clear_dispatch_requests();
    arghtos_timer_interrupt();
    assert(ARGHTOS_CTRL_STATE(&ctrl[1]) == ARGHTOS_TASK_READY);
    assert(arghtos_host_dispatch_request_count() == 1u);
}
#endif

#if ARGHTOS_R6AJ_DIRECT_ASYNC_PREBUILT_ENTRY
static void test_direct_async_return_guard_reason(void)
{
    jmp_buf env;

    reset_all();
    assert(arghtos_init(NULL) == ARGHTOS_OK);
    start_and_escape();

    arghtos_host_set_fault_jmp(&env);
    if (setjmp(env) == 0) {
        arghtos_task_return_guard();
        assert(!"async return guard returned");
    }
    arghtos_host_set_fault_jmp(NULL);

    assert(arghtos_static_crash_record.reason ==
           ARGHTOS_FAULT_ASYNC_RETURNED);
    assert(arghtos_static_crash_record.commit_magic ==
           ARGHTOS_CRASH_COMMIT_MAGIC);
}
#endif

#if ARGHTOS_R6M_COMMIT_RECURSION_GUARD
static void test_recursive_fatal_uses_commit_guard(void)
{
    jmp_buf env;

    reset_all();
    assert(arghtos_init(NULL) == ARGHTOS_OK);
    start_and_escape();

    assert(arghtos_static_crash_record.commit_magic == 0u);
    arghtos_host_set_recursive_fault_on_crash_fill(true);
    arghtos_host_set_fault_jmp(&env);
    if (setjmp(env) == 0) {
        arghtos_fatal(ARGHTOS_FAULT_KERNEL_INVARIANT, NULL);
        assert(!"fatal returned");
    }
    arghtos_host_set_fault_jmp(NULL);

    assert(arghtos_host_crash_fill_count() == 1u);
    assert(arghtos_static_crash_record.commit_magic ==
           ARGHTOS_R6M_CAPTURE_IN_PROGRESS);
}
#endif

#if ARGHTOS_R6O_PRESEEDED_CURRENT_INVALID
static void test_preseeded_kernel_current_invalid(void)
{
    assert(arghtos_test_kernel()->current_task == ARGHTOS_TASK_ID_INVALID);
#if !ARGHTOS_R6R_FLAGLESS_KERNEL_STATE
#if ARGHTOS_R6P_INFALLIBLE_PORT_INIT
    assert(arghtos_test_kernel()->flags == ARGHTOS_KF_INITIALIZED);
#else
    assert(arghtos_test_kernel()->flags == 0u);
#endif
#endif
}
#endif

int main(void)
{
#if ARGHTOS_R6O_PRESEEDED_CURRENT_INVALID
    test_preseeded_kernel_current_invalid();
#endif
    test_static_init_ignores_runtime_config();
    test_static_dispatch_and_async_wake();
    test_static_crash_record_symbol_is_used();
#if ARGHTOS_R6AF_UNCONDITIONAL_CRASH_TASK_ID
    test_crash_attribution_with_current_task();
#endif
#if ARGHTOS_R6AE_DISPATCH_OWNS_YIELD_READY
    test_deferred_yield_ready_owned_by_dispatch();
#endif
#if ARGHTOS_R6AG_TIMER_OWNS_SLEEP_RELEASE
    test_sleep_time_processing_owned_by_timer();
#endif
#if ARGHTOS_R6AJ_DIRECT_ASYNC_PREBUILT_ENTRY
    test_direct_async_return_guard_reason();
#endif
#if ARGHTOS_R6M_COMMIT_RECURSION_GUARD
    test_recursive_fatal_uses_commit_guard();
#endif
#if ARGHTOS_COMPACT_RUNTIME
    test_compact_runtime_saturation();
#endif

    puts("arghtos static-profile host tests: PASS");
    return 0;
}
