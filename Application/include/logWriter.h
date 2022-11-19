#ifndef LOGWRITER_H_
#define LOGWRITER_H_

#include <stdint.h>
#include <stdbool.h>


#define assert(x)   Journal_assert(x, __FUNCTION__, __LINE__)


typedef enum
{
    SPI_ERROR = 0,
    SPI_SELFTEST_FAIL,
    RTC_SETTIME_ERROR,
    DMA_ERROR,
    RTOS_ERROR,
    RF_ERROR,
    RF_BAD_JOB,
    JOB_QUEUE_FULL,
    THROTTLE_TIMEOUT,
    I2C_ERROR,
    MPU6050_ERROR,
    SRF05_ERROR,
    TC_ERROR,
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
void Journal_assert(bool eval, const char *func, uint32_t line);


#endif /* LOGWRITER_H_ */
