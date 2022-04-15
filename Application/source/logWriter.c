#include <stdio.h>
#include "same70.h"
#include "RTOS.h"
#include "SEGGER_RTT.h"
#include "system.h"
#include "logWriter.h"
#include "rtc.h"


extern OS_TASK journalTCB;


/*
 * @brief   Log error to file system/terminal.
 *
 * @param   id    Error id.
 *
 * @return  None.
 */
static void logError(uint32_t id)
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
        "MPU6050_ERROR"
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
void Journal_vErrorTask(void *arg)
{
    (void)arg;
    Error_t evtMask;
    
    while (1)
    {
        evtMask = (Error_t)OS_TASKEVENT_GetBlocked(0xFFFFFFFF);
        logError(evtMask);
    }
}


/*
 * @brief   Report error.
 *
 * @param   ulError   Error id to report.
 *
 * @return  None.
 */
void Journal_vWriteError(Error_t ulError)
{
    assert(ulError < ERROR_COUNT);
    OS_TASKEVENT_Set(&journalTCB, 1 << ulError);
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
void Journal_assert(bool           eval,
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
