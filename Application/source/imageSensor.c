#include "RTOS.h"
#include "imageSensor.h"
#include "same70_camera.h"



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
        OS_TASK_Delay(100);
    }
}
