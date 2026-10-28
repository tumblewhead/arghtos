#ifndef ARGHTOS_INTERNAL_H
#define ARGHTOS_INTERNAL_H

#include <arghtos/arghtos.h>
#include <arghtos/arghtos_port.h>
#include "arghtos_kernel_port.h"

#ifndef ARGHTOS_STATIC_CONFIG
#define ARGHTOS_STATIC_CONFIG 0
#endif

#ifndef ARGHTOS_COMPACT_RUNTIME
#define ARGHTOS_COMPACT_RUNTIME 0
#endif

#ifndef ARGHTOS_COMPACT_DESCRIPTOR
#define ARGHTOS_COMPACT_DESCRIPTOR 0
#endif

#ifndef ARGHTOS_UNIFIED_SLOT
#define ARGHTOS_UNIFIED_SLOT 0
#endif

#ifndef ARGHTOS_PRODUCTION_PROFILE
#define ARGHTOS_PRODUCTION_PROFILE 0
#endif

#ifndef ARGHTOS_SPECIALIZED_SERVICES
#define ARGHTOS_SPECIALIZED_SERVICES 0
#endif

#ifndef ARGHTOS_MINIMAL_VALIDATION
#define ARGHTOS_MINIMAL_VALIDATION 0
#endif

#ifndef ARGHTOS_SIZE_FIRST_SELECTOR
#define ARGHTOS_SIZE_FIRST_SELECTOR 0
#endif

#ifndef ARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME
#define ARGHTOS_SIZE_FIRST_NO_DISPATCH_TIME 0
#endif

#ifndef ARGHTOS_SIZE_FIRST_NO_START_TIME
#define ARGHTOS_SIZE_FIRST_NO_START_TIME 0
#endif

#ifndef ARGHTOS_SPLIT_META
#define ARGHTOS_SPLIT_META 0
#endif

#ifndef ARGHTOS_ENTRY_NEXT_OVERLAY
#define ARGHTOS_ENTRY_NEXT_OVERLAY 0
#endif

#ifndef ARGHTOS_R5C_COMPACT_WAKE_RESULT
#define ARGHTOS_R5C_COMPACT_WAKE_RESULT 0
#endif

#if ARGHTOS_R5C_COMPACT_WAKE_RESULT && !ARGHTOS_SPECIALIZED_SERVICES
#error "R5c compact wake result requires specialized production services"
#endif

#ifndef ARGHTOS_R5D_COMPACT_SERVICE_OUTCOME
#define ARGHTOS_R5D_COMPACT_SERVICE_OUTCOME 0
#endif

#if ARGHTOS_R5D_COMPACT_SERVICE_OUTCOME && !ARGHTOS_SPECIALIZED_SERVICES
#error "R5d compact service outcome requires specialized production services"
#endif

#ifndef ARGHTOS_R5E_AGE_NEXT_OVERLAY
#define ARGHTOS_R5E_AGE_NEXT_OVERLAY 0
#endif

#if ARGHTOS_R5E_AGE_NEXT_OVERLAY && \
    (!ARGHTOS_ENTRY_NEXT_OVERLAY || !ARGHTOS_SPLIT_META || ARGHTOS_RUNTIME_PROFILING)
#error "R5e age/deadline overlay requires no-profile split entry/next profile"
#endif

#ifndef ARGHTOS_R5F_COMPRESSED_SCHEDULE
#define ARGHTOS_R5F_COMPRESSED_SCHEDULE 0
#endif

#if ARGHTOS_R5F_COMPRESSED_SCHEDULE && \
    (!ARGHTOS_UNIFIED_SLOT || !ARGHTOS_ENTRY_NEXT_OVERLAY || !ARGHTOS_SPLIT_META)
#error "R5f compressed schedule requires unified split entry/next profile"
#endif

#define ARGHTOS_R5F_SCHEDULE_ESCAPE 0x7Fu

#if ARGHTOS_COMPACT_DESCRIPTOR && !ARGHTOS_STATIC_CONFIG
#error "ARGHTOS_COMPACT_DESCRIPTOR requires ARGHTOS_STATIC_CONFIG=1"
#endif

#if ARGHTOS_UNIFIED_SLOT && \
    (!ARGHTOS_STATIC_CONFIG || !ARGHTOS_COMPACT_RUNTIME || !ARGHTOS_COMPACT_DESCRIPTOR)
#error "ARGHTOS_UNIFIED_SLOT requires static + compact runtime + compact descriptor"
#endif

#if ARGHTOS_PRODUCTION_PROFILE && !ARGHTOS_UNIFIED_SLOT
#error "ARGHTOS_PRODUCTION_PROFILE requires the R2c unified-slot profile"
#endif

#if ARGHTOS_SPECIALIZED_SERVICES && !ARGHTOS_PRODUCTION_PROFILE
#error "ARGHTOS_SPECIALIZED_SERVICES requires ARGHTOS_PRODUCTION_PROFILE=1"
#endif

#if ARGHTOS_MINIMAL_VALIDATION && !ARGHTOS_SPECIALIZED_SERVICES
#error "ARGHTOS_MINIMAL_VALIDATION requires ARGHTOS_SPECIALIZED_SERVICES=1"
#endif

#if !ARGHTOS_RUNTIME_PROFILING && !ARGHTOS_MINIMAL_VALIDATION
#error "profiling may be disabled only in the minimum-size production profile"
#endif

#if ARGHTOS_SIZE_FIRST_NO_START_TIME && \
    (!ARGHTOS_MINIMAL_VALIDATION || ARGHTOS_RUNTIME_PROFILING)
#error "start_time may be removed only in the no-profile minimum-size profile"
#endif

#if ARGHTOS_SPLIT_META && \
    (!ARGHTOS_UNIFIED_SLOT || ARGHTOS_RUNTIME_PROFILING)
#error "split metadata requires the no-profile unified-slot minimum profile"
#endif

#if ARGHTOS_ENTRY_NEXT_OVERLAY && !ARGHTOS_SPLIT_META
#error "entry/next-time overlay requires split task metadata"
#endif

#if ARGHTOS_STATIC_CONFIG
#ifndef ARGHTOS_STATIC_TASK_COUNT
#error "ARGHTOS_STATIC_TASK_COUNT required for static configuration profile"
#endif
#if ARGHTOS_STATIC_TASK_COUNT < 1 || ARGHTOS_STATIC_TASK_COUNT > ARGHTOS_MAX_TASKS
#error "ARGHTOS_STATIC_TASK_COUNT out of range"
#endif
#endif

#if ARGHTOS_COMPACT_DESCRIPTOR
typedef struct {
    arghtos_task_fn_t entry;
    void             *stack_top;
    uint32_t          schedule;
} arghtos_static_task_t;

#define ARGHTOS_STATIC_ASYNC_TAG 0x80000000u
#define ARGHTOS_STATIC_ASYNC_MASK 0x000000FFu

#define ARGHTOS_STATIC_PERIOD(p) ((uint32_t)(p))
#define ARGHTOS_STATIC_ASYNC(prio) \
    (ARGHTOS_STATIC_ASYNC_TAG | ((uint32_t)(prio) & ARGHTOS_STATIC_ASYNC_MASK))
#endif

#define ARGHTOS_TCF_WAKE_PENDING (1u << 0)
#define ARGHTOS_TCF_RESERVED     (0xFEu)

#define ARGHTOS_KF_INITIALIZED  (1u << 0)
#define ARGHTOS_KF_STARTED      (1u << 1)
#define ARGHTOS_KF_FAULT_ACTIVE (1u << 2)
#define ARGHTOS_KF_RESERVED     (0xF8u)

#if ARGHTOS_COMPACT_RUNTIME
typedef struct {
    arghtos_context_t context;
#if ARGHTOS_ENTRY_NEXT_OVERLAY
    union {
        arghtos_task_fn_t entry;
        arghtos_time_t    next_time;
#if ARGHTOS_R5E_AGE_NEXT_OVERLAY
        uint8_t           age_credit;
#endif
    } entry_or_next;
#else
    arghtos_time_t    next_time;
#endif
#if ARGHTOS_RUNTIME_PROFILING
    uint32_t          total_cpu_ticks;
    uint16_t          max_run_ticks;
#endif
#if !ARGHTOS_SPLIT_META
    uint8_t           age_credit;
    uint8_t           status;
#endif
#if ARGHTOS_UNIFIED_SLOT
#if !ARGHTOS_ENTRY_NEXT_OVERLAY
    arghtos_task_fn_t entry;
#endif
#if !ARGHTOS_R5F_COMPRESSED_SCHEDULE
    uint32_t          schedule;
#endif
#endif
} arghtos_task_control_t;

#if ARGHTOS_ENTRY_NEXT_OVERLAY
#define ARGHTOS_CTRL_NEXT(c)  ((c)->entry_or_next.next_time)
#define ARGHTOS_CTRL_ENTRY(c) ((c)->entry_or_next.entry)
#else
#define ARGHTOS_CTRL_NEXT(c)  ((c)->next_time)
#if ARGHTOS_UNIFIED_SLOT
#define ARGHTOS_CTRL_ENTRY(c) ((c)->entry)
#endif
#endif

#if ARGHTOS_SPLIT_META
#if ARGHTOS_R5E_AGE_NEXT_OVERLAY
typedef struct {
    uint8_t status;
} arghtos_task_meta_t;
#else
typedef struct {
    uint8_t age_credit;
    uint8_t status;
} arghtos_task_meta_t;
#endif
extern arghtos_task_meta_t arghtos_static_meta[ARGHTOS_MAX_TASKS];
#define ARGHTOS_CTRL_META(c) \
    (arghtos_static_meta[(size_t)((c) - arghtos_static_slots)])
#if ARGHTOS_R5E_AGE_NEXT_OVERLAY
#define ARGHTOS_CTRL_AGE(c)    ((c)->entry_or_next.age_credit)
#else
#define ARGHTOS_CTRL_AGE(c)    (ARGHTOS_CTRL_META(c).age_credit)
#endif
#define ARGHTOS_CTRL_STATUS(c) (ARGHTOS_CTRL_META(c).status)
#else
#define ARGHTOS_CTRL_AGE(c)    ((c)->age_credit)
#define ARGHTOS_CTRL_STATUS(c) ((c)->status)
#endif

#define ARGHTOS_CTRL_STATE_MASK   0x07u
#define ARGHTOS_CTRL_WAKE_MASK    0x08u
#define ARGHTOS_CTRL_MISSED_SHIFT 4u
#define ARGHTOS_CTRL_MISSED_MAX   0x0Fu

#define ARGHTOS_CTRL_STATE(c) \
    ((uint8_t)(ARGHTOS_CTRL_STATUS(c) & ARGHTOS_CTRL_STATE_MASK))

#define ARGHTOS_CTRL_SET_STATE(c, v) do { \
    ARGHTOS_CTRL_STATUS(c) = (uint8_t)( \
        (ARGHTOS_CTRL_STATUS(c) & (uint8_t)~ARGHTOS_CTRL_STATE_MASK) | \
        ((uint8_t)(v) & ARGHTOS_CTRL_STATE_MASK)); \
} while (0)

#define ARGHTOS_CTRL_WAKE_PENDING(c) \
    ((ARGHTOS_CTRL_STATUS(c) & ARGHTOS_CTRL_WAKE_MASK) != 0u)

#define ARGHTOS_CTRL_SET_WAKE(c) \
    (ARGHTOS_CTRL_STATUS(c) |= ARGHTOS_CTRL_WAKE_MASK)

#define ARGHTOS_CTRL_CLEAR_WAKE(c) \
    (ARGHTOS_CTRL_STATUS(c) &= (uint8_t)~ARGHTOS_CTRL_WAKE_MASK)

#define ARGHTOS_CTRL_MISSED(c) \
    ((uint8_t)(ARGHTOS_CTRL_STATUS(c) >> ARGHTOS_CTRL_MISSED_SHIFT))

#define ARGHTOS_CTRL_SET_MISSED(c, v) do { \
    ARGHTOS_CTRL_STATUS(c) = (uint8_t)( \
        (ARGHTOS_CTRL_STATUS(c) & 0x0Fu) | \
        (((uint8_t)(v) & ARGHTOS_CTRL_MISSED_MAX) << \
         ARGHTOS_CTRL_MISSED_SHIFT)); \
} while (0)

#define ARGHTOS_CTRL_FLAGS(c) \
    (ARGHTOS_CTRL_WAKE_PENDING(c) ? ARGHTOS_TCF_WAKE_PENDING : 0u)

#if ARGHTOS_RUNTIME_PROFILING
#define ARGHTOS_CTRL_MAX_RUN(c) ((uint32_t)(c)->max_run_ticks)
#endif
#else
typedef struct {
    arghtos_context_t context;
    arghtos_time_t    next_time;
    uint32_t          total_cpu_ticks;
    uint32_t          max_run_ticks;
    uint8_t           missed_release_count;
    uint8_t           age_credit;
    uint8_t           state;
    uint8_t           flags;
} arghtos_task_control_t;

#define ARGHTOS_CTRL_NEXT(c) ((c)->next_time)
#define ARGHTOS_CTRL_AGE(c) ((c)->age_credit)
#define ARGHTOS_CTRL_STATE(c) ((c)->state)
#define ARGHTOS_CTRL_SET_STATE(c, v) ((c)->state = (uint8_t)(v))
#define ARGHTOS_CTRL_WAKE_PENDING(c) \
    (((c)->flags & ARGHTOS_TCF_WAKE_PENDING) != 0u)
#define ARGHTOS_CTRL_SET_WAKE(c) \
    ((c)->flags |= ARGHTOS_TCF_WAKE_PENDING)
#define ARGHTOS_CTRL_CLEAR_WAKE(c) \
    ((c)->flags &= (uint8_t)~ARGHTOS_TCF_WAKE_PENDING)
#define ARGHTOS_CTRL_MISSED(c) ((c)->missed_release_count)
#define ARGHTOS_CTRL_SET_MISSED(c, v) \
    ((c)->missed_release_count = (uint8_t)(v))
#define ARGHTOS_CTRL_FLAGS(c) ((c)->flags)
#define ARGHTOS_CTRL_MAX_RUN(c) ((uint32_t)(c)->max_run_ticks)
#endif

typedef struct {
#if !ARGHTOS_STATIC_CONFIG
    const arghtos_task_config_t *tasks;
    arghtos_crash_record_t      *crash_record;
#endif
#if !ARGHTOS_SIZE_FIRST_NO_START_TIME
    arghtos_time_t               start_time;
#endif
#if ARGHTOS_RUNTIME_PROFILING
    arghtos_time_t               run_start;
#endif
    arghtos_task_id_t            current_task;
#if !ARGHTOS_STATIC_CONFIG
    uint8_t                      task_count;
#endif
    uint8_t                      flags;
#if ARGHTOS_STATIC_CONFIG
    uint8_t                      reserved[2];
#else
    uint8_t                      reserved;
#endif
} arghtos_kernel_state_t;

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(arghtos_task_config_t) == 16u, "32-bit task descriptor ABI changed");
#if ARGHTOS_COMPACT_DESCRIPTOR
_Static_assert(sizeof(arghtos_static_task_t) == 12u, "32-bit compact static descriptor ABI changed");
#endif
_Static_assert(sizeof(arghtos_config_t) == 12u, "32-bit init config ABI changed");
_Static_assert(sizeof(arghtos_task_profile_t) == 12u, "task profile ABI changed");
_Static_assert(sizeof(arghtos_crash_record_t) == 64u, "crash record ABI changed");
#if ARGHTOS_UNIFIED_SLOT
#if ARGHTOS_RUNTIME_PROFILING
_Static_assert(sizeof(arghtos_task_control_t) == 24u, "32-bit unified task slot ABI changed");
#elif ARGHTOS_R5F_COMPRESSED_SCHEDULE
_Static_assert(sizeof(arghtos_task_control_t) == 8u, "32-bit R5f core slot ABI changed");
_Static_assert(sizeof(arghtos_task_meta_t) == 1u, "R5f metadata ABI changed");
#elif ARGHTOS_R5E_AGE_NEXT_OVERLAY
_Static_assert(sizeof(arghtos_task_control_t) == 12u, "32-bit R5e core slot ABI changed");
_Static_assert(sizeof(arghtos_task_meta_t) == 1u, "R5e metadata ABI changed");
#elif ARGHTOS_ENTRY_NEXT_OVERLAY
_Static_assert(sizeof(arghtos_task_control_t) == 12u, "32-bit entry-overlay core slot ABI changed");
_Static_assert(sizeof(arghtos_task_meta_t) == 2u, "entry-overlay metadata ABI changed");
#elif ARGHTOS_SPLIT_META
_Static_assert(sizeof(arghtos_task_control_t) == 16u, "32-bit split-meta core slot ABI changed");
_Static_assert(sizeof(arghtos_task_meta_t) == 2u, "split-meta task metadata ABI changed");
#else
_Static_assert(sizeof(arghtos_task_control_t) == 20u, "32-bit no-profile unified task slot ABI changed");
#endif
#elif ARGHTOS_COMPACT_RUNTIME
_Static_assert(sizeof(arghtos_task_control_t) == 16u, "32-bit compact task control ABI changed");
#else
_Static_assert(sizeof(arghtos_task_control_t) == 20u, "32-bit task control ABI changed");
#endif
#if ARGHTOS_STATIC_CONFIG
#if ARGHTOS_RUNTIME_PROFILING
_Static_assert(sizeof(arghtos_kernel_state_t) == 12u, "32-bit static-profile kernel state ABI changed");
#elif ARGHTOS_SIZE_FIRST_NO_START_TIME
_Static_assert(sizeof(arghtos_kernel_state_t) == 4u, "32-bit minimum-size kernel state ABI changed");
#else
_Static_assert(sizeof(arghtos_kernel_state_t) == 8u, "32-bit no-profile static kernel state ABI changed");
#endif
#else
_Static_assert(sizeof(arghtos_kernel_state_t) == 20u, "32-bit kernel state ABI changed");
#endif
#endif

arghtos_task_id_t arghtos_select_next(void);
#if ARGHTOS_ENTRY_NEXT_OVERLAY
ARGHTOS_NORETURN void arghtos_task_start_trampoline(arghtos_task_fn_t entry);
#else
ARGHTOS_NORETURN void arghtos_task_start_trampoline(void);
#endif
ARGHTOS_NORETURN void arghtos_task_return_guard(void);

#endif
