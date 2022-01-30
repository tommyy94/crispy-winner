#ifndef LOGWRITER_H_
#define LOGWRITER_H_

#include <stdint.h>
#include <stdbool.h>


#define assert(x)   Journal_vAssert(x, __FILE__, __LINE__)


typedef enum
{
    SPI_ERROR           = (1u <<  0),
    SPI_SELFTEST_FAIL   = (1u <<  1),
    RTC_SETTIME_ERROR   = (1u <<  2),
    DMA_ERROR           = (1u <<  3),
    RTOS_ERROR          = (1u <<  4),
    RF_ERROR            = (1u <<  5),
    RF_BAD_JOB          = (1u <<  6),
    JOB_QUEUE_FULL      = (1u <<  7),
    THROTTLE_TIMEOUT    = (1u <<  8),
    I2C_ERROR           = (1u <<  9),
    MPU6050_ERROR       = (1u << 10),
    ERROR_COUNT
} Error_t;


typedef struct Diagnostics
{
    uint32_t uptime;            /* Seconds      */
    uint32_t battery;           /* Percentage   */
    uint32_t totalDistance;     /* Kilometers   */
    uint32_t dataTransferred;   /* KB           */
} Diagnostics;


void Journal_vErrorTask(void *arg);
void Journal_vWriteError(Error_t ulError);
void Journal_vAssert(bool bEval, char *pucFile, uint32_t ulLine);


#endif /* LOGWRITER_H_ */
