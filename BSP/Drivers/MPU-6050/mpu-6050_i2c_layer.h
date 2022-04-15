#ifndef MPU_6050_I2C_LAYER_H
#define MPU_6050_I2C_LAYER_H
#include "mpu-6050.h"

uint8_t  MPU6050_ReadReg(uint8_t reg, uint8_t *pVal);
bool     MPU6050_WriteReg(uint8_t reg, uint8_t val);
bool     MPU6050_SensorRead(AxisStruct_t      *pxAxis,
                            MPU6050_Sensor_t   sensor);
                                   
#endif /* MPU_6050_I2C_LAYER_H */
