#include "arghtos_port_private.h"

#ifndef ARGHTOS_R5K_ASM_CRASH_FILL
#define ARGHTOS_R5K_ASM_CRASH_FILL 0
#endif

static inline uint32_t m23_read_primask(void)
{
    uint32_t v;
    __asm volatile("mrs %0, primask" : "=r"(v));
    return v;
}

static inline void m23_write_primask(uint32_t v)
{
    __asm volatile("msr primask, %0" :: "r"(v) : "memory");
}

static inline uint32_t m23_read_ipsr(void)
{
    uint32_t v;
    __asm volatile("mrs %0, ipsr" : "=r"(v));
    return v;
}

static inline uint32_t m23_read_control(void)
{
    uint32_t v;
    __asm volatile("mrs %0, control" : "=r"(v));
    return v;
}

#if !ARGHTOS_R5K_ASM_CRASH_FILL
static inline uint32_t m23_read_msp(void)
{
    uint32_t v;
    __asm volatile("mrs %0, msp" : "=r"(v));
    return v;
}

static inline uint32_t m23_read_psp(void)
{
    uint32_t v;
    __asm volatile("mrs %0, psp" : "=r"(v));
    return v;
}
#endif

arghtos_result_t arghtos_port_init(void)
{
    volatile uint8_t *const pendsv_priority =
        (volatile uint8_t *)ARGHTOS_M23_SHPR3_PENDSV_ADDR;
    volatile uint32_t *const icsr =
        (volatile uint32_t *)ARGHTOS_M23_SCB_ICSR_ADDR;

    *pendsv_priority = 0xFFu;
    *icsr = ARGHTOS_M23_ICSR_PENDSVCLR;
    arghtos_m23_board_timer_disarm();
    return ARGHTOS_OK;
}

bool arghtos_port_stack_valid(void *stack_base, uint32_t stack_size)
{
    const uintptr_t base = (uintptr_t)stack_base;
    uintptr_t end;
    uintptr_t top;

    if ((stack_base == NULL) ||
        (stack_size < ARGHTOS_M23_MIN_CONTEXT_RESERVE) ||
        ((base & 0x3u) != 0u) ||
        ((uintptr_t)stack_size > UINTPTR_MAX - base)) {
        return false;
    }

    end = base + stack_size;
    top = end & ~(uintptr_t)0x7u;
    return (top >= base) &&
           ((top - base) >= ARGHTOS_M23_MIN_CONTEXT_RESERVE);
}

arghtos_time_t arghtos_port_time_now(void)
{
    return arghtos_m23_board_time_now();
}

void arghtos_port_timer_arm(arghtos_time_t absolute_deadline)
{
    arghtos_m23_board_timer_arm(absolute_deadline);
}

void arghtos_port_timer_disarm(void)
{
    arghtos_m23_board_timer_disarm();
}

arghtos_irq_key_t arghtos_port_critical_enter(void)
{
    const uint32_t old = m23_read_primask();
    __asm volatile("cpsid i" ::: "memory");
    return old;
}

void arghtos_port_critical_exit(arghtos_irq_key_t key)
{
    m23_write_primask((uint32_t)key);
}

bool arghtos_port_is_task_context(void)
{
    return (m23_read_ipsr() == 0u) &&
           ((m23_read_control() & ARGHTOS_M23_CONTROL_SPSEL) != 0u);
}

bool arghtos_port_is_handler_context(void)
{
    const uint32_t ipsr = m23_read_ipsr();

    if (ipsr == 0u) {
        return false;
    }

    return (ipsr != 1u) && (ipsr != 2u) && (ipsr != 3u) &&
           (ipsr != 11u) && (ipsr != 14u);
}

void arghtos_port_request_dispatch(void)
{
    volatile uint32_t *const icsr =
        (volatile uint32_t *)ARGHTOS_M23_SCB_ICSR_ADDR;
    *icsr = ARGHTOS_M23_ICSR_PENDSVSET;
}

ARGHTOS_NORETURN void arghtos_port_start(void)
{
    arghtos_port_request_dispatch();
    __asm volatile(
        "cpsie i\n"
        "isb\n"
        ::: "memory");
    for (;;) {
        __asm volatile("wfi" ::: "memory");
    }
}

void arghtos_port_idle_wait(void)
{
    /*
     * Called from the PendSV idle loop with PRIMASK=1.
     *
     * WFI wake qualification ignores PRIMASK for an otherwise eligible
     * asynchronous exception. After WFI completes, briefly restore normal
     * interrupt recognition so a pending higher-priority handler can run.
     * On return from that handler, mask again before the next scheduler
     * readiness check. Do not execute a second WFI after the handler has
     * had a chance to make work READY.
     */
    __asm volatile(
        "dsb\n"
        "wfi\n"
        "cpsie i\n"
        "isb\n"
        "cpsid i\n"
        ::: "memory");
}

static arghtos_context_t m23_build_context_at_top(
    uintptr_t top,
    void (*thread_start)(void),
    void (*return_guard)(void))
{
    uint32_t *sp;
    unsigned i;

    if ((top == 0u) || ((top & 0x7u) != 0u) ||
        (thread_start == NULL) || (return_guard == NULL)) {
        return ARGHTOS_CONTEXT_NONE;
    }

    sp = (uint32_t *)(top - 64u);

    for (i = 0u; i < 16u; ++i) {
        sp[i] = 0u;
    }

    sp[13] = (uint32_t)((uintptr_t)return_guard | (uintptr_t)1u);
    sp[14] = (uint32_t)((uintptr_t)thread_start & ~(uintptr_t)1u);
    sp[15] = ARGHTOS_M23_XPSR_T;

    return (arghtos_context_t)(uintptr_t)sp;
}

arghtos_context_t arghtos_port_build_initial_context(
    void *stack_base,
    uint32_t stack_size,
    void (*thread_start)(void),
    void (*return_guard)(void))
{
    uintptr_t top;

    if (!arghtos_port_stack_valid(stack_base, stack_size)) {
        return ARGHTOS_CONTEXT_NONE;
    }

    top = ((uintptr_t)stack_base + stack_size) & ~(uintptr_t)0x7u;
    return m23_build_context_at_top(top, thread_start, return_guard);
}

arghtos_context_t arghtos_port_build_initial_context_top(
    void *stack_top,
    void (*thread_start)(void),
    void (*return_guard)(void))
{
    return m23_build_context_at_top(
        (uintptr_t)stack_top, thread_start, return_guard);
}

#if ARGHTOS_ENTRY_NEXT_OVERLAY
arghtos_context_t arghtos_port_build_initial_context_top_arg(
    void *stack_top,
    void (*thread_start)(arghtos_task_fn_t),
    void (*return_guard)(void),
    arghtos_task_fn_t initial_arg)
{
    const arghtos_context_t context = m23_build_context_at_top(
        (uintptr_t)stack_top,
        (void (*)(void))thread_start,
        return_guard);

    if ((context != ARGHTOS_CONTEXT_NONE) && (initial_arg != NULL)) {
        ((uint32_t *)(uintptr_t)context)[8] =
            (uint32_t)(uintptr_t)initial_arg;
        return context;
    }

    return ARGHTOS_CONTEXT_NONE;
}
#endif

#if !ARGHTOS_R5K_ASM_CRASH_FILL
void arghtos_port_crash_fill(
    arghtos_crash_record_t *record,
    const void *port_fault_context)
{
    const arghtos_m23_fault_context_t *ctx =
        (const arghtos_m23_fault_context_t *)port_fault_context;

    record->port_format = ARGHTOS_M23_CRASH_FORMAT;

    if (ctx == NULL) {
        record->port_words = 2u;
        record->port_state[0] = m23_read_msp();
        record->port_state[1] = m23_read_psp();
        return;
    }

    record->port_state[0] = ctx->msp;
    record->port_state[1] = ctx->psp;
    record->port_state[2] = ctx->exc_return;

    if (ctx->basic_frame != NULL) {
        record->port_state[3] = ctx->basic_frame[0];
        record->port_state[4] = ctx->basic_frame[1];
        record->port_state[5] = ctx->basic_frame[2];
        record->port_state[6] = ctx->basic_frame[3];
        record->port_state[7] = ctx->basic_frame[5];
        record->port_state[8] = ctx->basic_frame[6];
        record->port_state[9] = ctx->basic_frame[7];
        record->port_words = 10u;
    } else {
        record->port_words = 3u;
    }
}
#endif

ARGHTOS_NORETURN void arghtos_port_fault_terminal(void)
{
    arghtos_m23_board_fault_terminal();
}


ARGHTOS_NORETURN void arghtos_m23_hardfault_entry(
    uint32_t exc_return,
    const uint32_t *basic_frame,
    uint32_t msp_at_entry,
    uint32_t psp_at_entry)
{
    arghtos_m23_fault_context_t ctx;

    ctx.msp = msp_at_entry;
    ctx.psp = psp_at_entry;
    ctx.exc_return = exc_return;
    ctx.basic_frame = basic_frame;

    arghtos_fatal(ARGHTOS_FAULT_CPU_EXCEPTION, &ctx);
}
