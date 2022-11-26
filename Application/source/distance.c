#include "distance.h"
#include "RTOS.h"
#include "srf05.h"
#include "logWriter.h"


#define DISTANCE_MEAS_PERIOD_MS   (100)


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

        /* Pass message forward here */

        OS_TASK_Delay(DISTANCE_MEAS_PERIOD_MS);
    }
}
