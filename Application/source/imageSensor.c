#include "RTOS.h"
#include "imageSensor.h"
#include "ov5640.h"


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

    OV5640_Init();

    while (1)
    {
        OS_TASK_Delay(100);
    }
}
