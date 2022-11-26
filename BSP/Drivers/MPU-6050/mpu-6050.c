#include <stdbool.h>
#include <math.h>

#include "RTOS.h"

#include "mpu-6050.h"
#include "mpu-6050_defs.h"
#include "mpu-6050_i2c_layer.h"
#include "err.h"


#define SELF_TEST_RESPONSE_MIN            (-14)
#define SELF_TEST_RESPONSE_MAX            (14)


typedef enum
{
    X_AXIS = 0,
    Y_AXIS,
    Z_AXIS,
    CNT_AXIS
} Axis_t;

static uint32_t utestAxisTbl[CNT_AXIS] =
{
    MPU6050_SELF_TEST_X,
    MPU6050_SELF_TEST_Y,
    MPU6050_SELF_TEST_Z
};

static bool     MPU6050_SetSampleRate(void);
static bool     MPU6050_SelfTest(MPU6050_Sensor_t sensor);
static int32_t  MPU6050_ObtainFactoryTrim(MPU6050_Sensor_t sensor,
                                          Axis_t           axis);
static int32_t  MPU6050_CalculateAccelFactoryTrim(const uint32_t ft,
                                                  const Axis_t   axis);
static int32_t  MPU6050_CalculateGyroFactoryTrim(const uint32_t ft,
                                                 const Axis_t   axis);
static bool     MPU6050_ValidateSelfTest(const uint8_t ft,
                                         const uint8_t dev);


/**
 * @brief   Initialize MPU-6050.
 *
 * @param   None.
 *
 * @retval  ret   Initialize status.
 */
bool MPU6050_Init(void)
{
    uint8_t id = 0xFF;
    bool    status;

    /* Bus test */
    status = MPU6050_ReadReg(MPU6050_WHO_AM_I, &id);
    if (id == 0xFF)
    {
        err_report(MPU6050_ERROR);
    }

    /* Reset device just to be safe */
    status &= MPU6050_WriteReg(MPU6050_PWR_MGMT_1, MPU6050_PWR_MGMT_1_DEVICE_RESET);
    status &= MPU6050_WriteReg(MPU6050_USER_CTRL, MPU6050_USER_CTRL_SIG_COND_RESET);

    /* Free to configure the device now */
    status &= MPU6050_SetSampleRate();
    status &= MPU6050_WriteReg(MPU6050_PWR_MGMT_1, MPU6050_PWR_MGMT_1_CLKSEL_X_GYRO);

    /* Perform self-testing */
    status &= MPU6050_SelfTest(SENSOR_ACCEL);
    status &= MPU6050_SelfTest(SENSOR_GYRO);
    if (status == false)
    {
        err_report(MPU6050_ERROR);
    }

    return status;
}


/**
 * @brief   Set sampling rate.
 *
 * @param   None.
 *
 * @retval  status    Setting sample rate pass.
 *
 * @note    SampleRate = GyroscopeOutputRate / (1 + SMPLRT_DIV)
 *
 *          Gyroscope Output Rate = 8 kHz when DLPF disabled,
 *                                  1 kHz when DLPF enabled.
 */
static bool MPU6050_SetSampleRate(void)
{
    bool status;

    /* Set 1 kHz sampling rate */
    status  = MPU6050_WriteReg(MPU6050_SMPRT_DIV, MPU6050_SMPRT_DIV_SMPRT_DIV(1));
    status &= MPU6050_WriteReg(MPU6050_CONFIG, MPU6050_CONFIG_DLPF_CFG(1));
    return status;
}


/**
 * @brief   Calculate Change from Factory Trim
 *          of the Self-Test Response (%).
 *
 * @param   ft    Factory Trim value.
 *
 * @param   dev   Device sensor value.
 *
 * @retval  pass   Test PASS/FAIL.
 */
static bool MPU6050_ValidateSelfTest(const uint8_t ft,
                                     const uint8_t dev)
{
    int32_t test;
    int32_t str;
    bool    pass = true;

    //assert(ft > 0);
    
    str = ft - dev;
    if (ft == 0)
    {
        test = 0;
    }
    else
    {
        test = (str - ft) / ft;
    }
    if ((test >= SELF_TEST_RESPONSE_MAX)
     || (test <= SELF_TEST_RESPONSE_MIN))
    {
        pass = false;
    }

    return pass;
}


/**
 * @brief   Obtain axis factory trim value.
 *
 * @param   axis    Target axis.
 *
 * @retval  return  Factory Trimmed value. -1 if read failed.
 */
static int32_t MPU6050_ObtainFactoryTrim(MPU6050_Sensor_t sensor,
                                          Axis_t           axis)
{
    typedef int32_t (*CalculateFactoryTrim_t)(const uint32_t, const Axis_t);
    bool     status;
    uint32_t axisTest;
    int32_t  trim              = 0;
    const uint8_t pucMaskTbl[] =
    {
        MPU6050_SELF_TEST_X_XA_TEST, 0, MPU6050_SELF_TEST_X_XG_TEST
    };
    const CalculateFactoryTrim_t ptrimTbl[SENSOR_COUNT] =
    {
        &MPU6050_CalculateAccelFactoryTrim,
        NULL,
        &MPU6050_CalculateGyroFactoryTrim
    };

    /* Sanity check */
    assert((sensor == SENSOR_ACCEL) || (sensor == SENSOR_GYRO));
    assert(axis    <  CNT_AXIS);

    /* Read MPU6050 register here */
    status = MPU6050_ReadReg(utestAxisTbl[axis], (uint8_t *)&axisTest);
    if (status == false)
    {
        trim = -1;
    }
    else
    {
        axisTest &= pucMaskTbl[sensor];
        if (axisTest > 0)
        {
            if (ptrimTbl[sensor] != NULL)
            {
                trim = ptrimTbl[sensor](axisTest, axis);
            }
        }
    }

    return trim;
}


/**
 * @brief   Calculate the accelerometer Factory Trim (FT) value
 *          according the manual.
 *
 * @param   ft      Raw Factory Trim value from MPU-60X0.
 *
 * @retval  return  Factory Trimmed value.
 *
 * @note    trim = 4096 * 0.34 * (0.92/0.34)^((xA_TEST-1)/(2^5)-2)
 */
static int32_t MPU6050_CalculateAccelFactoryTrim(const uint32_t ft,
                                                 const Axis_t   axis)
{
    (void)axis;
    uint32_t trim = 0;
    if (ft > 0)
    {
        trim = (1392.64 * pow(0.3218, ((ft) - 1) / 30));
    }

    return trim;
}


/**
 * @brief   Calculate the gyroscope Factory Trim (FT) value
 *          according the manual.
 *
 * @param   ft      Raw Factory Trim value from MPU-60X0.
 *
 * @retval  return  Factory Trimmed value.
 *
 * @note    trim = 25 * 131 * 1.046^(xG_TEST - 1)
 */
static int32_t MPU6050_CalculateGyroFactoryTrim(const uint32_t ft,
                                                const Axis_t   axis)
{
    int32_t trim = 0;
    if (ft > 0)
    {
        trim = 3275 * pow(1.046, ft - 1);
    }
    if (axis == Y_AXIS)
    {
        trim = -trim;
    }

    return trim;
}


/**
 * @brief   Run self-test of the selected device.
 *
 * @param   None.
 *
 * @retval  pass   Self test PASS/FAIL.
 */
static bool MPU6050_SelfTest(MPU6050_Sensor_t sensor)
{
    AxisStruct_t    dev  = { 0 };
    AxisStruct_t    ft   = { 0 };
    bool            pass = true;
    
    /* Self-test enable mask is the same for both
     * the gyroscope and the accelerometer
     */
    uint32_t const  testMask = MPU6050_ACCEL_CONFIG_XA_ST |
                               MPU6050_ACCEL_CONFIG_YA_ST |
                               MPU6050_ACCEL_CONFIG_ZA_ST;

    uint32_t const confTbl[SENSOR_COUNT][2] =
    {
        { MPU6050_ACCEL_CONFIG, MPU6050_ACCEL_CONFIG_AFS_SEL_8G },
        { NULL,                 NULL                            },
        { MPU6050_GYRO_CONFIG,  MPU6050_GYRO_CONFIG_FS_SEL_2000 }
    };

    assert((sensor == SENSOR_ACCEL) || (sensor == SENSOR_GYRO));

    pass &= MPU6050_WriteReg(confTbl[sensor][0], testMask | confTbl[sensor][1]);

    ft.x = MPU6050_ObtainFactoryTrim(sensor, X_AXIS);
    ft.y = MPU6050_ObtainFactoryTrim(sensor, Y_AXIS);
    ft.z = MPU6050_ObtainFactoryTrim(sensor, Z_AXIS);
    if ((ft.x < 0) || (ft.y < 0) || (ft.z < 0))
    {
        pass = false;
    }
    else
    {
        /* Disable self-test mode and read again */
        pass &= MPU6050_WriteReg(confTbl[sensor][0], confTbl[sensor][1]);
        pass &= MPU6050_SensorRead(&dev, sensor);

        /* Change from Factory Trim of the Self-Test Response (%) */
        pass &= MPU6050_ValidateSelfTest(ft.x, dev.x);
        pass &= MPU6050_ValidateSelfTest(ft.y, dev.y);
        pass &= MPU6050_ValidateSelfTest(ft.z, dev.z);
    }

    return pass;
}
