#include "RTOS.h"
#include "imageSensor.h"
#include "same70_camera.h"

#ifdef EVABOARD_WORKAROUND
extern OS_MUTEX evabrdWaMutex;
#endif /* EVABOARD_WORKAROUND */


/**
 * Read image sensor and pass the image forward.
 *
 * @param   arg   Unused.
 *
 * @retval  None.
 */
void Image_Task(void *arg)
{
    (void)arg;

    BSP_CAMERA_Init(CAMERA_INSTANCE, OV5640_R640x480, OV5640_RGB565);
   
    while (1)
    {
#ifdef EVABOARD_WORKAROUND
        OS_MUTEX_LockBlocked(&evabrdWaMutex);
        /* ISI_InitIO(); */
        /* BSP_CAMERA_Read(); */
#endif /* EVABOARD_WORKAROUND */

        /* Do magic here */

#ifdef EVABOARD_WORKAROUND
        OS_MUTEX_Unlock(&evabrdWaMutex);
#endif /* EVABOARD_WORKAROUND */

        OS_TASK_Delay(100);
    }
}
