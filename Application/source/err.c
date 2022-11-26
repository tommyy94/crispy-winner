#include <stdio.h>
#include "same70.h"
#include "RTOS.h"
#include "SEGGER_RTT.h"
#include "system.h"
#include "err.h"
#include "rtc.h"


extern OS_TASK errTCB;


/*
 * @brief   Log error to file system/terminal.
 *
 * @param   id    Error id.
 *
 * @return  None.
 */
static void err_log(uint32_t id)
{
    char *enumTbl[ERROR_COUNT] =
    {
        "SPI_ERROR",
        "SPI_SELFTEST_FAIL",
        "RTC_SETTIME_ERROR",
        "DMA_ERROR",
        "RTOS_ERROR",
        "RF_ERROR",
        "RF_BAD_JOB",
        "JOB_QUEUE_FULL",
        "THROTTLE_TIMEOUT",
        "I2C_ERROR",
        "MPU6050_ERROR",
        "SRF05_ERROR"
    };

    for (uint32_t i = 0; i < ERROR_COUNT; i++)
    {
        if (id & (1 << i))
        {
            puts(enumTbl[i]);
            puts("\n\r");
        }
    }
}


/*
 * @brief   Task responsible for recording errors.
 *
 * @param   arg   Unused.
 *
 * @return  None.
 */
void err_task(void *arg)
{
    (void)arg;
    Error_t evtMask;
    
    while (1)
    {
        evtMask = (Error_t)OS_TASKEVENT_GetBlocked(0xFFFFFFFF);
        err_log(evtMask);
    }
}


/*
 * @brief   Report error.
 *
 * @param   ulError   Error id to report.
 *
 * @return  None.
 */
void err_report(Error_t error)
{
    assert(error < ERROR_COUNT);
    OS_TASKEVENT_Set(&errTCB, 1 << error);
}


/*
 * @brief   Assert.
 *
 * @param   eval    Statement.
 *
 * @param   func    Function name (string) where assert occurred.
 *
 * @param   line    Line where assert occurred.
 *
 * @return  None.
 */
void err_assert(bool           eval,
                const char    *func,
                uint32_t       line)
{
    if (eval == false)
    {
        printf(
            "Assert:\n\r"
            "    Function: %s()\n\r"
            "    Line: %u\n\r",
            func,
            line
        );
    }
}