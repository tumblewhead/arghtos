#include <arghtos/arghtos.h>
#include "../src/arghtos_kernel_port.h"
#include "../port/host/arghtos_host_port.h"
#include "arghtos_test.h"

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

static _Alignas(8) uint8_t stack0[256];
static _Alignas(8) uint8_t stack1[256];

static void periodic_task(void) {}
static void async_task(void) {}

#if ARGHTOS_COMPACT_DESCRIPTOR
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
    arghtos_test_reset();
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
    start_and_escape();

    c = arghtos_test_controls();
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_READY);
    assert(c[0].next_time == 110u);
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

    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    assert(arghtos_port_service_call(
        ARGHTOS_SERVICE_TASK_RETURN, 0u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_DORMANT);

    assert(arghtos_kernel_dispatch(periodic_context) == c[1].context);
    assert(arghtos_test_kernel()->current_task == 1u);
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
    arghtos_task_profile_t p;
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

    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    assert(arghtos_profile_get(0u, &p) == ARGHTOS_OK);
    assert(p.max_run_ticks == UINT16_MAX);

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
