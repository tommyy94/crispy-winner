#include "same70.h"
#include "RTOS.h"
#include "SEGGER_RTT.h"
#include "system.h"
#include "err.h"
#include "rtc.h"
#include "trace.h"


#define ASSERT_MSG_LIMIT    (128u)


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
        "Error > SPI_ERROR\r\n",
        "Error > SPI_SELFTEST_FAIL\r\n",
        "Error > RTC_SETTIME_ERROR\r\n",
        "Error > DMA_ERROR\r\n",
        "Error > RTOS_ERROR\r\n",
        "Error > THROTTLE_TIMEOUT\r\n",
        "Error > I2C_ERROR\r\n",
        "Error > MPU6050_ERROR\r\n",
        "Error > SRF05_ERROR\r\n"
    };

    for (uint32_t i = 0; i < ERROR_COUNT; i++)
    {
        if (id & (1 << i))
        {
            TRACE_ERROR(enumTbl[i]);
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
void err_Task(void *arg)
{
    (void)arg;
    uint32_t evtMask;
    
    while (1)
    {
        evtMask = OS_TASKEVENT_GetSingleBlocked(0xFFFFFFFF);
        assert(evtMask);
        assert(evtMask < (1 << ERROR_COUNT));
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
        TRACE_INFO(
            "Info > Assert @ Function: %s(), line: %u\r\n",
            func,
            line
        );
    }
}
