#include <arghtos/arghtos.h>
#include "../src/arghtos_kernel_port.h"
#include "../port/host/arghtos_host_port.h"
#include "arghtos_test.h"

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

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
        .entry_or_next.next_time = 0u,
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
    10u,
    ARGHTOS_R5I_ASYNC_BASE | 3u
};
#endif
#endif
#if ARGHTOS_SPLIT_META
arghtos_task_meta_t arghtos_static_meta[ARGHTOS_MAX_TASKS];
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

arghtos_crash_record_t arghtos_static_crash_record;

static void reset_all(void)
{
#if ARGHTOS_R5L_PREBUILT_CONTEXT
    /* Entry pointers live in the generated target stack image in this profile. */
    (void)periodic_task;
    (void)async_task;
#endif
    arghtos_test_reset();
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
    memset(&arghtos_static_crash_record, 0, sizeof(arghtos_static_crash_record));
}

static void start_and_escape(void)
{
    jmp_buf env;

    arghtos_host_set_start_jmp(&env);
    if (setjmp(env) == 0) {
        arghtos_start();
        assert(!"arghtos_start returned");
    }
    arghtos_host_set_start_jmp(NULL);
}

static void test_static_init_ignores_runtime_config(void)
{
    const arghtos_task_control_t *c;

    reset_all();
    arghtos_host_set_time(100u);

    assert(arghtos_init(NULL) == ARGHTOS_OK);
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
    assert(ARGHTOS_CTRL_NEXT(&c[0]) == 110u);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_DORMANT);
    assert(arghtos_host_timer_is_armed());
    assert(arghtos_host_timer_deadline() == 110u);
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
    assert(arghtos_wake_from_interrupt(1u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_READY);
    assert(ARGHTOS_CTRL_AGE(&c[1]) == 0u);

    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    assert(arghtos_port_service_call(
        ARGHTOS_SERVICE_TASK_RETURN, 0u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_DORMANT);

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
}

static void test_static_crash_record_symbol_is_used(void)
{
    jmp_buf env;

    reset_all();
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
}

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
    arghtos_host_set_time(1000u);
    arghtos_timer_interrupt();
    assert(ARGHTOS_CTRL_MISSED(&ctrl[0]) == ARGHTOS_CTRL_MISSED_MAX);
}
#endif

int main(void)
{
    test_static_init_ignores_runtime_config();
    test_static_dispatch_and_async_wake();
    test_static_crash_record_symbol_is_used();
#if ARGHTOS_COMPACT_RUNTIME
    test_compact_runtime_saturation();
#endif

    puts("arghtos static-profile host tests: PASS");
    return 0;
}
