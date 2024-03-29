#include "gyro.h"
#include "mpu-6050.h"
#include "err.h"
#include "RTOS.h"
#include "trace.h"


#define SUPERVISION_EVT_GYRO    (1u << 0)
extern OS_QUEUE   gyroQ;
extern OS_EVENT   svEvt;


/**
 * @brief   Read accelerometer and gyroscope measurements
 *          and relay the sensor values to the controller.
 *
 * @param   pvArg     Pointer to argument, unused.
 *
 * @return  None.
 */
void gyro_Task(void *pArg)
{
    uint32_t    ret;
    uint32_t    evtMask;
    MPU6050_t   mpu6050;
    (void)pArg;

    MPU6050_Init();

    while (1)
    {
        evtMask = OS_EVENT_GetMaskTimed(&svEvt, SUPERVISION_EVT_GYRO, 10);
        //assert(evtMask == SUPERVISION_EVT_GYRO);

        ret = MPU6050_SensorsRead(&mpu6050.accel, &mpu6050.gyro);
        if (ret == true)
        {   
            TRACE_LOG(
                "MPU-6050:\r\n"
                "  Accelerometer\r\n"
                "    X: %u\r\n"
                "    Y: %u\r\n"
                "    Z: %u\r\n"
                "  Gyroscope\r\n"
                "    X: %u\r\n"
                "    Y: %u\r\n"
                "    Z: %u\r\n",
                mpu6050.accel.x,
                mpu6050.accel.y,
                mpu6050.accel.z,
                mpu6050.gyro.x,
                mpu6050.gyro.y,
                mpu6050.gyro.z
            );
            /*
            ret = OS_QUEUE_Put(&gyroQ, &mpu6050, sizeof(mpu6050));
            if (ret != 0)
            {
                //err_report(JOB_QUEUE_FULL);
            }
            */
        }
    }
}
