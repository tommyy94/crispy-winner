#include "mpu-6050.h"
#include "twi.h"
#include "mpu-6050_i2c_layer.h"
#include "mpu-6050_defs.h"
#include "err.h"


static TWI_Adapter twiAdap =
{
    .pInst = TWIHS0,
    .addr  = MPU6050_ADDR
};


/**
 * @brief   Read MPU-6050 sensor.
 *
 * @param   pxAxis  Pointer to a sensor struct.
 *
 * @param   sensor  Sensor to read.
 *
 * @retval  ret     Read status.
 */
bool MPU6050_SensorRead(AxisStruct_t    *pxAxis,
                        MPU6050_Sensor_t sensor)
{
    uint32_t      status;
    uint8_t const sensorTbl[] =
    {
        MPU6050_ACCEL_XOUT_H,
        MPU6050_GYRO_XOUT_H,
        MPU6050_TEMP_OUT_H
    };
    uint32_t const plenTbl[SENSOR_COUNT] = { 6, 2, 6 };
    uint8_t recv[6]                      = { 0x00 };

    assert(sensor < SENSOR_COUNT);

    twiAdap.msgArr[0].pBuf     = (uint8_t *)&sensorTbl[sensor];
    twiAdap.msgArr[0].len     = 1;
    twiAdap.msgArr[0].flags   = TWI_WRITE;
    twiAdap.msgArr[1].pBuf     = recv;
    twiAdap.msgArr[1].len     = plenTbl[sensor];
    twiAdap.msgArr[1].flags   = TWI_READ;
    status  = TWI_Xfer(&twiAdap, 2);
    status &= TWI_SUCCESS;
    if (status == true)
    {
        pxAxis->x = (recv[0] << 4) | recv[1];
        pxAxis->y = (recv[2] << 4) | recv[3];
        pxAxis->z = (recv[4] << 4) | recv[5];
    }

    return status;
}


/**
 * @brief   Read MPU6050 gyroscope and accelerometer
 *          measurements.
 *
 * @param   pAccel   Pointer to accelerometer struct.
 *
 * @param   pGyro    Pointer to gyroscope struct.
 *
 * @retval  status   Read status.
 */
bool MPU6050_SensorsRead(AxisStruct_t *pAccel,
                         AxisStruct_t *pGyro)
{
    uint32_t status;
    uint8_t  reg        = MPU6050_ACCEL_XOUT_H;
    uint8_t  ucRecv[14] = { 0x00 };

    twiAdap.msgArr[0].pBuf     = &reg;
    twiAdap.msgArr[0].len     = 1;
    twiAdap.msgArr[0].flags   = TWI_WRITE;
    twiAdap.msgArr[1].pBuf     = ucRecv;
    twiAdap.msgArr[1].len     = 14;
    twiAdap.msgArr[1].flags   = TWI_READ;
    status  = TWI_Xfer(&twiAdap, 2);
    status &= TWI_SUCCESS;
    if (status == true)
    {
        /* Store accelerometer measurements to it's struct */
        pAccel->x = (ucRecv[0]  << 4) | ucRecv[1];
        pAccel->y = (ucRecv[2]  << 4) | ucRecv[3];
        pAccel->z = (ucRecv[4]  << 4) | ucRecv[5];

        /* Discard 2 garbage bytes (temperature) */

        /* Store gyroscope measurements to respective struct */
        pGyro->x  = (ucRecv[8]  << 4) | ucRecv[9];
        pGyro->y  = (ucRecv[10] << 4) | ucRecv[11];
        pGyro->z  = (ucRecv[12] << 4) | ucRecv[13];
    }

    return status;
}


/**
 * @brief   Read MPU-6050 register.
 *
 * @param   reg     Register to read.
 *
 * @retval  pVal    Pointer to register contents.
 *
 * @retval  status  Register contents.
 *
 * @note    Stores 0xFF in pVal on read failure.
 */
uint8_t MPU6050_ReadReg(uint8_t reg, uint8_t *pVal)
{
    uint32_t status;

    twiAdap.msgArr[0].pBuf     = &reg;
    twiAdap.msgArr[0].len     = 1;
    twiAdap.msgArr[0].flags   = TWI_WRITE;
    twiAdap.msgArr[1].pBuf     = pVal;
    twiAdap.msgArr[1].len     = 1;
    twiAdap.msgArr[1].flags   = TWI_READ;
    status = TWI_Xfer(&twiAdap, 2);

    return status & TWI_SUCCESS;
}


/**
 * @brief   Write MPU-6050 register.
 *
 * @param   reg     Register to write.
 *
 * @paraml  val     Register contents.
 *
 * @retval  TWI_Xfer  Write status.
 */
bool MPU6050_WriteReg(uint8_t reg, uint8_t val)
{
    uint32_t status;
    uint8_t  buf[2] = { reg, val };
    
    twiAdap.msgArr[0].pBuf     = buf;
    twiAdap.msgArr[0].len     = 2;
    twiAdap.msgArr[0].flags   = TWI_WRITE;
    status = TWI_Xfer(&twiAdap, 1);
    
    return status & TWI_SUCCESS;
}


/**
 * @brief   Read accelerometer X, Y and Z axes.
 *
 * @param   pAccel    Pointer to accelerometer struct.
 *
 * @retval  status    Read status.
 */
bool MPU6050_AccelRead(AxisStruct_t *pAccel)
{
    uint32_t status;
    uint8_t  reg       = MPU6050_ACCEL_XOUT_H;
    uint8_t  ucRecv[6] = { 0x00 };

    twiAdap.msgArr[0].pBuf     = &reg;
    twiAdap.msgArr[0].len     = 1;
    twiAdap.msgArr[0].flags   = TWI_WRITE;
    twiAdap.msgArr[1].pBuf     = ucRecv;
    twiAdap.msgArr[1].len     = 6;
    twiAdap.msgArr[1].flags   = TWI_READ;
    status = TWI_Xfer(&twiAdap, 2);
    if (status == true)
    {
        pAccel->x = (ucRecv[0] << 4) | ucRecv[1];
        pAccel->y = (ucRecv[2] << 4) | ucRecv[3];
        pAccel->z = (ucRecv[4] << 4) | ucRecv[5];
    }
    return status;
}


/**
 * @brief   Read gyroscope X, Y and Z axes.
 *
 * @param   pGyro     Pointer to gyroscope struct.
 *
 * @retval  status    Read status.
 */
bool MPU6050_GyroRead(AxisStruct_t *pGyro)
{
    uint32_t status;
    uint8_t  reg       = MPU6050_GYRO_XOUT_H;
    uint8_t  ucRecv[6] = { 0x00 };

    twiAdap.msgArr[0].pBuf     = &reg;
    twiAdap.msgArr[0].len     = 1;
    twiAdap.msgArr[0].flags   = TWI_WRITE;
    twiAdap.msgArr[1].pBuf     = ucRecv;
    twiAdap.msgArr[1].len     = 6;
    twiAdap.msgArr[1].flags   = TWI_READ;
    status  = TWI_Xfer(&twiAdap, 2);
    status &= TWI_SUCCESS;
    if (status == true)
    {
        pGyro->x = (ucRecv[0] << 4) | ucRecv[1];
        pGyro->y = (ucRecv[2] << 4) | ucRecv[3];
        pGyro->z = (ucRecv[4] << 4) | ucRecv[5];
    }
    return status;
}
