#ifndef ARGHTOS_TEST_H
#define ARGHTOS_TEST_H

#include "../src/arghtos_internal.h"

const arghtos_task_control_t *arghtos_test_controls(void);
const arghtos_kernel_state_t *arghtos_test_kernel(void);
void arghtos_test_reset(void);

#endif
