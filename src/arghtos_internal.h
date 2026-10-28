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

#if ARGHTOS_STATIC_CONFIG
#ifndef ARGHTOS_STATIC_TASK_COUNT
#error "ARGHTOS_STATIC_TASK_COUNT required for static configuration profile"
#endif
#if ARGHTOS_STATIC_TASK_COUNT < 1 || ARGHTOS_STATIC_TASK_COUNT > ARGHTOS_MAX_TASKS
#error "ARGHTOS_STATIC_TASK_COUNT out of range"
#endif
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
    arghtos_time_t    next_time;
    uint32_t          total_cpu_ticks;
    uint16_t          max_run_ticks;
    uint8_t           age_credit;
    uint8_t           status;
} arghtos_task_control_t;

#define ARGHTOS_CTRL_STATE_MASK   0x07u
#define ARGHTOS_CTRL_WAKE_MASK    0x08u
#define ARGHTOS_CTRL_MISSED_SHIFT 4u
#define ARGHTOS_CTRL_MISSED_MAX   0x0Fu

#define ARGHTOS_CTRL_STATE(c) \
    ((uint8_t)((c)->status & ARGHTOS_CTRL_STATE_MASK))

#define ARGHTOS_CTRL_SET_STATE(c, v) do { \
    (c)->status = (uint8_t)(((c)->status & (uint8_t)~ARGHTOS_CTRL_STATE_MASK) | \
        ((uint8_t)(v) & ARGHTOS_CTRL_STATE_MASK)); \
} while (0)

#define ARGHTOS_CTRL_WAKE_PENDING(c) \
    (((c)->status & ARGHTOS_CTRL_WAKE_MASK) != 0u)

#define ARGHTOS_CTRL_SET_WAKE(c) \
    ((c)->status |= ARGHTOS_CTRL_WAKE_MASK)

#define ARGHTOS_CTRL_CLEAR_WAKE(c) \
    ((c)->status &= (uint8_t)~ARGHTOS_CTRL_WAKE_MASK)

#define ARGHTOS_CTRL_MISSED(c) \
    ((uint8_t)((c)->status >> ARGHTOS_CTRL_MISSED_SHIFT))

#define ARGHTOS_CTRL_SET_MISSED(c, v) do { \
    (c)->status = (uint8_t)(((c)->status & 0x0Fu) | \
        (((uint8_t)(v) & ARGHTOS_CTRL_MISSED_MAX) << \
         ARGHTOS_CTRL_MISSED_SHIFT)); \
} while (0)

#define ARGHTOS_CTRL_FLAGS(c) \
    (ARGHTOS_CTRL_WAKE_PENDING(c) ? ARGHTOS_TCF_WAKE_PENDING : 0u)

#define ARGHTOS_CTRL_MAX_RUN(c) ((uint32_t)(c)->max_run_ticks)
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
    arghtos_time_t               start_time;
    arghtos_time_t               run_start;
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
_Static_assert(sizeof(arghtos_config_t) == 12u, "32-bit init config ABI changed");
_Static_assert(sizeof(arghtos_task_profile_t) == 12u, "task profile ABI changed");
_Static_assert(sizeof(arghtos_crash_record_t) == 64u, "crash record ABI changed");
#if ARGHTOS_COMPACT_RUNTIME
_Static_assert(sizeof(arghtos_task_control_t) == 16u, "32-bit compact task control ABI changed");
#else
_Static_assert(sizeof(arghtos_task_control_t) == 20u, "32-bit task control ABI changed");
#endif
#if ARGHTOS_STATIC_CONFIG
_Static_assert(sizeof(arghtos_kernel_state_t) == 12u, "32-bit static-profile kernel state ABI changed");
#else
_Static_assert(sizeof(arghtos_kernel_state_t) == 20u, "32-bit kernel state ABI changed");
#endif
#endif

arghtos_task_id_t arghtos_select_next(void);
ARGHTOS_NORETURN void arghtos_task_start_trampoline(void);
ARGHTOS_NORETURN void arghtos_task_return_guard(void);

#endif
