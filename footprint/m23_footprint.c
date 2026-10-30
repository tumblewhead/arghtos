#include <arghtos/arghtos.h>
#include <arghtos/arghtos_port.h>
#include "../src/arghtos_kernel_port.h"
#include "../src/arghtos_internal.h"

extern void SVC_Handler(void);
extern void PendSV_Handler(void);
#if ARGHTOS_ENTRY_NEXT_OVERLAY
extern ARGHTOS_NORETURN void arghtos_task_start_trampoline(arghtos_task_fn_t entry);
#else
extern ARGHTOS_NORETURN void arghtos_task_start_trampoline(void);
#endif
extern ARGHTOS_NORETURN void arghtos_task_return_guard(void);

#if ARGHTOS_R5L_PREBUILT_CONTEXT
extern uint8_t app_stacks[24][128];

__attribute__((section(".app_text"), noinline))
void task_stub(void)
{
    __asm volatile("");
}
#else
__attribute__((section(".app_stacks"), aligned(8)))
static uint8_t app_stacks[24][128];

__attribute__((section(".app_text"), noinline))
static void task_stub(void)
{
    __asm volatile("");
}
#endif

#define DESC(i, p) { task_stub, app_stacks[(i)], (p), sizeof(app_stacks[(i)]), 0u, 0u }

#if ARGHTOS_STATIC_CONFIG
#if ARGHTOS_UNIFIED_SLOT
#if ARGHTOS_RUNTIME_PROFILING
#define ARGHTOS_PROFILE_SLOT_INIT .total_cpu_ticks = 0u, .max_run_ticks = 0u,
#else
#define ARGHTOS_PROFILE_SLOT_INIT
#endif
#if ARGHTOS_SPLIT_META
#define ARGHTOS_META_SLOT_INIT
#else
#define ARGHTOS_META_SLOT_INIT .age_credit = 0u, .status = 0u,
#endif
#if ARGHTOS_R5L_PREBUILT_CONTEXT
#if ARGHTOS_R6Q_PRESEEDED_PERIOD_OFFSETS
#define ARGHTOS_TIME_ENTRY_INIT(i, p) \
    .entry_or_next.next_time = \
        (((i) < ARGHTOS_R5P_PERIODIC_COUNT) ? (p) : 0u),
#else
#define ARGHTOS_TIME_ENTRY_INIT(i, p) .entry_or_next.next_time = 0u,
#endif
#define ARGHTOS_ENTRY_FIELD_INIT
#elif ARGHTOS_ENTRY_NEXT_OVERLAY
#define ARGHTOS_TIME_ENTRY_INIT(i, p) .entry_or_next.entry = task_stub,
#define ARGHTOS_ENTRY_FIELD_INIT
#else
#define ARGHTOS_TIME_ENTRY_INIT(i, p) .next_time = 0u,
#define ARGHTOS_ENTRY_FIELD_INIT .entry = task_stub,
#endif
#if ARGHTOS_R5F_COMPRESSED_SCHEDULE || ARGHTOS_R5G_SCHEDULE16 || ARGHTOS_R5I_SCHEDULE8
#define ARGHTOS_SCHEDULE_SLOT_INIT(p)
#else
#define ARGHTOS_SCHEDULE_SLOT_INIT(p) .schedule = ARGHTOS_STATIC_PERIOD(p),
#endif
#if ARGHTOS_R5L_PREBUILT_CONTEXT
#define ARGHTOS_INITIAL_CONTEXT(i) \
    ((arghtos_context_t)(uintptr_t)&app_stacks[(i)][64])
#else
#define ARGHTOS_INITIAL_CONTEXT(i) \
    ((arghtos_context_t)(uintptr_t)&app_stacks[(i)][sizeof(app_stacks[(i)])])
#endif
#define SLOT(i, p) { \
    .context = ARGHTOS_INITIAL_CONTEXT(i), \
    ARGHTOS_TIME_ENTRY_INIT(i, p) \
    ARGHTOS_PROFILE_SLOT_INIT \
    ARGHTOS_META_SLOT_INIT \
    ARGHTOS_ENTRY_FIELD_INIT \
    ARGHTOS_SCHEDULE_SLOT_INIT(p) \
}

__attribute__((section(".arghtos_cfg")))
arghtos_task_control_t arghtos_static_slots[ARGHTOS_MAX_TASKS] = {
    SLOT(0, 1u), SLOT(1, 2u), SLOT(2, 3u), SLOT(3, 4u),
    SLOT(4, 5u), SLOT(5, 6u), SLOT(6, 7u), SLOT(7, 8u),
    SLOT(8, 9u), SLOT(9, 10u), SLOT(10, 11u), SLOT(11, 12u),
    SLOT(12, 13u), SLOT(13, 14u), SLOT(14, 15u), SLOT(15, 16u),
    SLOT(16, 17u), SLOT(17, 18u), SLOT(18, 19u), SLOT(19, 20u),
    SLOT(20, 21u), SLOT(21, 22u), SLOT(22, 23u), SLOT(23, 24u)
};
#if ARGHTOS_R5F_COMPRESSED_SCHEDULE
__attribute__((section(".arghtos_cfg")))
uint8_t arghtos_static_schedule_stream[ARGHTOS_MAX_TASKS] = {
    1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u,
    9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u,
    17u, 18u, 19u, 20u, 21u, 22u, 23u, 24u
};
#endif
#if ARGHTOS_R5G_SCHEDULE16
__attribute__((section(".arghtos_cfg")))
uint16_t arghtos_static_schedule16[ARGHTOS_MAX_TASKS] = {
    1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u,
    9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u,
    17u, 18u, 19u, 20u, 21u, 22u, 23u, 24u
};
#endif
#if ARGHTOS_R5I_SCHEDULE8
__attribute__((section(".arghtos_cfg")))
#if ARGHTOS_R6AO_PERIODIC_ONLY_SCHEDULE8
uint8_t arghtos_static_schedule8[ARGHTOS_R5P_PERIODIC_COUNT] = {
    1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u,
    9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u,
    17u, 18u, 19u, 20u, 21u, 22u, 23u
};
_Static_assert(sizeof(arghtos_static_schedule8) == ARGHTOS_R5P_PERIODIC_COUNT,
               "R6ao physical schedule8 must contain only periodic entries");
#else
uint8_t arghtos_static_schedule8[ARGHTOS_MAX_TASKS] = {
    1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u,
    9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u,
    17u, 18u, 19u, 20u, 21u, 22u, 23u,
#if ARGHTOS_R5Z_RAW_CLASS_SCHEDULE
    3u
#else
    ARGHTOS_R5I_ASYNC_BASE | 3u
#endif
};
#endif
#endif
#if ARGHTOS_SPLIT_META
__attribute__((section(".arghtos_cfg")))
#if ARGHTOS_R6K_PRESEEDED_PERIODIC_READY
arghtos_task_meta_t arghtos_static_meta[ARGHTOS_MAX_TASKS] = {
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }, { .status = ARGHTOS_TASK_READY },
    { .status = ARGHTOS_TASK_READY }
};
#else
arghtos_task_meta_t arghtos_static_meta[ARGHTOS_MAX_TASKS];
#endif
#endif
#elif ARGHTOS_COMPACT_DESCRIPTOR
#define CDESC(i, p) \
    { task_stub, &app_stacks[(i)][sizeof(app_stacks[(i)])], ARGHTOS_STATIC_PERIOD(p) }

__attribute__((section(".arghtos_cfg")))
const arghtos_static_task_t arghtos_static_tasks[24] = {
    CDESC(0, 1u), CDESC(1, 2u), CDESC(2, 3u), CDESC(3, 4u),
    CDESC(4, 5u), CDESC(5, 6u), CDESC(6, 7u), CDESC(7, 8u),
    CDESC(8, 9u), CDESC(9, 10u), CDESC(10, 11u), CDESC(11, 12u),
    CDESC(12, 13u), CDESC(13, 14u), CDESC(14, 15u), CDESC(15, 16u),
    CDESC(16, 17u), CDESC(17, 18u), CDESC(18, 19u), CDESC(19, 20u),
    CDESC(20, 21u), CDESC(21, 22u), CDESC(22, 23u), CDESC(23, 24u)
};
#else
__attribute__((section(".arghtos_cfg")))
const arghtos_task_config_t arghtos_static_tasks[24] = {
    DESC(0, 1u), DESC(1, 2u), DESC(2, 3u), DESC(3, 4u),
    DESC(4, 5u), DESC(5, 6u), DESC(6, 7u), DESC(7, 8u),
    DESC(8, 9u), DESC(9, 10u), DESC(10, 11u), DESC(11, 12u),
    DESC(12, 13u), DESC(13, 14u), DESC(14, 15u), DESC(15, 16u),
    DESC(16, 17u), DESC(17, 18u), DESC(18, 19u), DESC(19, 20u),
    DESC(20, 21u), DESC(21, 22u), DESC(22, 23u), DESC(23, 24u)
};
#endif

__attribute__((section(".arghtos_crash"), aligned(4)))
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
#else
__attribute__((section(".arghtos_cfg")))
static const arghtos_task_config_t task_cfg[24] = {
    DESC(0, 1u), DESC(1, 2u), DESC(2, 3u), DESC(3, 4u),
    DESC(4, 5u), DESC(5, 6u), DESC(6, 7u), DESC(7, 8u),
    DESC(8, 9u), DESC(9, 10u), DESC(10, 11u), DESC(11, 12u),
    DESC(12, 13u), DESC(13, 14u), DESC(14, 15u), DESC(15, 16u),
    DESC(16, 17u), DESC(17, 18u), DESC(18, 19u), DESC(19, 20u),
    DESC(20, 21u), DESC(21, 22u), DESC(22, 23u), DESC(23, 24u)
};

__attribute__((section(".arghtos_crash"), aligned(4)))
static arghtos_crash_record_t crash_record;

__attribute__((section(".app_rodata")))
static const arghtos_config_t config = {
    task_cfg,
    &crash_record,
    24u,
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
#if ARGHTOS_RUNTIME_PROFILING
    anchor_sink = (uintptr_t)&arghtos_profile_get;
#endif
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
#if ARGHTOS_UNIFIED_SLOT
    anchor_sink = (uintptr_t)&arghtos_static_slots[0];
#else
    anchor_sink = (uintptr_t)&arghtos_static_tasks[0];
#endif
    anchor_sink = (uintptr_t)&arghtos_static_crash_record;
#else
    anchor_sink = (uintptr_t)&config;
#endif

    for (;;) {
        __asm volatile("wfi");
    }
}
