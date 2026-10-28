#include <arghtos/arghtos_port.h>
#include "../../src/arghtos_kernel_port.h"
#include "arghtos_host_port.h"

#include <stdlib.h>

static arghtos_host_mode_t host_mode = ARGHTOS_HOST_STARTUP;
static arghtos_time_t host_now;
static bool host_timer_armed;
static arghtos_time_t host_timer_deadline_value;
static uint32_t host_arm_count;
static uint32_t host_disarm_count;
static uint32_t host_dispatch_requests;
static bool host_masked;
static uint32_t host_idle_calls;
static jmp_buf *host_start_env;
static jmp_buf *host_fault_env;

void arghtos_host_reset(void)
{
    host_mode = ARGHTOS_HOST_STARTUP;
    host_now = 0u;
    host_timer_armed = false;
    host_timer_deadline_value = 0u;
    host_arm_count = 0u;
    host_disarm_count = 0u;
    host_dispatch_requests = 0u;
    host_masked = false;
    host_idle_calls = 0u;
    host_start_env = NULL;
    host_fault_env = NULL;
}

void arghtos_host_set_mode(arghtos_host_mode_t mode) { host_mode = mode; }
arghtos_host_mode_t arghtos_host_mode(void) { return host_mode; }
void arghtos_host_set_time(arghtos_time_t now) { host_now = now; }
void arghtos_host_advance_time(arghtos_duration_t dt) { host_now += dt; }
bool arghtos_host_timer_is_armed(void) { return host_timer_armed; }
arghtos_time_t arghtos_host_timer_deadline(void) { return host_timer_deadline_value; }
uint32_t arghtos_host_timer_arm_count(void) { return host_arm_count; }
uint32_t arghtos_host_timer_disarm_count(void) { return host_disarm_count; }
uint32_t arghtos_host_dispatch_request_count(void) { return host_dispatch_requests; }
void arghtos_host_clear_dispatch_requests(void) { host_dispatch_requests = 0u; }
bool arghtos_host_interrupts_masked(void) { return host_masked; }
uint32_t arghtos_host_idle_count(void) { return host_idle_calls; }
void arghtos_host_set_start_jmp(jmp_buf *env) { host_start_env = env; }
void arghtos_host_set_fault_jmp(jmp_buf *env) { host_fault_env = env; }

arghtos_result_t arghtos_port_init(void)
{
    host_timer_armed = false;
    host_mode = ARGHTOS_HOST_STARTUP;
    return ARGHTOS_OK;
}

bool arghtos_port_stack_valid(void *stack_base, uint32_t stack_size)
{
    const uintptr_t base = (uintptr_t)stack_base;
    return (stack_base != NULL) &&
           (stack_size >= 16u) &&
           ((base & 0x7u) == 0u) &&
           ((uintptr_t)stack_size <= UINTPTR_MAX - base);
}

arghtos_time_t arghtos_port_time_now(void) { return host_now; }

void arghtos_port_timer_arm(arghtos_time_t absolute_deadline)
{
    host_timer_armed = true;
    host_timer_deadline_value = absolute_deadline;
    host_arm_count++;
}

void arghtos_port_timer_disarm(void)
{
    host_timer_armed = false;
    host_disarm_count++;
}

arghtos_irq_key_t arghtos_port_critical_enter(void)
{
    const arghtos_irq_key_t old = host_masked ? 1u : 0u;
    host_masked = true;
    return old;
}

void arghtos_port_critical_exit(arghtos_irq_key_t key)
{
    host_masked = key != 0u;
}

bool arghtos_port_is_task_context(void)
{
    return host_mode == ARGHTOS_HOST_TASK;
}

bool arghtos_port_is_handler_context(void)
{
    return host_mode == ARGHTOS_HOST_HANDLER;
}

arghtos_result_t arghtos_port_service_call(uint8_t service_id, uint32_t arg0)
{
    return arghtos_kernel_service(service_id, arg0);
}

void arghtos_port_request_dispatch(void)
{
    host_dispatch_requests++;
}

ARGHTOS_NORETURN void arghtos_port_start(void)
{
    host_masked = false;
    if (host_start_env != NULL) {
        longjmp(*host_start_env, 1);
    }
    abort();
}

void arghtos_port_idle_wait(void)
{
    host_idle_calls++;
}

arghtos_context_t arghtos_port_build_initial_context(
    void *stack_base,
    uint32_t stack_size,
    void (*thread_start)(void),
    void (*return_guard)(void))
{
    uintptr_t top;
    (void)thread_start;
    (void)return_guard;

    if (!arghtos_port_stack_valid(stack_base, stack_size)) {
        return ARGHTOS_CONTEXT_NONE;
    }

    top = (uintptr_t)stack_base + stack_size;
    top &= ~(uintptr_t)0x7u;
    return top == 0u ? ARGHTOS_CONTEXT_NONE : (arghtos_context_t)top;
}

void arghtos_port_crash_fill(
    arghtos_crash_record_t *record,
    const void *port_fault_context)
{
    (void)port_fault_context;
    record->port_format = 0x484Fu;
    record->port_words = 1u;
    record->port_state[0] = (uint32_t)host_mode;
}

ARGHTOS_NORETURN void arghtos_port_fault_terminal(void)
{
    if (host_fault_env != NULL) {
        longjmp(*host_fault_env, 1);
    }
    abort();
}
