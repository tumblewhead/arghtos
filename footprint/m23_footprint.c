#include <arghtos/arghtos.h>
#include <arghtos/arghtos_port.h>
#include "../src/arghtos_kernel_port.h"

extern void SVC_Handler(void);
extern void PendSV_Handler(void);
extern ARGHTOS_NORETURN void arghtos_task_start_trampoline(void);
extern ARGHTOS_NORETURN void arghtos_task_return_guard(void);

__attribute__((section(".app_stacks"), aligned(8)))
static uint8_t app_stacks[32][128];

__attribute__((section(".app_text"), noinline))
static void task_stub(void)
{
    __asm volatile("");
}

#define DESC(i, p) { task_stub, app_stacks[(i)], (p), sizeof(app_stacks[(i)]), 0u, 0u }

#if ARGHTOS_STATIC_CONFIG
__attribute__((section(".arghtos_cfg")))
const arghtos_task_config_t arghtos_static_tasks[32] = {
    DESC(0, 1u), DESC(1, 2u), DESC(2, 3u), DESC(3, 4u),
    DESC(4, 5u), DESC(5, 6u), DESC(6, 7u), DESC(7, 8u),
    DESC(8, 9u), DESC(9, 10u), DESC(10, 11u), DESC(11, 12u),
    DESC(12, 13u), DESC(13, 14u), DESC(14, 15u), DESC(15, 16u),
    DESC(16, 17u), DESC(17, 18u), DESC(18, 19u), DESC(19, 20u),
    DESC(20, 21u), DESC(21, 22u), DESC(22, 23u), DESC(23, 24u),
    DESC(24, 25u), DESC(25, 26u), DESC(26, 27u), DESC(27, 28u),
    DESC(28, 29u), DESC(29, 30u), DESC(30, 31u), DESC(31, 32u)
};

__attribute__((section(".arghtos_crash"), aligned(4)))
arghtos_crash_record_t arghtos_static_crash_record;
#else
__attribute__((section(".arghtos_cfg")))
static const arghtos_task_config_t task_cfg[32] = {
    DESC(0, 1u), DESC(1, 2u), DESC(2, 3u), DESC(3, 4u),
    DESC(4, 5u), DESC(5, 6u), DESC(6, 7u), DESC(7, 8u),
    DESC(8, 9u), DESC(9, 10u), DESC(10, 11u), DESC(11, 12u),
    DESC(12, 13u), DESC(13, 14u), DESC(14, 15u), DESC(15, 16u),
    DESC(16, 17u), DESC(17, 18u), DESC(18, 19u), DESC(19, 20u),
    DESC(20, 21u), DESC(21, 22u), DESC(22, 23u), DESC(23, 24u),
    DESC(24, 25u), DESC(25, 26u), DESC(26, 27u), DESC(27, 28u),
    DESC(28, 29u), DESC(29, 30u), DESC(30, 31u), DESC(31, 32u)
};

__attribute__((section(".arghtos_crash"), aligned(4)))
static arghtos_crash_record_t crash_record;

__attribute__((section(".app_rodata")))
static const arghtos_config_t config = {
    task_cfg,
    &crash_record,
    32u,
    {0u, 0u, 0u}
};
#endif

__attribute__((section(".app_data")))
static volatile uintptr_t anchor_sink;

__attribute__((section(".app_text"), noinline, used))
void footprint_entry(void)
{
    anchor_sink = (uintptr_t)&arghtos_init;
    anchor_sink = (uintptr_t)&arghtos_start;
    anchor_sink = (uintptr_t)&arghtos_yield;
    anchor_sink = (uintptr_t)&arghtos_wait;
    anchor_sink = (uintptr_t)&arghtos_sleep_until;
    anchor_sink = (uintptr_t)&arghtos_sleep_for;
    anchor_sink = (uintptr_t)&arghtos_wake;
    anchor_sink = (uintptr_t)&arghtos_wake_from_interrupt;
    anchor_sink = (uintptr_t)&arghtos_profile_get;
    anchor_sink = (uintptr_t)&arghtos_kernel_dispatch;
    anchor_sink = (uintptr_t)&arghtos_kernel_service;
    anchor_sink = (uintptr_t)&arghtos_timer_interrupt;
    anchor_sink = (uintptr_t)&arghtos_fatal;
    anchor_sink = (uintptr_t)&arghtos_port_service_call;
    anchor_sink = (uintptr_t)&SVC_Handler;
    anchor_sink = (uintptr_t)&PendSV_Handler;
    anchor_sink = (uintptr_t)&arghtos_task_start_trampoline;
    anchor_sink = (uintptr_t)&arghtos_task_return_guard;
#if ARGHTOS_STATIC_CONFIG
    anchor_sink = (uintptr_t)&arghtos_static_tasks[0];
    anchor_sink = (uintptr_t)&arghtos_static_crash_record;
#else
    anchor_sink = (uintptr_t)&config;
#endif

    for (;;) {
        __asm volatile("wfi");
    }
}
