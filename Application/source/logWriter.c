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
    printf("Error mask: %#08x\r\n", id);
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
    OS_TASKEVENT_Set(&journalTCB, ulError);
}


/*
 * @brief   Assert.
 *
 * @param   bEval   Statement.
 *
 * @param   pucFile File pointer where assert occurred.
 *
 * @param   ulLine  Line where assert occurred.
 *
 * @return  None.
 */
void Journal_vAssert(bool bEval, char *pucFile, uint32_t ulLine)
{
    if (bEval == false)
    {
        printf("Assert failed:\nFile: %s\nLine: %ul\r\n", pucFile, ulLine);
    }
}
