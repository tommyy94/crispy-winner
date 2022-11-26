#include "distance.h"
#include "RTOS.h"
#include "srf05.h"
#include "logWriter.h"
#include <stdio.h>
#include "SEGGER_RTT.h"


#define MEAS_INTERVAL_LIMIT_MS  (SRF05_MEAS_INTERVAL_LIMIT_MS)


/**
 * @brief   Measure distance with Ultrasonic Range Finder.
 *
 * @param   arg     Unused.
 *
 * @retval  None.
 */
void Distance_Task(void *arg)
{
    (void)arg;
    float     distanceCm;
    bool      status;

    SRF05_Init();

    while (1)
    {
        status = SRF05_MeasureDistance(&distanceCm);
        if (status == false)
        {
            SRF05_StopMeasuring();
            Journal_vWriteError(SRF05_ERROR);
        }
        else
        {
            /* Pass message forward here */
            printf("Distance: %.2f cm\r\n", distanceCm);
        }

        OS_TASK_Delay(MEAS_INTERVAL_LIMIT_MS);
    }
}
