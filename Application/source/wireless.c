/* System includes */
#include "same70.h"

/* User includes */
#include "wireless.h"

/* RTOS includes */
#include "RTOS.h"


/**
 * @brief   Task responsible for communication with
 *          the remote controller.
 *
 * @param   pvArg   Unused.
 *
 * @return  None.
 */
void Wireless_Task(void *pvArg)
{
    (void)pvArg;

    while (1)
    {
        OS_TASK_Delay(1000);
    }
}
