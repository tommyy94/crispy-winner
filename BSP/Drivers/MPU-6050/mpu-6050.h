#ifndef MPU_6050_H
#define MPU_6050_H
#include <stdint.h>
#include <stdbool.h>


typedef struct
{
    uint8_t ucX;
    uint8_t ucY;
    uint8_t ucZ;
} AxisStruct_t;

typedef struct
{
    AxisStruct_t xAccel;
    AxisStruct_t xGyro;
} MPU6050_t;


bool MPU6050_Init(void);
bool MPU6050_AccelRead(AxisStruct_t    *pxAccel);
bool MPU6050_GyroRead(AxisStruct_t     *pxGyro);
bool MPU6050_SensorsRead(AxisStruct_t  *pxAccel,
                          AxisStruct_t  *pxGyro);

#endif /* MPU_6050_H */
