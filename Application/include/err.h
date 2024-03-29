#ifndef LOGWRITER_H_
#define LOGWRITER_H_

#include <stdint.h>
#include <stdbool.h>


#define assert(x)   err_assert(x, __FUNCTION__, __LINE__)


typedef enum
{
    SPI_ERROR = 0,
    SPI_SELFTEST_FAIL,
    RTC_SETTIME_ERROR,
    DMA_ERROR,
    RTOS_ERROR,
    THROTTLE_TIMEOUT,
    I2C_ERROR,
    MPU6050_ERROR,
    SRF05_ERROR,
    TC_ERROR,
    ERROR_COUNT
} Error_t;


typedef struct
{
    uint32_t uptime;            /* Seconds      */
    uint32_t battery;           /* Percentage   */
    uint32_t totalDistance;     /* Kilometers   */
    uint32_t dataTransferred;   /* KB           */
} Diagnostics_t;


void err_Task(void *arg);
void err_report(Error_t error);
void err_assert(bool eval, const char *func, uint32_t line);


#endif /* LOGWRITER_H_ */
