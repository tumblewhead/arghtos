#include <arghtos/arghtos.h>
#include "../src/arghtos_kernel_port.h"
#include "../port/host/arghtos_host_port.h"
#include "arghtos_test.h"

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>

static _Alignas(8) uint8_t stack0[256];
static _Alignas(8) uint8_t stack1[256];
static _Alignas(8) uint8_t stack2[256];
static arghtos_crash_record_t crash_record;

static void task0(void) {}
static void task1(void) {}
static void task2(void) {}

static void reset_all(void)
{
    arghtos_test_reset();
    arghtos_host_reset();
    crash_record.magic = 0u;
    crash_record.commit_magic = 0u;
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

static arghtos_config_t make_config(
    const arghtos_task_config_t *tasks,
    uint8_t count)
{
    arghtos_config_t cfg;
    cfg.tasks = tasks;
    cfg.crash_record = &crash_record;
    cfg.task_count = count;
    cfg.reserved[0] = 0u;
    cfg.reserved[1] = 0u;
    cfg.reserved[2] = 0u;
    return cfg;
}

static void test_public_fixed_sizes(void)
{
    assert(sizeof(arghtos_crash_record_t) == 64u);
    assert(sizeof(arghtos_task_profile_t) == 12u);
}

#if !ARGHTOS_TRUSTED_CONFIG
static void test_init_rejects_overlap(void)
{
    arghtos_task_config_t tasks[2] = {
        {task0, stack0, 10u, 128u, 0u, 0u},
        {task1, stack0 + 64u, 20u, 128u, 0u, 0u}
    };
    arghtos_config_t cfg = make_config(tasks, 2u);

    reset_all();
    assert(arghtos_init(&cfg) == ARGHTOS_ERR_BAD_CONFIG);
    assert((arghtos_test_kernel()->flags & ARGHTOS_KF_INITIALIZED) == 0u);
}

#endif

static void test_prestart_interrupt_wake_is_rejected(void)
{
    arghtos_task_config_t task = {
        task0, stack0, 0u, sizeof(stack0), 1u, 0u
    };
    arghtos_config_t cfg = make_config(&task, 1u);

    reset_all();
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    assert(arghtos_wake_from_interrupt(0u) == ARGHTOS_ERR_BAD_STATE);
    assert(ARGHTOS_CTRL_STATE(&arghtos_test_controls()[0]) == ARGHTOS_TASK_DORMANT);
    assert(arghtos_host_dispatch_request_count() == 0u);
}

static void test_async_priority_sentinels_rejected(void)
{
#if !ARGHTOS_TRUSTED_CONFIG
    arghtos_task_config_t task = {
        task0, stack0, 0u, sizeof(stack0), ARGHTOS_PRIORITY_NONE, 0u
    };
    arghtos_config_t cfg = make_config(&task, 1u);

    reset_all();
    assert(arghtos_init(&cfg) == ARGHTOS_ERR_BAD_CONFIG);

    task.async_priority = ARGHTOS_PRIORITY_INVALID;
    reset_all();
    assert(arghtos_init(&cfg) == ARGHTOS_ERR_BAD_CONFIG);

    task.async_priority = (uint8_t)ARGHTOS_ASYNC_PRIORITY_MAX;
    reset_all();
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
#endif
}

static void test_start_establishes_normal_interrupt_state(void)
{
    arghtos_task_config_t task = {
        task0, stack0, 0u, sizeof(stack0), 1u, 0u
    };
    arghtos_config_t cfg = make_config(&task, 1u);

    reset_all();
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
    (void)arghtos_port_critical_enter();
    assert(arghtos_host_interrupts_masked());
    start_and_escape();
    assert(!arghtos_host_interrupts_masked());
}

static void test_start_releases_all_periodic_at_one_epoch(void)
{
    arghtos_task_config_t tasks[3] = {
        {task0, stack0, 100u, sizeof(stack0), 0u, 0u},
        {task1, stack1, 10u,  sizeof(stack1), 0u, 0u},
        {task2, stack2, 0u,   sizeof(stack2), 3u, 0u}
    };
    arghtos_config_t cfg = make_config(tasks, 3u);
    const arghtos_task_control_t *c;

    reset_all();
    arghtos_host_set_time(1000u);
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
    start_and_escape();
    assert(!arghtos_host_interrupts_masked());

    c = arghtos_test_controls();
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_READY);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_READY);
    assert(ARGHTOS_CTRL_STATE(&c[2]) == ARGHTOS_TASK_DORMANT);
    assert(c[0].next_time == 1100u);
    assert(c[1].next_time == 1010u);
    assert(arghtos_host_timer_is_armed());
    assert(arghtos_host_timer_deadline() == 1010u);
}

static void test_periodic_rms_and_same_task_reselect_accounting(void)
{
    arghtos_task_config_t tasks[2] = {
        {task0, stack0, 100u, sizeof(stack0), 0u, 0u},
        {task1, stack1, 10u,  sizeof(stack1), 0u, 0u}
    };
    arghtos_config_t cfg = make_config(tasks, 2u);
    arghtos_context_t ctx;
    const arghtos_task_control_t *c;

    reset_all();
    arghtos_host_set_time(50u);
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
    start_and_escape();

    ctx = arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE);
    assert(arghtos_test_kernel()->current_task == 1u);
    assert(ctx == arghtos_test_controls()[1].context);

    arghtos_host_advance_time(5u);
    ctx = arghtos_kernel_dispatch(ctx);
    c = arghtos_test_controls();
    assert(arghtos_test_kernel()->current_task == 1u);
    assert(c[1].total_cpu_ticks == 5u);
    assert(ARGHTOS_CTRL_MAX_RUN(&c[1]) == 5u);
    assert(ARGHTOS_CTRL_STATE(&c[1]) == ARGHTOS_TASK_RUNNING);
}

static void setup_one_async_at(arghtos_time_t now)
{
    static arghtos_task_config_t task = {
        task0, stack0, 0u, sizeof(stack0), 1u, 0u
    };
    arghtos_config_t cfg = make_config(&task, 1u);

    reset_all();
    arghtos_host_set_time(now);
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
    start_and_escape();
    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    assert(arghtos_wake_from_interrupt(0u) == ARGHTOS_OK);
    assert(arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE) != ARGHTOS_CONTEXT_NONE);
    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    arghtos_host_clear_dispatch_requests();
}

static void setup_one_async(void)
{
    setup_one_async_at(0u);
}

static void test_wake_wait_handshake(void)
{
    const arghtos_task_control_t *c;
    arghtos_context_t ctx;

    setup_one_async();
    c = arghtos_test_controls();
    ctx = c[0].context;

    assert(arghtos_wake(0u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_WAKE_PENDING(&c[0]));

    assert(arghtos_wait() == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_RUNNING);
    assert(!ARGHTOS_CTRL_WAKE_PENDING(&c[0]));
    assert(arghtos_host_dispatch_request_count() == 0u);

    assert(arghtos_wait() == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_WAITING);
    assert(arghtos_host_dispatch_request_count() == 1u);

    assert(arghtos_kernel_dispatch(ctx) == ARGHTOS_CONTEXT_NONE);
    assert(arghtos_test_kernel()->current_task == ARGHTOS_TASK_ID_INVALID);

    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    assert(arghtos_wake_from_interrupt(0u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_READY);
    assert(arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE) == c[0].context);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_RUNNING);
}

static void test_sleep_is_timer_only_and_preserves_pending_wake(void)
{
    const arghtos_task_control_t *c;
    arghtos_context_t ctx;

    setup_one_async();
    c = arghtos_test_controls();
    ctx = c[0].context;
    arghtos_host_set_time(100u);

    assert(arghtos_sleep_for(10u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_SLEEPING);
    assert(c[0].next_time == 110u);
    assert(arghtos_host_timer_deadline() == 110u);

    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    assert(arghtos_wake_from_interrupt(0u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_SLEEPING);
    assert(ARGHTOS_CTRL_WAKE_PENDING(&c[0]));

    assert(arghtos_kernel_dispatch(ctx) == ARGHTOS_CONTEXT_NONE);
    arghtos_host_set_time(109u);
    arghtos_timer_interrupt();
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_SLEEPING);

    arghtos_host_set_time(110u);
    arghtos_timer_interrupt();
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_READY);
    assert(arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE) == c[0].context);

    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    arghtos_host_clear_dispatch_requests();
    assert(arghtos_wait() == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_RUNNING);
    assert(!ARGHTOS_CTRL_WAKE_PENDING(&c[0]));
}

static void test_sleep_wraparound(void)
{
    const arghtos_task_control_t *c;

    setup_one_async();
    c = arghtos_test_controls();
    arghtos_host_set_time(0xFFFFFFF0u);
    assert(arghtos_sleep_until(0x00000010u) == ARGHTOS_OK);
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_SLEEPING);
    assert(arghtos_host_timer_deadline() == 0x00000010u);
}

static void test_periodic_delayed_release_catchup(void)
{
    arghtos_task_config_t task = {
        task0, stack0, 10u, sizeof(stack0), 0u, 0u
    };
    arghtos_config_t cfg = make_config(&task, 1u);
    const arghtos_task_control_t *c;
    arghtos_context_t ctx;

    reset_all();
    arghtos_host_set_time(100u);
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
    start_and_escape();
    ctx = arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE);

    arghtos_host_set_mode(ARGHTOS_HOST_TASK);
    assert(arghtos_port_service_call(ARGHTOS_SERVICE_TASK_RETURN, 0u) == ARGHTOS_OK);
    assert(arghtos_test_controls()ARGHTOS_CTRL_STATE(&[0]) == ARGHTOS_TASK_DORMANT);
    assert(arghtos_kernel_dispatch(ctx) == ARGHTOS_CONTEXT_NONE);

    arghtos_host_set_time(137u);
    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    arghtos_timer_interrupt();
    c = arghtos_test_controls();
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_READY);
    assert(c[0].next_time == 140u);
    assert(ARGHTOS_CTRL_MISSED(&c[0]) == 2u);
}

static void test_periodic_active_counts_all_missed_releases(void)
{
    arghtos_task_config_t task = {
        task0, stack0, 10u, sizeof(stack0), 0u, 0u
    };
    arghtos_config_t cfg = make_config(&task, 1u);
    const arghtos_task_control_t *c;

    reset_all();
    arghtos_host_set_time(100u);
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
    start_and_escape();
    c = arghtos_test_controls();
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_READY);

    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    arghtos_host_set_time(137u);
    arghtos_timer_interrupt();
    assert(ARGHTOS_CTRL_STATE(&c[0]) == ARGHTOS_TASK_READY);
    assert(c[0].next_time == 140u);
    assert(ARGHTOS_CTRL_MISSED(&c[0]) == 3u);
}

static void test_async_aging_eventually_beats_high_base_peer(void)
{
    arghtos_task_config_t tasks[2] = {
        {task0, stack0, 0u, sizeof(stack0), 1u, 0u},
        {task1, stack1, 0u, sizeof(stack1),
            (uint8_t)ARGHTOS_ASYNC_PRIORITY_MAX, 0u}
    };
    arghtos_config_t cfg = make_config(tasks, 2u);
    arghtos_context_t ctx;
    unsigned i;
    bool low_ran = false;

    reset_all();
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
    start_and_escape();
    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    assert(arghtos_wake_from_interrupt(0u) == ARGHTOS_OK);
    assert(arghtos_wake_from_interrupt(1u) == ARGHTOS_OK);

    ctx = arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE);
    assert(arghtos_test_kernel()->current_task == 1u);

    for (i = 0u; i < 32u; ++i) {
        arghtos_host_advance_time(1u);
        ctx = arghtos_kernel_dispatch(ctx);
        if (arghtos_test_kernel()->current_task == 0u) {
            low_ran = true;
            break;
        }
    }
    assert(low_ran);
}

static void test_periodic_selection_does_not_age_async(void)
{
    arghtos_task_config_t tasks[2] = {
        {task0, stack0, 10u, sizeof(stack0), 0u, 0u},
        {task1, stack1, 0u, sizeof(stack1), 1u, 0u}
    };
    arghtos_config_t cfg = make_config(tasks, 2u);

    reset_all();
    arghtos_host_set_time(0u);
    assert(arghtos_init(&cfg) == ARGHTOS_OK);
    start_and_escape();
    arghtos_host_set_mode(ARGHTOS_HOST_HANDLER);
    assert(arghtos_wake_from_interrupt(1u) == ARGHTOS_OK);
    assert(arghtos_test_controls()[1].age_credit == 0u);

    assert(arghtos_kernel_dispatch(ARGHTOS_CONTEXT_NONE) != ARGHTOS_CONTEXT_NONE);
    assert(arghtos_test_kernel()->current_task == 0u);
    assert(arghtos_test_controls()[1].age_credit == 0u);
}

static void test_profile_snapshot_is_committed_only(void)
{
    arghtos_task_profile_t p;
    arghtos_context_t ctx;

    setup_one_async_at(100u);
    ctx = arghtos_test_controls()[0].context;

    assert(arghtos_profile_get(0u, &p) == ARGHTOS_OK);
    assert(p.total_cpu_ticks == 0u);

    arghtos_host_set_time(125u);
    assert(arghtos_profile_get(0u, &p) == ARGHTOS_OK);
    assert(p.total_cpu_ticks == 0u);

    (void)arghtos_kernel_dispatch(ctx);
    assert(arghtos_profile_get(0u, &p) == ARGHTOS_OK);
    assert(p.total_cpu_ticks == 25u);
    assert(p.max_run_ticks == 25u);
}

static void test_invalid_wake_id_does_not_mutate(void)
{
    arghtos_task_control_t before;
    const arghtos_task_control_t *c;

    setup_one_async();
    c = arghtos_test_controls();
    before = c[0];
    assert(arghtos_wake(99u) == ARGHTOS_ERR_BAD_TASK_ID);
    assert(c[0].context == before.context);
    assert(c[0].state == before.state);
    assert(c[0].flags == before.flags);
    assert(c[0].age_credit == before.age_credit);
}

static void test_fatal_commits_generic_crash_record(void)
{
    jmp_buf env;

    setup_one_async();
    arghtos_host_set_fault_jmp(&env);
    if (setjmp(env) == 0) {
        arghtos_fatal(ARGHTOS_FAULT_ASYNC_RETURNED, NULL);
        assert(!"fatal returned");
    }
    arghtos_host_set_fault_jmp(NULL);

    assert(crash_record.magic == ARGHTOS_CRASH_MAGIC);
    assert(crash_record.version == ARGHTOS_CRASH_VERSION);
    assert(crash_record.size == 64u);
    assert(crash_record.reason == ARGHTOS_FAULT_ASYNC_RETURNED);
    assert(crash_record.task_id == 0u);
    assert((crash_record.flags & ARGHTOS_CRASH_F_TASK_VALID) != 0u);
    assert((crash_record.flags & ARGHTOS_CRASH_F_PORT_STATE_VALID) != 0u);
    assert(crash_record.port_words == 1u);
    assert(crash_record.commit_magic == ARGHTOS_CRASH_COMMIT_MAGIC);
    assert(arghtos_test_controls()ARGHTOS_CTRL_STATE(&[0]) == ARGHTOS_TASK_FAULTED);
}

int main(void)
{
    test_public_fixed_sizes();
#if !ARGHTOS_TRUSTED_CONFIG
    test_init_rejects_overlap();
#endif
    test_prestart_interrupt_wake_is_rejected();
    test_async_priority_sentinels_rejected();
    test_start_establishes_normal_interrupt_state();
    test_start_releases_all_periodic_at_one_epoch();
    test_periodic_rms_and_same_task_reselect_accounting();
    test_wake_wait_handshake();
    test_sleep_is_timer_only_and_preserves_pending_wake();
    test_sleep_wraparound();
    test_periodic_delayed_release_catchup();
    test_periodic_active_counts_all_missed_releases();
    test_async_aging_eventually_beats_high_base_peer();
    test_periodic_selection_does_not_age_async();
    test_profile_snapshot_is_committed_only();
    test_invalid_wake_id_does_not_mutate();
    test_fatal_commits_generic_crash_record();

    puts("arghtos host kernel tests: PASS");
    return 0;
}
