#include <stdbool.h>
#include <math.h>

#include "RTOS.h"

#include "mpu-6050.h"
#include "twi.h"
#include "logWriter.h"

#define MPU6050_ADDR                      (0x68)

#define SELF_TEST_RESPONSE_MIN            (-14)
#define SELF_TEST_RESPONSE_MAX            (14)

/* Memory Map */
#define MPU6050_SELF_TEST_X               (0x0D) /* Read Only   */
#define MPU6050_SELF_TEST_Y               (0x0E) /* Read Only   */
#define MPU6050_SELF_TEST_Z               (0x0F) /* Read Only   */
#define MPU6050_SELF_TEST_A               (0x10) /* Read Only   */
#define MPU6050_SMPRT_DIV                 (0x19) /* Read/Write  */
#define MPU6050_CONFIG                    (0x1A) /* Read/Write  */
#define MPU6050_GYRO_CONFIG               (0x1B) /* Read/Write  */
#define MPU6050_ACCEL_CONFIG              (0x1C) /* Read/Write  */
#define MPU6050_FIFO_EN                   (0x23) /* Read/Write  */
#define MPU6050_ACCEL_XOUT_H              (0x3B) /* Read Only   */
#define MPU6050_ACCEL_XOUT_L              (0x3C) /* Read Only   */
#define MPU6050_ACCEL_YOUT_H              (0x3D) /* Read Only   */
#define MPU6050_ACCEL_YOUT_L              (0x3E) /* Read Only   */
#define MPU6050_ACCEL_ZOUT_H              (0x3F) /* Read Only   */
#define MPU6050_ACCEL_ZOUT_L              (0x40) /* Read Only   */
#define MPU6050_TEMP_OUT_H                (0x41) /* Read Only   */
#define MPU6050_TEMP_OUT_L                (0x42) /* Read Only   */
#define MPU6050_GYRO_XOUT_H               (0x43) /* Read Only   */
#define MPU6050_GYRO_XOUT_L               (0x44) /* Read Only   */
#define MPU6050_GYRO_YOUT_H               (0x45) /* Read Only   */
#define MPU6050_GYRO_YOUT_L               (0x46) /* Read Only   */
#define MPU6050_GYRO_ZOUT_H               (0x47) /* Read Only   */
#define MPU6050_GYRO_ZOUT_L               (0x48) /* Read Only   */
#define MPU6050_SIGNAL_PATH_RESET         (0x68) /* Write Only  */
#define MPU6050_USER_CTRL                 (0x6A) /* Read/Write  */
#define MPU6050_PWR_MGMT_1                (0x6B) /* Read/Write  */
#define MPU6050_PWR_MGMT_2                (0x6C) /* Read/Write  */
#define MPU6050_FIFO_COUNT_H              (0x72) /* Read Only   */
#define MPU6050_FIFO_COUNT_L              (0x73) /* Read Only   */
#define MPU6050_WHO_AM_I                  (0x75) /* Read Only   */

/* Bit Masks */
#define MPU6050_SELF_TEST_X_XA_TEST           (7u  << 5)
#define MPU6050_SELF_TEST_X_XG_TEST           (31u << 0)
#define MPU6050_SELF_TEST_Y_YA_TEST           (7u  << 5)
#define MPU6050_SELF_TEST_Y_YG_TEST           (31u << 0)
#define MPU6050_SELF_TEST_Z_ZA_TEST           (7u  << 5)
#define MPU6050_SELF_TEST_Z_ZG_TEST           (31u << 0)
#define MPU6050_SELF_TEST_A_XA_TEST           (3u  << 4)
#define MPU6050_SELF_TEST_A_YA_TEST           (3u  << 2)
#define MPU6050_SELF_TEST_A_ZA_TEST           (3u  << 0)

#define MPU6050_SMPRT_DIV_SMPRT_DIV(x)      (((x)  << 0) & 0xFF)

#define MPU6050_CONFIG_EXT_SYNC_DISABLED      (0u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_TEMP_OUT      (1u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_GYRO_XOUT     (2u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_GYRO_YOUT     (3u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_GYRO_ZOUT     (4u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_ACCEL_XOUT    (5u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_ACCEL_YOUT    (6u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_ACCEL_ZOUT    (7u  << 3)
#define MPU6050_CONFIG_DLPF_CFG(x)          (((x)  << 0) & 0x7)

#define MPU6050_GYRO_CONFIG_XG_ST             (1u  << 7)
#define MPU6050_GYRO_CONFIG_YG_ST             (1u  << 6)
#define MPU6050_GYRO_CONFIG_ZG_ST             (1u  << 5)
#define MPU6050_GYRO_CONFIG_FS_SEL_250        (0u  << 3)
#define MPU6050_GYRO_CONFIG_FS_SEL_500        (1u  << 3)
#define MPU6050_GYRO_CONFIG_FS_SEL_1000       (2u  << 3)
#define MPU6050_GYRO_CONFIG_FS_SEL_2000       (3u  << 3)

#define MPU6050_ACCEL_CONFIG_XA_ST            (1u  << 7)
#define MPU6050_ACCEL_CONFIG_YA_ST            (1u  << 6)
#define MPU6050_ACCEL_CONFIG_ZA_ST            (1u  << 5)
#define MPU6050_ACCEL_CONFIG_AFS_SEL_2G       (0u  << 3)
#define MPU6050_ACCEL_CONFIG_AFS_SEL_4G       (1u  << 3)
#define MPU6050_ACCEL_CONFIG_AFS_SEL_8G       (2u  << 3)
#define MPU6050_ACCEL_CONFIG_AFS_SEL_16G      (3u  << 3)

#define MPU6050_FIFO_EN_TEMP_FIFO_EN          (1u  << 7)
#define MPU6050_FIFO_EN_XG_FIFO_EN            (1u  << 6)
#define MPU6050_FIFO_EN_YG_FIFO_EN            (1u  << 5)
#define MPU6050_FIFO_EN_ZG_FIFO_EN            (1u  << 4)
#define MPU6050_FIFO_EN_ACCEL_FIFO_EN         (1u  << 3)
#define MPU6050_FIFO_EN_SLV2_FIFO_EN          (1u  << 2)
#define MPU6050_FIFO_EN_SLV1_FIFO_EN          (1u  << 1)
#define MPU6050_FIFO_EN_SLV0_FIFO_EN          (1u  << 0)

#define MPU6050_SIGNAL_PATH_RESET_GYRO_RESET  (1u  << 2)
#define MPU6050_SIGNAL_PATH_RESET_ACCEL_RESET (1u  << 1)
#define MPU6050_SIGNAL_PATH_RESET_TEMP_RESET  (1u  << 0)

#define MPU6050_USER_CTRL_FIFO_EN             (1u  << 6)
#define MPU6050_USER_CTRL_I2C_MST_EN          (1u  << 5)
#define MPU6050_USER_CTRL_I2C_IF_DIS          (1u  << 4)
#define MPU6050_USER_CTRL_FIFO_RESET          (1u  << 2)
#define MPU6050_USER_CTRL_I2C_MST_RESET       (1u  << 1)
#define MPU6050_USER_CTRL_SIG_COND_RESET      (1u  << 0)

#define MPU6050_PWR_MGMT_1_DEVICE_RESET       (1u  << 7)
#define MPU6050_PWR_MGMT_1_SLEEP              (1u  << 6)
#define MPU6050_PWR_MGMT_1_CYCLE              (1u  << 5)
#define MPU6050_PWR_MGMT_1_TEMP_DIS           (1u  << 3)
#define MPU6050_PWR_MGMT_1_CLKSEL_INT_OSC     (1u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_X_GYRO      (1u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_Y_GYRO      (2u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_Z_GYRO      (3u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_EXT_32KHZ   (4u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_EXT_19MHZ   (5u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_STOP        (7u  << 0)


typedef enum
{
    X_AXIS = 0,
    Y_AXIS,
    Z_AXIS,
    CNT_AXIS
} Axis_t;

typedef enum
{
    SENSOR_ACCEL = 0,
    SENSOR_TEMP,
    SENSOR_GYRO,
    SENSOR_COUNT
} MPU6050_Sensor_t;

static uint32_t ulTestAxisTbl[CNT_AXIS] =
{
    MPU6050_SELF_TEST_X,
    MPU6050_SELF_TEST_Y,
    MPU6050_SELF_TEST_Z
};

static TWI_Adapter xTwiAdap =
{
    .pxInst = TWIHS0,
    .ulAddr = MPU6050_ADDR
};

static bool     MPU6050_SetSampleRate(void);
static uint32_t MPU6050_ObtainFactoryTrim(MPU6050_Sensor_t eSensor,
                                          Axis_t           eAxis);
static int32_t  MPU6050_CalculateAccelFactoryTrim(const uint32_t ulFt,
                                                  const Axis_t   eAxis);
static int32_t  MPU6050_CalculateGyroFactoryTrim(const uint32_t ulFt,
                                                 const Axis_t   eAxis);
static uint8_t  MPU6050_ReadReg(uint8_t ucReg);
static bool     MPU6050_WriteReg(uint8_t ucReg, uint8_t ucVal);
static bool     MPU6050_SelfTest(MPU6050_Sensor_t eSensor);
static bool     MPU6050_SensorRead(AxisStruct_t      *pxAxis,
                                   MPU6050_Sensor_t   eSensor);
static bool     MPU6050_ValidateSelfTest(const uint8_t ucFt,
                                         const uint8_t ucDev);


/**
 * @brief   Initialize MPU-6050.
 *
 * @param   None.
 *
 * @retval  ret   Initialize status.
 */
bool MPU6050_Init(void)
{
    uint8_t ucId;
    bool    status = true;

    /* Bus test */
    ucId = MPU6050_ReadReg(MPU6050_WHO_AM_I);
    if (ucId == 0xFF)
    {
        Journal_vWriteError(MPU6050_ERROR);
        status &= false;
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
    if (status != true)
    {
        Journal_vWriteError(MPU6050_ERROR);
    }

    return status;
}


/**
 * @brief   Read MPU-6050 register.
 *
 * @param   ucReg   Register to read.
 *
 * @retval  ucVal   Register contents.
 *
 * @note    Returns 0xFF on read failure.
 */
static uint8_t MPU6050_ReadReg(uint8_t ucReg)
{
    uint8_t ucVal;

    xTwiAdap.pxMsg[0].pucBuf    = &ucReg;
    xTwiAdap.pxMsg[0].ulLen     = 1;
    xTwiAdap.pxMsg[0].ulFlags   = TWI_WRITE;
    xTwiAdap.pxMsg[1].pucBuf    = &ucVal;
    xTwiAdap.pxMsg[1].ulLen     = 1;
    xTwiAdap.pxMsg[1].ulFlags   = TWI_READ;
    (void)TWI_Xfer(&xTwiAdap, 2);

    return ucVal;
}

/**
 * @brief   Write MPU-6050 register.
 *
 * @param   ucReg     Register to write.
 *
 * @paraml  ucVal     Register contents.
 *
 * @retval  TWI_Xfer  Write status.
 */
static bool MPU6050_WriteReg(uint8_t ucReg, uint8_t ucVal)
{
    uint8_t pucBuf[2] = { ucReg, ucVal };
    xTwiAdap.pxMsg[0].pucBuf    = pucBuf;
    xTwiAdap.pxMsg[0].ulLen     = 2;
    xTwiAdap.pxMsg[0].ulFlags   = TWI_WRITE;
    return TWI_Xfer(&xTwiAdap, 1);
}


/**
 * @brief   Read accelerometer X, Y and Z axes.
 *
 * @param   pxAccel   Pointer to accelerometer struct.
 *
 * @retval  status    Read status.
 */
bool MPU6050_AccelRead(AxisStruct_t *pxAccel)
{
    bool    status;
    uint8_t ucReg     = MPU6050_ACCEL_XOUT_H;
    uint8_t ucRecv[6] = { 0x00 };

    xTwiAdap.pxMsg[0].pucBuf    = &ucReg;
    xTwiAdap.pxMsg[0].ulLen     = 1;
    xTwiAdap.pxMsg[0].ulFlags   = TWI_WRITE;
    xTwiAdap.pxMsg[1].pucBuf    = ucRecv;
    xTwiAdap.pxMsg[1].ulLen     = 6;
    xTwiAdap.pxMsg[1].ulFlags   = TWI_READ;
    status = TWI_Xfer(&xTwiAdap, 2);
    if (status == true)
    {
        pxAccel->ucX = (ucRecv[0] << 4) | ucRecv[1];
        pxAccel->ucY = (ucRecv[2] << 4) | ucRecv[3];
        pxAccel->ucZ = (ucRecv[4] << 4) | ucRecv[5];
    }
    return status;
}


/**
 * @brief   Read gyroscope X, Y and Z axes.
 *
 * @param   pxGyro    Pointer to gyroscope struct.
 *
 * @retval  status    Read status.
 */
bool MPU6050_vGyroRead(AxisStruct_t *pxGyro)
{
    bool    status;
    uint8_t ucReg     = MPU6050_GYRO_XOUT_H;
    uint8_t ucRecv[6] = { 0x00 };

    xTwiAdap.pxMsg[0].pucBuf    = &ucReg;
    xTwiAdap.pxMsg[0].ulLen     = 1;
    xTwiAdap.pxMsg[0].ulFlags   = TWI_WRITE;
    xTwiAdap.pxMsg[1].pucBuf    = ucRecv;
    xTwiAdap.pxMsg[1].ulLen     = 6;
    xTwiAdap.pxMsg[1].ulFlags   = TWI_READ;
    status = TWI_Xfer(&xTwiAdap, 2);
    if (status == true)
    {
        pxGyro->ucX = (ucRecv[0] << 4) | ucRecv[1];
        pxGyro->ucY = (ucRecv[2] << 4) | ucRecv[3];
        pxGyro->ucZ = (ucRecv[4] << 4) | ucRecv[5];
    }
    return status;
}


/**
 * @brief   Read MPU6050 gyroscope and accelerometer
 *          measurements.
 *
 * @param   pxAccel   Pointer to accelerometer struct.
 *
 * @param   pxGyro    Pointer to gyroscope struct.
 *
 * @retval  status    Read status.
 */
bool MPU6050_SensorsRead(AxisStruct_t *pxAccel,
                         AxisStruct_t *pxGyro)
{
    bool    status;
    uint8_t ucReg      = MPU6050_ACCEL_XOUT_H;
    uint8_t ucRecv[14] = { 0x00 };

    xTwiAdap.pxMsg[0].pucBuf    = &ucReg;
    xTwiAdap.pxMsg[0].ulLen     = 1;
    xTwiAdap.pxMsg[0].ulFlags   = TWI_WRITE;
    xTwiAdap.pxMsg[1].pucBuf    = ucRecv;
    xTwiAdap.pxMsg[1].ulLen     = 14;
    xTwiAdap.pxMsg[1].ulFlags   = TWI_READ;
    status = TWI_Xfer(&xTwiAdap, 2);
    if (status == true)
    {
        /* Store accelerometer measurements to it's struct */
        pxAccel->ucX = (ucRecv[0]  << 4) | ucRecv[1];
        pxAccel->ucY = (ucRecv[2]  << 4) | ucRecv[3];
        pxAccel->ucZ = (ucRecv[4]  << 4) | ucRecv[5];

        /* Discard 2 garbage bytes (temperature) */

        /* Store gyroscope measurements to respective struct */
        pxGyro->ucX  = (ucRecv[8]  << 4) | ucRecv[9];
        pxGyro->ucY  = (ucRecv[10] << 4) | ucRecv[11];
        pxGyro->ucZ  = (ucRecv[12] << 4) | ucRecv[13];
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
 * @brief   Run self-test of the selected device.
 *
 * @param   None.
 *
 * @retval  bPass   Self test PASS/FAIL.
 */
static bool MPU6050_SelfTest(MPU6050_Sensor_t eSensor)
{
    AxisStruct_t    xDev = { 0 };
    AxisStruct_t    xFT  = { 0 };
    bool            bPass = true;
    
    /* Self-test enable mask is the same for both
     * the gyroscope and the accelerometer
     */
    uint32_t const  ulTestMask = MPU6050_ACCEL_CONFIG_XA_ST |
                                 MPU6050_ACCEL_CONFIG_YA_ST |
                                 MPU6050_ACCEL_CONFIG_ZA_ST;

    uint32_t const pulConfTbl[SENSOR_COUNT][2] =
    {
        { MPU6050_ACCEL_CONFIG, MPU6050_ACCEL_CONFIG_AFS_SEL_8G },
        { NULL,                 NULL                            },
        { MPU6050_GYRO_CONFIG,  MPU6050_GYRO_CONFIG_FS_SEL_2000 }
    };

    assert((eSensor == SENSOR_ACCEL) || (eSensor == SENSOR_GYRO));

    MPU6050_WriteReg(pulConfTbl[eSensor][0], ulTestMask | pulConfTbl[eSensor][1]);

    xFT.ucX = MPU6050_ObtainFactoryTrim(eSensor, X_AXIS);
    xFT.ucY = MPU6050_ObtainFactoryTrim(eSensor, Y_AXIS);
    xFT.ucZ = MPU6050_ObtainFactoryTrim(eSensor, Z_AXIS);

    /* Disable self-test mode and read again */
    MPU6050_WriteReg(pulConfTbl[eSensor][0], pulConfTbl[eSensor][1]);
    MPU6050_SensorRead(&xDev, eSensor);

    /* Change from Factory Trim of the Self-Test Response (%) */
    bPass &= MPU6050_ValidateSelfTest(xFT.ucX, xDev.ucX);
    bPass &= MPU6050_ValidateSelfTest(xFT.ucY, xDev.ucY);
    bPass &= MPU6050_ValidateSelfTest(xFT.ucZ, xDev.ucZ);

    return bPass;
}


/**
 * @brief   Calculate Change from Factory Trim
 *          of the Self-Test Response (%).
 *
 * @param   ucFt    Factory Trim value.
 *
 * @param   ucDev   Device sensor value.
 *
 * @retval  bPass   Test PASS/FAIL.
 */
static bool MPU6050_ValidateSelfTest(const uint8_t ucFt,
                                     const uint8_t ucDev)
{
    int32_t lTest;
    int32_t lStr;
    bool    bPass = true;

    //assert(ucFt > 0);
    
    lStr  = ucFt - ucDev;
    if (ucFt == 0)
    {
        lTest = 0;
    }
    else
    {
        lTest = (lStr - ucFt) / ucFt;
    }
    if ((lTest >= SELF_TEST_RESPONSE_MAX)
     || (lTest <= SELF_TEST_RESPONSE_MIN))
    {
        bPass = false;
    }

    return bPass;
}


/**
 * @brief   Read MPU-6050 sensor.
 *
 * @param   pxAxis  Pointer to a sensor struct.
 *
 * @param   eSensor Sensor to read.
 *
 * @retval  ret     Read status.
 */
static bool MPU6050_SensorRead(AxisStruct_t    *pxAxis,
                               MPU6050_Sensor_t eSensor)
{
    bool          ret;
    uint8_t const pucTbl[] =
    {
        MPU6050_ACCEL_XOUT_H,
        MPU6050_GYRO_XOUT_H,
        MPU6050_TEMP_OUT_H
    };
    uint32_t const pulLenTbl[SENSOR_COUNT] = { 6, 2, 6 };
    uint8_t pucRecv[6]                     = { 0x00 };

    assert(eSensor < SENSOR_COUNT);

    xTwiAdap.pxMsg[0].pucBuf    = (uint8_t *)&pucTbl[eSensor];
    xTwiAdap.pxMsg[0].ulLen     = 1;
    xTwiAdap.pxMsg[0].ulFlags   = TWI_WRITE;
    xTwiAdap.pxMsg[1].pucBuf    = pucRecv;
    xTwiAdap.pxMsg[1].ulLen     = pulLenTbl[eSensor];
    xTwiAdap.pxMsg[1].ulFlags   = TWI_READ;
    ret = TWI_Xfer(&xTwiAdap, 2);
    if (ret == true)
    {
        pxAxis->ucX = (pucRecv[0] << 4) | pucRecv[1];
        pxAxis->ucY = (pucRecv[2] << 4) | pucRecv[3];
        pxAxis->ucZ = (pucRecv[4] << 4) | pucRecv[5];
    }

    return ret;
}


/**
 * @brief   Obtain axis factory trim value.
 *
 * @param   eAxis   Target axis.
 *
 * @retval  return  Factory Trimmed value.
 */
static uint32_t MPU6050_ObtainFactoryTrim(MPU6050_Sensor_t eSensor,
                                          Axis_t           eAxis)
{
    typedef int32_t (*CalculateFactoryTrim_t)(const uint32_t, const Axis_t);
    uint32_t ulAxisTest;
    uint32_t ulTrim            = 0;
    const uint8_t pucMaskTbl[] =
    {
        MPU6050_SELF_TEST_X_XA_TEST, 0, MPU6050_SELF_TEST_X_XG_TEST
    };
    const CalculateFactoryTrim_t plTrimTbl[SENSOR_COUNT] =
    {
        &MPU6050_CalculateAccelFactoryTrim,
        NULL,
        &MPU6050_CalculateGyroFactoryTrim
    };

    /* Sanity check */
    assert((eSensor == SENSOR_ACCEL) || (eSensor == SENSOR_GYRO));
    assert(eAxis    <  CNT_AXIS);

    /* Read MPU6050 register here */
    ulAxisTest  = MPU6050_ReadReg(ulTestAxisTbl[eAxis]);
    ulAxisTest &= pucMaskTbl[eSensor];
    if (ulAxisTest > 0)
    {
        if (plTrimTbl[eSensor] != NULL)
        {
            ulTrim = plTrimTbl[eSensor](ulAxisTest, eAxis);
        }
    }

    return ulTrim;
}


/**
 * @brief   Calculate the accelerometer Factory Trim (FT) value
 *          according the manual.
 *
 * @param   ulFt    Raw Factory Trim value from MPU-60X0.
 *
 * @retval  return  Factory Trimmed value.
 *
 * @note    ulTrim = 4096 * 0.34 * (0.92/0.34)^((xA_TEST-1)/(2^5)-2)
 */
static int32_t MPU6050_CalculateAccelFactoryTrim(const uint32_t ulFt,
                                                 const Axis_t   eAxis)
{
    (void)eAxis;
    uint32_t ulTrim = 0;
    if (ulFt > 0)
    {
        ulTrim = (1392.64 * pow(0.3218, ((ulFt) - 1) / 30));
    }

    return ulTrim;
}


/**
 * @brief   Calculate the gyroscope Factory Trim (FT) value
 *          according the manual.
 *
 * @param   ulFt    Raw Factory Trim value from MPU-60X0.
 *
 * @retval  return  Factory Trimmed value.
 *
 * @note    ulTrim = 25 * 131 * 1.046^(xG_TEST - 1)
 */
static int32_t MPU6050_CalculateGyroFactoryTrim(const uint32_t ulFt,
                                                const Axis_t   eAxis)
{
    int32_t lTrim = 0;
    if (ulFt > 0)
    {
        lTrim = 3275 * pow(1.046, ulFt - 1);
    }
    if (eAxis == Y_AXIS)
    {
        lTrim = -lTrim;
    }

    return lTrim;
}
