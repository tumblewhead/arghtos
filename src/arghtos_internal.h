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

#ifndef ARGHTOS_R5G_SCHEDULE16
#define ARGHTOS_R5G_SCHEDULE16 0
#endif

#ifndef ARGHTOS_R5G_SCHEDULE_OVERFLOW_COUNT
#define ARGHTOS_R5G_SCHEDULE_OVERFLOW_COUNT 0u
#endif

#if ARGHTOS_R5G_SCHEDULE16 && \
    (!ARGHTOS_UNIFIED_SLOT || !ARGHTOS_R5E_AGE_NEXT_OVERLAY)
#error "R5g schedule16 requires the accepted R5e unified profile"
#endif

#if ARGHTOS_R5F_COMPRESSED_SCHEDULE && ARGHTOS_R5G_SCHEDULE16
#error "R5f and R5g schedule encodings are mutually exclusive"
#endif

#if ARGHTOS_R5G_SCHEDULE_OVERFLOW_COUNT > ARGHTOS_MAX_TASKS
#error "R5g schedule overflow count exceeds task capacity"
#endif

#define ARGHTOS_R5G_OVERFLOW_BASE 0x7F00u
#define ARGHTOS_R5G_ASYNC_BASE    0x8000u

#ifndef ARGHTOS_R5H_DIRECT_SCHEDULE16
#define ARGHTOS_R5H_DIRECT_SCHEDULE16 0
#endif

#if ARGHTOS_R5H_DIRECT_SCHEDULE16 && \
    (!ARGHTOS_R5G_SCHEDULE16 || ARGHTOS_R5G_SCHEDULE_OVERFLOW_COUNT != 0u)
#error "R5h direct schedule16 requires R5g with zero overflow entries"
#endif

#ifndef ARGHTOS_R5L_PREBUILT_CONTEXT
#define ARGHTOS_R5L_PREBUILT_CONTEXT 0
#endif

#ifndef ARGHTOS_R5M_PRISTINE_INIT
#define ARGHTOS_R5M_PRISTINE_INIT 0
#endif

#ifndef ARGHTOS_R5N_PROVEN_DISPATCH
#define ARGHTOS_R5N_PROVEN_DISPATCH 0
#endif

#ifndef ARGHTOS_R5O_TWO_PASS_TIME
#define ARGHTOS_R5O_TWO_PASS_TIME 0
#endif

#if ARGHTOS_R5O_TWO_PASS_TIME && !ARGHTOS_R5N_PROVEN_DISPATCH
#error "R5o two-pass time experiment requires the R5n trusted profile"
#endif

#ifndef ARGHTOS_R5O_GENERATED_START_TIMER
#define ARGHTOS_R5O_GENERATED_START_TIMER 0
#endif

#ifndef ARGHTOS_R5O_MIN_PERIOD
#define ARGHTOS_R5O_MIN_PERIOD 0u
#endif

#if ARGHTOS_R5O_GENERATED_START_TIMER && !ARGHTOS_R5N_PROVEN_DISPATCH
#error "R5o generated start timer requires accepted R5n generated profile"
#endif

#if ARGHTOS_R5O_MIN_PERIOD >= 0x80000000u
#error "R5o minimum period must satisfy the scheduler half-range rule"
#endif

#ifndef ARGHTOS_R5P_GENERATED_ORDER
#define ARGHTOS_R5P_GENERATED_ORDER 0
#endif

#ifndef ARGHTOS_R5P_PERIODIC_COUNT
#define ARGHTOS_R5P_PERIODIC_COUNT 0u
#endif

#if ARGHTOS_R5P_GENERATED_ORDER && !ARGHTOS_R5O_GENERATED_START_TIMER
#error "R5p generated ordering requires accepted R5o generated profile"
#endif

#if ARGHTOS_R5P_PERIODIC_COUNT > ARGHTOS_STATIC_TASK_COUNT
#error "R5p periodic prefix exceeds configured task count"
#endif

#ifndef ARGHTOS_R5Z_RAW_CLASS_SCHEDULE
#define ARGHTOS_R5Z_RAW_CLASS_SCHEDULE 0
#endif

#if ARGHTOS_R5Z_RAW_CLASS_SCHEDULE && \
    (!ARGHTOS_R5P_GENERATED_ORDER || !ARGHTOS_R5I_SCHEDULE8 || \
     ARGHTOS_R5I_SCHEDULE_OVERFLOW_COUNT != 0u)
#error "R5z raw class schedule requires generated-order zero-overflow schedule8"
#endif

#ifndef ARGHTOS_R5AB_SINGLE_ASYNC
#define ARGHTOS_R5AB_SINGLE_ASYNC 0
#endif

#if ARGHTOS_R5AB_SINGLE_ASYNC && \
    (!ARGHTOS_R5Z_RAW_CLASS_SCHEDULE || !ARGHTOS_R5R_ELIDE_AGE_RESETS || \
     (ARGHTOS_R5P_PERIODIC_COUNT + 1u) != ARGHTOS_STATIC_TASK_COUNT)
#error "R5ab single-async selector requires exactly one generated async task"
#endif

#ifndef ARGHTOS_R5AC_TERMINAL_NO_FAULT_STATE
#define ARGHTOS_R5AC_TERMINAL_NO_FAULT_STATE 0
#endif

#ifndef ARGHTOS_R6G_PROVEN_CURRENT_DOMAIN
#define ARGHTOS_R6G_PROVEN_CURRENT_DOMAIN 0
#endif

#ifndef ARGHTOS_R6H_SINGLE_ASYNC_TIME
#define ARGHTOS_R6H_SINGLE_ASYNC_TIME 0
#endif

#ifndef ARGHTOS_R6J_PROVEN_WAKE_DOMAIN
#define ARGHTOS_R6J_PROVEN_WAKE_DOMAIN 0
#endif

#ifndef ARGHTOS_R6K_PRESEEDED_PERIODIC_READY
#define ARGHTOS_R6K_PRESEEDED_PERIODIC_READY 0
#endif

#ifndef ARGHTOS_R6L_PREMASKED_START
#define ARGHTOS_R6L_PREMASKED_START 0
#endif

#ifndef ARGHTOS_R6M_COMMIT_RECURSION_GUARD
#define ARGHTOS_R6M_COMMIT_RECURSION_GUARD 0
#endif

#ifndef ARGHTOS_R6AN_REASON_RECURSION_GUARD
#define ARGHTOS_R6AN_REASON_RECURSION_GUARD 0
#endif

#ifndef ARGHTOS_R6AO_PERIODIC_ONLY_SCHEDULE8
#define ARGHTOS_R6AO_PERIODIC_ONLY_SCHEDULE8 0
#endif

#ifndef ARGHTOS_R6AP_OMIT_UNIT_PERIOD_BYTE
#define ARGHTOS_R6AP_OMIT_UNIT_PERIOD_BYTE 0
#endif

#ifndef ARGHTOS_R6AQ_TASK0_ALWAYS_DUE
#define ARGHTOS_R6AQ_TASK0_ALWAYS_DUE 0
#endif

#ifndef ARGHTOS_R6AR_NIBBLE_PERIOD_DELTAS
#define ARGHTOS_R6AR_NIBBLE_PERIOD_DELTAS 0
#endif

#ifndef ARGHTOS_R6AS_TWO_BIT_PERIOD_DELTAS
#define ARGHTOS_R6AS_TWO_BIT_PERIOD_DELTAS 0
#endif

#ifndef ARGHTOS_R6AT_CACHED_NIBBLE_PERIODS
#define ARGHTOS_R6AT_CACHED_NIBBLE_PERIODS 0
#endif

#if ARGHTOS_R6AT_CACHED_NIBBLE_PERIODS && \
    (!ARGHTOS_R6AR_NIBBLE_PERIOD_DELTAS || ARGHTOS_R6AS_TWO_BIT_PERIOD_DELTAS)
#error "R6at cached nibble decoder requires the accepted R6ar nibble format"
#endif

#if ARGHTOS_R6AS_TWO_BIT_PERIOD_DELTAS && !ARGHTOS_R6AR_NIBBLE_PERIOD_DELTAS
#error "R6as two-bit period deltas require the accepted R6ar sequential decoder"
#endif

#if ARGHTOS_R6AR_NIBBLE_PERIOD_DELTAS && \
    (!ARGHTOS_R6AP_OMIT_UNIT_PERIOD_BYTE || !ARGHTOS_R5P_GENERATED_ORDER || \
     !ARGHTOS_R5Q_PARTITIONED_TIME || !ARGHTOS_R5V_UNIT_PERIOD_TIMER || \
     ARGHTOS_R5P_PERIODIC_COUNT < 1u)
#error "R6ar nibble period deltas require sequential generated unit-period scan"
#endif

#if ARGHTOS_R6AQ_TASK0_ALWAYS_DUE && \
    (!ARGHTOS_R6AP_OMIT_UNIT_PERIOD_BYTE || !ARGHTOS_R6AG_TIMER_OWNS_SLEEP_RELEASE || \
     !ARGHTOS_R6B_TRUSTED_TIMER_ENTRY || !ARGHTOS_R5V_UNIT_PERIOD_TIMER)
#error "R6aq task0-always-due requires trusted timer-owned unit-period profile"
#endif

#if ARGHTOS_R6AP_OMIT_UNIT_PERIOD_BYTE && \
    (!ARGHTOS_R6AO_PERIODIC_ONLY_SCHEDULE8 || !ARGHTOS_R5V_UNIT_PERIOD_TIMER || \
     !ARGHTOS_R5P_GENERATED_ORDER || ARGHTOS_R5P_PERIODIC_COUNT < 1u)
#error "R6ap omitted unit-period byte requires periodic-only unit-tick generated schedule"
#endif

#if ARGHTOS_R6AO_PERIODIC_ONLY_SCHEDULE8 && \
    (!ARGHTOS_R5I_SCHEDULE8 || ARGHTOS_R5I_SCHEDULE_OVERFLOW_COUNT != 0u || \
     !ARGHTOS_R5Z_RAW_CLASS_SCHEDULE || !ARGHTOS_R5P_GENERATED_ORDER || \
     !ARGHTOS_R5AB_SINGLE_ASYNC || !ARGHTOS_R6AJ_DIRECT_ASYNC_PREBUILT_ENTRY || \
     (ARGHTOS_R5P_PERIODIC_COUNT + 1u) != ARGHTOS_STATIC_TASK_COUNT)
#error "R6ao periodic-only schedule8 requires generated raw single-async profile"
#endif

#if ARGHTOS_R6AN_REASON_RECURSION_GUARD && \
    (!ARGHTOS_R6M_COMMIT_RECURSION_GUARD || !ARGHTOS_R5S_PRESEEDED_CRASH || \
     !ARGHTOS_R5AC_TERMINAL_NO_FAULT_STATE || !ARGHTOS_STATIC_CONFIG)
#error "R6an reason recursion guard requires preseeded terminal R6m profile"
#endif

#ifndef ARGHTOS_R6N_OK_ONLY_SERVICES
#define ARGHTOS_R6N_OK_ONLY_SERVICES 0
#endif

#ifndef ARGHTOS_R6O_PRESEEDED_CURRENT_INVALID
#define ARGHTOS_R6O_PRESEEDED_CURRENT_INVALID 0
#endif

#ifndef ARGHTOS_R6P_INFALLIBLE_PORT_INIT
#define ARGHTOS_R6P_INFALLIBLE_PORT_INIT 0
#endif

#ifndef ARGHTOS_R6Q_PRESEEDED_PERIOD_OFFSETS
#define ARGHTOS_R6Q_PRESEEDED_PERIOD_OFFSETS 0
#endif

#ifndef ARGHTOS_R6R_FLAGLESS_KERNEL_STATE
#define ARGHTOS_R6R_FLAGLESS_KERNEL_STATE 0
#endif

#ifndef ARGHTOS_R6V_ALWAYS_DISPATCH_TICK
#define ARGHTOS_R6V_ALWAYS_DISPATCH_TICK 0
#endif

#ifndef ARGHTOS_R6W_PERIODIC_READY_NORMALIZE
#define ARGHTOS_R6W_PERIODIC_READY_NORMALIZE 0
#endif

#ifndef ARGHTOS_R6X_BOUNDED_MISSED_LOOP
#define ARGHTOS_R6X_BOUNDED_MISSED_LOOP 0
#endif

#ifndef ARGHTOS_R6Y_PACKED_MISSED_ADD
#define ARGHTOS_R6Y_PACKED_MISSED_ADD 0
#endif

#ifndef ARGHTOS_R6Z_TRUSTED_CRASH_PAYLOAD
#define ARGHTOS_R6Z_TRUSTED_CRASH_PAYLOAD 0
#endif

#ifndef ARGHTOS_R6AB_SINGLE_WAKE_TARGET
#define ARGHTOS_R6AB_SINGLE_WAKE_TARGET 0
#endif

#ifndef ARGHTOS_R6AC_PRESEEDED_PORT_FORMAT
#define ARGHTOS_R6AC_PRESEEDED_PORT_FORMAT 0
#endif

#ifndef ARGHTOS_R6AD_ALWAYS_DISPATCH_WAKE
#define ARGHTOS_R6AD_ALWAYS_DISPATCH_WAKE 0
#endif

#ifndef ARGHTOS_R6AE_DISPATCH_OWNS_YIELD_READY
#define ARGHTOS_R6AE_DISPATCH_OWNS_YIELD_READY 0
#endif

#ifndef ARGHTOS_R6AF_UNCONDITIONAL_CRASH_TASK_ID
#define ARGHTOS_R6AF_UNCONDITIONAL_CRASH_TASK_ID 0
#endif

#ifndef ARGHTOS_R6AG_TIMER_OWNS_SLEEP_RELEASE
#define ARGHTOS_R6AG_TIMER_OWNS_SLEEP_RELEASE 0
#endif

#ifndef ARGHTOS_R6AH_DEFAULT_DISPATCH_SERVICES
#define ARGHTOS_R6AH_DEFAULT_DISPATCH_SERVICES 0
#endif

#ifndef ARGHTOS_R6AI_GENERATED_TRAMPOLINE_CLASS
#define ARGHTOS_R6AI_GENERATED_TRAMPOLINE_CLASS 0
#endif

#ifndef ARGHTOS_R6AJ_DIRECT_ASYNC_PREBUILT_ENTRY
#define ARGHTOS_R6AJ_DIRECT_ASYNC_PREBUILT_ENTRY 0
#endif

#ifndef ARGHTOS_R6AK_ASM_PERIODIC_TRAMPOLINE
#define ARGHTOS_R6AK_ASM_PERIODIC_TRAMPOLINE 0
#endif

#ifndef ARGHTOS_R6AL_DIRECT_UNIT_TIMER_ARM
#define ARGHTOS_R6AL_DIRECT_UNIT_TIMER_ARM 0
#endif

#if ARGHTOS_R6AL_DIRECT_UNIT_TIMER_ARM && \
    (!ARGHTOS_R5V_UNIT_PERIOD_TIMER || !ARGHTOS_R5P_GENERATED_ORDER || \
     ARGHTOS_R5P_PERIODIC_COUNT < 1u || ARGHTOS_R5O_MIN_PERIOD != 1u)
#error "R6al direct timer arm requires generated period-1 task 0"
#endif

#if ARGHTOS_R6AK_ASM_PERIODIC_TRAMPOLINE && \
    (!ARGHTOS_R6AJ_DIRECT_ASYNC_PREBUILT_ENTRY || !ARGHTOS_R5L_PREBUILT_CONTEXT || \
     !ARGHTOS_R5N_PROVEN_DISPATCH || !ARGHTOS_ENTRY_NEXT_OVERLAY)
#error "R6ak assembly trampoline requires the accepted R6aj generated context"
#endif

#if ARGHTOS_R6AJ_DIRECT_ASYNC_PREBUILT_ENTRY && \
    (!ARGHTOS_R5L_PREBUILT_CONTEXT || !ARGHTOS_R5P_GENERATED_ORDER || \
     !ARGHTOS_R5AB_SINGLE_ASYNC || !ARGHTOS_R5Z_RAW_CLASS_SCHEDULE || \
     !ARGHTOS_R5N_PROVEN_DISPATCH || !ARGHTOS_ENTRY_NEXT_OVERLAY || \
     (ARGHTOS_R5P_PERIODIC_COUNT + 1u) != ARGHTOS_STATIC_TASK_COUNT)
#error "R6aj direct async entry requires generated prebuilt single-async ordering"
#endif

#if ARGHTOS_R6AI_GENERATED_TRAMPOLINE_CLASS && \
    (!ARGHTOS_R5P_GENERATED_ORDER || !ARGHTOS_R5N_PROVEN_DISPATCH || \
     ARGHTOS_R5P_PERIODIC_COUNT < 1u || \
     ARGHTOS_R5P_PERIODIC_COUNT >= ARGHTOS_STATIC_TASK_COUNT)
#error "R6ai generated trampoline class requires generated periodic-prefix ordering"
#endif

#if ARGHTOS_R6AH_DEFAULT_DISPATCH_SERVICES && \
    (!ARGHTOS_R6AG_TIMER_OWNS_SLEEP_RELEASE || !ARGHTOS_R6AD_ALWAYS_DISPATCH_WAKE || \
     !ARGHTOS_R6AE_DISPATCH_OWNS_YIELD_READY || !ARGHTOS_R6N_OK_ONLY_SERVICES || \
     !ARGHTOS_R6F_TRUSTED_SERVICE_IDS || !ARGHTOS_R6E_SERIAL_SVC_MUTATION)
#error "R6ah default-dispatch services require the trusted closed service profile"
#endif

#if ARGHTOS_R6AG_TIMER_OWNS_SLEEP_RELEASE && \
    (!ARGHTOS_R6AE_DISPATCH_OWNS_YIELD_READY || !ARGHTOS_R5V_UNIT_PERIOD_TIMER || \
     !ARGHTOS_R6H_SINGLE_ASYNC_TIME || !ARGHTOS_R6V_ALWAYS_DISPATCH_TICK || \
     !ARGHTOS_R6E_SERIAL_SVC_MUTATION)
#error "R6ag timer-owned sleep release requires the trusted unit-tick single-async profile"
#endif

#if ARGHTOS_R6AF_UNCONDITIONAL_CRASH_TASK_ID && \
    (!ARGHTOS_R5N_PROVEN_DISPATCH || !ARGHTOS_R5S_PRESEEDED_CRASH || \
     !ARGHTOS_MINIMAL_VALIDATION || !ARGHTOS_STATIC_CONFIG)
#error "R6af unconditional crash task ID requires trusted generated current-task domain"
#endif

#if ARGHTOS_R6AE_DISPATCH_OWNS_YIELD_READY && \
    (!ARGHTOS_R6N_OK_ONLY_SERVICES || !ARGHTOS_R5N_PROVEN_DISPATCH || \
     !ARGHTOS_PRODUCTION_PROFILE || !ARGHTOS_MINIMAL_VALIDATION)
#error "R6ae deferred YIELD requires trusted dispatch-owned service profile"
#endif

#if ARGHTOS_R6AD_ALWAYS_DISPATCH_WAKE && \
    (!ARGHTOS_R6AB_SINGLE_WAKE_TARGET || !ARGHTOS_R6N_OK_ONLY_SERVICES || \
     !ARGHTOS_R6J_PROVEN_WAKE_DOMAIN || !ARGHTOS_R6D_SERIAL_HANDLER_MUTATION)
#error "R6ad unconditional wake dispatch requires trusted singleton wake profile"
#endif

#ifndef ARGHTOS_R6AC_PORT_FORMAT_VALUE
#define ARGHTOS_R6AC_PORT_FORMAT_VALUE 0u
#endif

#if ARGHTOS_R6AC_PRESEEDED_PORT_FORMAT && \
    (!ARGHTOS_R6Z_TRUSTED_CRASH_PAYLOAD || !ARGHTOS_R5S_PRESEEDED_CRASH || \
     ARGHTOS_R6AC_PORT_FORMAT_VALUE == 0u || \
     ARGHTOS_R6AC_PORT_FORMAT_VALUE > 0xFFFFu)
#error "R6ac preseeded port format requires a valid trusted selected-port format"
#endif

#if ARGHTOS_R6AB_SINGLE_WAKE_TARGET && \
    (!ARGHTOS_R5AB_SINGLE_ASYNC || !ARGHTOS_R5P_GENERATED_ORDER || \
     !ARGHTOS_R6N_OK_ONLY_SERVICES || !ARGHTOS_R6J_PROVEN_WAKE_DOMAIN || \
     (ARGHTOS_STATIC_TASK_COUNT - ARGHTOS_R5P_PERIODIC_COUNT) != 1)
#error "R6ab singleton wake target requires exactly one trusted generated async task"
#endif

#if ARGHTOS_R6Z_TRUSTED_CRASH_PAYLOAD && \
    (!ARGHTOS_R6M_COMMIT_RECURSION_GUARD || !ARGHTOS_R5S_PRESEEDED_CRASH || \
     !ARGHTOS_R5N_PROVEN_DISPATCH || !ARGHTOS_STATIC_CONFIG)
#error "R6z trusted crash payload requires the preseeded generated R6m profile"
#endif

#if ARGHTOS_R6Y_PACKED_MISSED_ADD && \
    (!ARGHTOS_R6V_ALWAYS_DISPATCH_TICK || !ARGHTOS_COMPACT_RUNTIME)
#error "R6y packed missed add requires the compact trusted R6v profile"
#endif

#if ARGHTOS_R6X_BOUNDED_MISSED_LOOP && \
    (!ARGHTOS_R6V_ALWAYS_DISPATCH_TICK || !ARGHTOS_COMPACT_RUNTIME)
#error "R6x bounded missed loop requires the compact trusted R6v profile"
#endif

#if ARGHTOS_R6W_PERIODIC_READY_NORMALIZE && \
    (!ARGHTOS_R6V_ALWAYS_DISPATCH_TICK || !ARGHTOS_R5Y_PROVEN_PERIODIC_STATE)
#error "R6w periodic READY normalization requires R6v and proven periodic states"
#endif

#if ARGHTOS_R6V_ALWAYS_DISPATCH_TICK && \
    (!ARGHTOS_R5V_UNIT_PERIOD_TIMER || !ARGHTOS_R5R_ELIDE_AGE_RESETS || \
     !ARGHTOS_R5N_PROVEN_DISPATCH || ARGHTOS_RUNTIME_PROFILING)
#error "R6v unconditional timer dispatch requires unit-tick no-profile trusted profile"
#endif

#if ARGHTOS_R6R_FLAGLESS_KERNEL_STATE && \
    (!ARGHTOS_R6P_INFALLIBLE_PORT_INIT || !ARGHTOS_R5N_PROVEN_DISPATCH || \
     !ARGHTOS_R6B_TRUSTED_TIMER_ENTRY || !ARGHTOS_R6M_COMMIT_RECURSION_GUARD || \
     !ARGHTOS_MINIMAL_VALIDATION || !ARGHTOS_STATIC_CONFIG)
#error "R6r flagless kernel state requires fully discharged trusted minimum profile"
#endif

#if ARGHTOS_R6Q_PRESEEDED_PERIOD_OFFSETS && \
    (!ARGHTOS_R6K_PRESEEDED_PERIODIC_READY || !ARGHTOS_R5L_PREBUILT_CONTEXT || \
     !ARGHTOS_R5P_GENERATED_ORDER || !ARGHTOS_R5E_AGE_NEXT_OVERLAY)
#error "R6q period offsets require generated prebuilt periodic startup"
#endif

#if ARGHTOS_R6P_INFALLIBLE_PORT_INIT && \
    (!ARGHTOS_R6O_PRESEEDED_CURRENT_INVALID || !ARGHTOS_R5M_PRISTINE_INIT || \
     !ARGHTOS_STATIC_CONFIG || !ARGHTOS_PRODUCTION_PROFILE)
#error "R6p infallible port init requires preseeded generated minimum profile"
#endif

#if ARGHTOS_R6O_PRESEEDED_CURRENT_INVALID && \
    (!ARGHTOS_R5M_PRISTINE_INIT || !ARGHTOS_STATIC_CONFIG || \
     !ARGHTOS_SIZE_FIRST_NO_START_TIME)
#error "R6o preseeded current task requires pristine static minimum profile"
#endif

#if ARGHTOS_R6N_OK_ONLY_SERVICES && \
    (!ARGHTOS_R6F_TRUSTED_SERVICE_IDS || !ARGHTOS_R6J_PROVEN_WAKE_DOMAIN || \
     !ARGHTOS_R5D_COMPACT_SERVICE_OUTCOME || !ARGHTOS_MINIMAL_VALIDATION || \
     !ARGHTOS_SPECIALIZED_SERVICES)
#error "R6n OK-only services require trusted closed minimum service domain"
#endif

#if ARGHTOS_R6M_COMMIT_RECURSION_GUARD && \
    (!ARGHTOS_R5S_PRESEEDED_CRASH || !ARGHTOS_R5AC_TERMINAL_NO_FAULT_STATE || \
     ARGHTOS_R5W_TRUSTED_CRASH_PORT || !ARGHTOS_STATIC_CONFIG)
#error "R6m commit recursion guard requires preseeded terminal static crash profile"
#endif

#define ARGHTOS_R6M_CAPTURE_IN_PROGRESS 1u

#if ARGHTOS_R6L_PREMASKED_START && \
    (!ARGHTOS_R6K_PRESEEDED_PERIODIC_READY || !ARGHTOS_R5N_PROVEN_DISPATCH || \
     !ARGHTOS_R6B_TRUSTED_TIMER_ENTRY || !ARGHTOS_PRODUCTION_PROFILE)
#error "R6l pre-masked start requires trusted generated startup profile"
#endif

#if ARGHTOS_R6K_PRESEEDED_PERIODIC_READY && \
    (!ARGHTOS_R5M_PRISTINE_INIT || !ARGHTOS_R5N_PROVEN_DISPATCH || \
     !ARGHTOS_R5P_GENERATED_ORDER || !ARGHTOS_R5E_AGE_NEXT_OVERLAY || \
     !ARGHTOS_MINIMAL_VALIDATION)
#error "R6k preseeded periodic READY requires trusted generated pristine profile"
#endif

#if ARGHTOS_R6J_PROVEN_WAKE_DOMAIN && \
    (!ARGHTOS_R5AC_TERMINAL_NO_FAULT_STATE || !ARGHTOS_R5N_PROVEN_DISPATCH || \
     !ARGHTOS_MINIMAL_VALIDATION || !ARGHTOS_SPECIALIZED_SERVICES)
#error "R6j proven wake domain requires terminal trusted generated profile"
#endif

#if ARGHTOS_R6H_SINGLE_ASYNC_TIME && \
    (!ARGHTOS_R5AB_SINGLE_ASYNC || !ARGHTOS_R5Q_PARTITIONED_TIME || \
     !ARGHTOS_R5P_GENERATED_ORDER || \
     (ARGHTOS_STATIC_TASK_COUNT - ARGHTOS_R5P_PERIODIC_COUNT) != 1)
#error "R6h singleton async time scan requires exactly one generated async task"
#endif

#if ARGHTOS_R6G_PROVEN_CURRENT_DOMAIN && \
    (!ARGHTOS_R5N_PROVEN_DISPATCH || !ARGHTOS_MINIMAL_VALIDATION || \
     !ARGHTOS_R5AC_TERMINAL_NO_FAULT_STATE || !ARGHTOS_R5S_PRESEEDED_CRASH || \
     !ARGHTOS_SPECIALIZED_SERVICES || ARGHTOS_RUNTIME_PROFILING)
#error "R6g proven current-task domain requires the trusted generated minimum profile"
#endif

#if ARGHTOS_R5AC_TERMINAL_NO_FAULT_STATE && \
    (!ARGHTOS_R5N_PROVEN_DISPATCH || !ARGHTOS_MINIMAL_VALIDATION)
#error "R5ac terminal fault-state elision requires trusted generated minimum profile"
#endif

#ifndef ARGHTOS_R5Q_PARTITIONED_TIME
#define ARGHTOS_R5Q_PARTITIONED_TIME 0
#endif

#if ARGHTOS_R5Q_PARTITIONED_TIME && !ARGHTOS_R5P_GENERATED_ORDER
#error "R5q partitioned time scan requires accepted R5p ordering"
#endif

#ifndef ARGHTOS_R5R_ELIDE_AGE_RESETS
#define ARGHTOS_R5R_ELIDE_AGE_RESETS 0
#endif

#if ARGHTOS_R5R_ELIDE_AGE_RESETS && !ARGHTOS_R5Q_PARTITIONED_TIME
#error "R5r age-reset elision requires accepted R5q generated profile"
#endif

#ifndef ARGHTOS_R5S_PRESEEDED_CRASH
#define ARGHTOS_R5S_PRESEEDED_CRASH 0
#endif

#ifndef ARGHTOS_R5AD_COMPACT_API_VENEERS
#define ARGHTOS_R5AD_COMPACT_API_VENEERS 0
#endif

#if ARGHTOS_R5AD_COMPACT_API_VENEERS && !ARGHTOS_PRODUCTION_PROFILE
#error "R5ad compact API veneers require production profile"
#endif

#if ARGHTOS_R5S_PRESEEDED_CRASH && \
    (!ARGHTOS_STATIC_CONFIG || !ARGHTOS_MINIMAL_VALIDATION || \
     !ARGHTOS_R5L_PREBUILT_CONTEXT)
#error "R5s preseeded crash record requires trusted generated static startup"
#endif

#ifndef ARGHTOS_R5T_TIMER_ALWAYS_ARMED
#define ARGHTOS_R5T_TIMER_ALWAYS_ARMED 0
#endif

#if ARGHTOS_R5T_TIMER_ALWAYS_ARMED && \
    (!ARGHTOS_R5Q_PARTITIONED_TIME || ARGHTOS_R5P_PERIODIC_COUNT < 1u)
#error "R5t always-armed timer requires generated periodic tasks"
#endif

#ifndef ARGHTOS_R5AA_DELTA_ONLY_TIMER
#define ARGHTOS_R5AA_DELTA_ONLY_TIMER 0
#endif

#if ARGHTOS_R5AA_DELTA_ONLY_TIMER && !ARGHTOS_R5T_TIMER_ALWAYS_ARMED
#error "R5aa delta-only timer requires accepted R5t always-armed timer"
#endif

#ifndef ARGHTOS_R5V_UNIT_PERIOD_TIMER
#define ARGHTOS_R5V_UNIT_PERIOD_TIMER 0
#endif

#if ARGHTOS_R5V_UNIT_PERIOD_TIMER && \
    (!ARGHTOS_R5T_TIMER_ALWAYS_ARMED || ARGHTOS_R5O_MIN_PERIOD != 1u)
#error "R5v unit-period timer requires accepted R5t profile with minimum period 1"
#endif

#ifndef ARGHTOS_R5W_TRUSTED_CRASH_PORT
#define ARGHTOS_R5W_TRUSTED_CRASH_PORT 0
#endif

#if ARGHTOS_R5W_TRUSTED_CRASH_PORT && \
    (!ARGHTOS_R5S_PRESEEDED_CRASH || !ARGHTOS_R5N_PROVEN_DISPATCH)
#error "R5w trusted crash port requires accepted R5s and R5n profiles"
#endif

#ifndef ARGHTOS_R5Y_PROVEN_PERIODIC_STATE
#define ARGHTOS_R5Y_PROVEN_PERIODIC_STATE 0
#endif

#if ARGHTOS_R5Y_PROVEN_PERIODIC_STATE && \
    (!ARGHTOS_R5Q_PARTITIONED_TIME || !ARGHTOS_R5N_PROVEN_DISPATCH)
#error "R5y periodic-state proof requires generated partitioned R5n profile"
#endif

#if ARGHTOS_R5N_PROVEN_DISPATCH && !ARGHTOS_R5M_PRISTINE_INIT
#error "R5n proven dispatch requires the trusted R5m generated profile"
#endif

#if ARGHTOS_R5M_PRISTINE_INIT && \
    (!ARGHTOS_R5L_PREBUILT_CONTEXT || !ARGHTOS_TRUSTED_CONFIG || \
     !ARGHTOS_MINIMAL_VALIDATION || !ARGHTOS_STATIC_CONFIG)
#error "R5m pristine init requires trusted static R5l minimum profile"
#endif

#if ARGHTOS_R5L_PREBUILT_CONTEXT && \
    (!ARGHTOS_STATIC_CONFIG || !ARGHTOS_UNIFIED_SLOT || \
     !ARGHTOS_ENTRY_NEXT_OVERLAY || !ARGHTOS_MINIMAL_VALIDATION)
#error "R5l prebuilt context requires trusted static unified minimum profile"
#endif

#ifndef ARGHTOS_R5I_SCHEDULE8
#define ARGHTOS_R5I_SCHEDULE8 0
#endif

#ifndef ARGHTOS_R5I_SCHEDULE_OVERFLOW_COUNT
#define ARGHTOS_R5I_SCHEDULE_OVERFLOW_COUNT 0u
#endif

#if ARGHTOS_R5I_SCHEDULE8 && \
    (!ARGHTOS_UNIFIED_SLOT || !ARGHTOS_R5E_AGE_NEXT_OVERLAY)
#error "R5i schedule8 requires the accepted R5e unified profile"
#endif

#if ARGHTOS_R5I_SCHEDULE8 && \
    (ARGHTOS_R5F_COMPRESSED_SCHEDULE || ARGHTOS_R5G_SCHEDULE16)
#error "R5i schedule8 is mutually exclusive with earlier schedule encodings"
#endif

#if ARGHTOS_R5I_SCHEDULE_OVERFLOW_COUNT > ARGHTOS_MAX_TASKS
#error "R5i schedule overflow count exceeds task capacity"
#endif

#define ARGHTOS_R5I_OVERFLOW_BASE 0x60u
#define ARGHTOS_R5I_OVERFLOW_LAST 0x77u
#define ARGHTOS_R5I_ASYNC_BASE    0x80u

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
#if !ARGHTOS_R5F_COMPRESSED_SCHEDULE && !ARGHTOS_R5G_SCHEDULE16 && !ARGHTOS_R5I_SCHEDULE8
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
#if !ARGHTOS_R6R_FLAGLESS_KERNEL_STATE
#if !ARGHTOS_STATIC_CONFIG
    uint8_t                      task_count;
#endif
    uint8_t                      flags;
#if ARGHTOS_STATIC_CONFIG
    uint8_t                      reserved[2];
#else
    uint8_t                      reserved;
#endif
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
#elif ARGHTOS_R5I_SCHEDULE8
_Static_assert(sizeof(arghtos_task_control_t) == 8u, "32-bit R5i core slot ABI changed");
_Static_assert(sizeof(arghtos_task_meta_t) == 1u, "R5i metadata ABI changed");
#elif ARGHTOS_R5G_SCHEDULE16
_Static_assert(sizeof(arghtos_task_control_t) == 8u, "32-bit R5g core slot ABI changed");
_Static_assert(sizeof(arghtos_task_meta_t) == 1u, "R5g metadata ABI changed");
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
#if ARGHTOS_R6R_FLAGLESS_KERNEL_STATE
_Static_assert(sizeof(arghtos_kernel_state_t) == 1u, "32-bit flagless minimum kernel state ABI changed");
#else
_Static_assert(sizeof(arghtos_kernel_state_t) == 4u, "32-bit minimum-size kernel state ABI changed");
#endif
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
