#ifndef MPU_6050_H
#define MPU_6050_H
#include <stdint.h>
#include <stdbool.h>


#define MPU6050_ADDR                      (0x68)


typedef enum
{
    SENSOR_ACCEL = 0,
    SENSOR_TEMP,
    SENSOR_GYRO,
    SENSOR_COUNT
} MPU6050_Sensor_t;

typedef struct
{
    uint8_t x;
    uint8_t y;
    uint8_t z;
} AxisStruct_t;

typedef struct
{
    AxisStruct_t accel;
    AxisStruct_t gyro;
} MPU6050_t;


bool MPU6050_Init(void);
bool MPU6050_AccelRead(AxisStruct_t    *pAccel);
bool MPU6050_GyroRead(AxisStruct_t     *pGyro);
bool MPU6050_SensorsRead(AxisStruct_t  *pAccel,
                         AxisStruct_t  *pGyro);

#endif /* MPU_6050_H */
