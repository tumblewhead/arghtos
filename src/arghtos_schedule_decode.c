#include "arghtos_internal.h"

extern uint8_t arghtos_static_schedule_stream[];

uint32_t arghtos_r5f_schedule_get(arghtos_task_id_t id)
{
    const uint8_t *p = arghtos_static_schedule_stream;
    uint32_t schedule = 0u;
    arghtos_task_id_t i;

    for (i = 0u; i <= id; ++i) {
        const uint8_t code = *p++;

        if (code == ARGHTOS_R5F_SCHEDULE_ESCAPE) {
            schedule = (uint32_t)p[0] |
                       ((uint32_t)p[1] << 8) |
                       ((uint32_t)p[2] << 16) |
                       ((uint32_t)p[3] << 24);
            p += 4;
        } else if ((code & 0x80u) != 0u) {
            schedule = ARGHTOS_STATIC_ASYNC_TAG |
                       (uint32_t)(code & 0x7Fu);
        } else {
            schedule = (uint32_t)code;
        }
    }

    return schedule;
}
