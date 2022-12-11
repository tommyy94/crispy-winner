/** @file */
#include "delay.h"
#include "RTOS.h"


/**
 * Non-blocking microseconds delay.
 *
 * @param[in] ms  Delay in microseconds.
 */
void delayUs(const uint32_t us)
{
    OS_TASK_Delay_us(us);
}


/**
 * Non-blocking milliseconds delay.
 *
 * @param[in] ms  Delay in milliseconds.
 */
void delayMs(const uint32_t ms)
{
    OS_TASK_Delay(ms);
}
