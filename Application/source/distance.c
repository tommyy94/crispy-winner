#include "distance.h"
#include "RTOS.h"
#include "srf05.h"
#include "logWriter.h"


#define DISTANCE_MEAS_PERIOD_MS   (50)


void Distance_Task(void *arg)
{
    (void)arg;
    uint32_t  distanceCm;
    bool      status;

    SRF05_Init();

    while (1)
    {
        status = SRF05_MeasureDistance(&distanceCm);
        if (status == false)
        {
            Journal_vWriteError(SRF05_ERROR);
        }

        /* Pass message forward here */

        OS_TASK_Delay(DISTANCE_MEAS_PERIOD_MS);
    }
}