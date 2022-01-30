/* Device vendor includes */
#include <same70.h>

/* System includes */
#include <stdbool.h>

/* RTOS includes */
#include "RTOS.h"

/* Application includes */
#include "rtc_driver.h"
#include "system.h"
//#include "ff.h"
#include "logWriter.h"


/* RTC must be read 2-3 times to ensure that two consecutive reads match */
#define STABLE_READ_COUNT   (3u)

#define BCD_SHIFT           (4u)
#define BCD_MASK            (0xFu)
#define BCD_FACTOR          (10)


enum
{
    MONDAY = 1,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY
};


enum
{
    CAL_SECOND,
    CAL_MINUTE,
    CAL_HOUR,
    CAL_DAY,
    CAL_WDAY,
    CAL_MONTH,
    CAL_YEAR,
    CAL_CENTURY
};


extern OS_QUEUE   tsQ;
extern OS_TASK    rtcTCB;

extern void RTC_IRQHandler(void);


static uint32_t ulDec2Bcd(uint32_t value, uint32_t mask);
static uint32_t ulBcd2Dec(uint32_t value, uint32_t mask);
static bool     RTC_bSetTime(Calendar *pxCal);
static bool     RTC_bGetTime(Calendar *pxCal);
static void     RTC_vStart(void);
static void     RTC_vStop(void);


/**
 * @brief   Send second period signal to RTC_vTask.
 *
 * @param   None.
 *
 * @retval  None.
 *
 * @note    PPM and clock correction should be done here.
 *          Reset any correction values and select the
 *          Gregorian calendar with 24-hour mode.
 */
void RTC_vInit(void)
{
    /* Disable RTC interrupts */
    RTC->RTC_IDR = RTC_IDR_TDERRDIS | RTC_IDR_CALDIS
                 | RTC_IDR_TIMDIS   | RTC_IDR_ALRDIS
                 | RTC_IDR_ACKDIS;

    RTC_vStop();

    RTC->RTC_SCCR = RTC_SCCR_TDERRCLR | RTC_SCCR_CALCLR
                  | RTC_SCCR_TIMCLR   | RTC_SCCR_SECCLR
                  | RTC_SCCR_ALRCLR   | RTC_SCCR_ACKCLR;

    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, RTC_IRQ_PRIO);
    NVIC_EnableIRQ(RTC_IRQn);

    /* Enable second periodic interrupt */
    RTC->RTC_IER = RTC_IER_SECEN;
    
    RTC_vStart();
}


/**
 * @brief   Send second period signal to RTC_vTask.
 *
 * @param   None.
 *
 * @retval  None.
 */
void RTC_IRQHandler(void)
{
    uint32_t ulStatus;

    OS_INT_Enter();

    /* Read status register */
    ulStatus = RTC->RTC_SR;

    /* Process IRQ */
    if ((ulStatus & RTC_SR_SEC_SECEVENT) != 0)
    {
        OS_TASKEVENT_Set(&rtcTCB, RTC_SEC_PERIOD);
    }

    /* Clear all status flags */
    RTC->RTC_SCCR = ulStatus;

    OS_INT_Leave();
}


/**
 * @brief   Convert decimal calendar value to BCD.
 *          Resulting BCD is shifted to correct
 *          RTC register bit field.
 *
 * @param   ulVal   Value to convert.
 *
 * @retval  ulMask  Century/year/month/day/wday/hour/min/sec.
 */
static uint32_t ulDec2Bcd(uint32_t ulVal, uint32_t ulMask)
{
    uint32_t ulBcd;

    switch(ulMask)
    {
        case CAL_CENTURY:
            ulBcd = ((ulVal / BCD_FACTOR / BCD_FACTOR / BCD_FACTOR)  << (RTC_CALR_CENT_Pos + BCD_SHIFT)
                |   ((ulVal / BCD_FACTOR / BCD_FACTOR) % BCD_FACTOR) << RTC_CALR_CENT_Pos);
            break;
        case CAL_YEAR:
            ulBcd = (((ulVal / BCD_FACTOR) % BCD_FACTOR) << (RTC_CALR_YEAR_Pos + BCD_SHIFT))
                |   ((ulVal % BCD_FACTOR) << RTC_CALR_YEAR_Pos);
            break;
        case CAL_MONTH:
            ulBcd = ((ulVal / BCD_FACTOR) << (RTC_CALR_MONTH_Pos + BCD_SHIFT))
                |   ((ulVal % BCD_FACTOR) <<  RTC_CALR_MONTH_Pos);
            break;
        case CAL_DAY:
            ulBcd = ((ulVal / BCD_FACTOR) << (RTC_CALR_DATE_Pos + BCD_SHIFT))
                |   ((ulVal % BCD_FACTOR) <<  RTC_CALR_DATE_Pos);
            break;
        case CAL_WDAY:
            ulBcd = ((ulVal / BCD_FACTOR) << (RTC_CALR_DAY_Pos + BCD_SHIFT))
                |   ((ulVal % BCD_FACTOR) <<  RTC_CALR_DAY_Pos);
            break;
        case CAL_HOUR:
            ulBcd = ((ulVal / BCD_FACTOR) << (RTC_TIMR_HOUR_Pos + BCD_SHIFT))
                |   ((ulVal % BCD_FACTOR) <<  RTC_TIMR_HOUR_Pos);
            break;
        case CAL_MINUTE:
            ulBcd = ((ulVal / BCD_FACTOR) << (RTC_TIMR_MIN_Pos + BCD_SHIFT))
                |  ((ulVal % BCD_FACTOR)  <<  RTC_TIMR_MIN_Pos);
            break;
        case CAL_SECOND:
            ulBcd = ((ulVal / BCD_FACTOR) << (RTC_TIMR_SEC_Pos + BCD_SHIFT))
                |   ((ulVal % BCD_FACTOR) <<  RTC_TIMR_SEC_Pos);
            break;
        default:
            ulBcd = 0;
            break;
    }

    return ulBcd;
}


/**
 * @brief   Convert BCD calendar value to decimal.
 *
 * @param   ulVal   Value to convert.
 *
 * @retval  ulMask  Century/year/month/day/wday/hour/min/sec.
 */
static uint32_t ulBcd2Dec(uint32_t ulVal, uint32_t ulMask)
{
    uint32_t ulDec;
    uint32_t ulTmp;

    switch(ulMask)
    {
        case CAL_CENTURY:
            ulTmp = (ulVal & RTC_CALR_CENT_Msk) >> RTC_CALR_CENT_Pos;
            ulDec = (ulTmp >> BCD_SHIFT) * BCD_FACTOR + (ulTmp & BCD_MASK);
            break;
        case CAL_YEAR:
            ulTmp = (ulVal & RTC_CALR_CENT_Msk) >> RTC_CALR_CENT_Pos;
            ulDec = (ulTmp >> BCD_SHIFT) * BCD_FACTOR + (ulTmp & BCD_MASK);
            ulTmp = (ulVal & RTC_CALR_YEAR_Msk) >> RTC_CALR_YEAR_Pos;
            ulDec = (ulDec * BCD_FACTOR * BCD_FACTOR) + (ulTmp >> BCD_SHIFT) * BCD_FACTOR + (ulTmp & BCD_MASK);
            break;
        case CAL_MONTH:
            ulTmp = (ulVal & RTC_CALR_MONTH_Msk) >> RTC_CALR_MONTH_Pos;
            ulDec = (ulTmp >> BCD_SHIFT) * BCD_FACTOR + (ulTmp & BCD_MASK);
            break;
        case CAL_DAY:
            ulTmp = (ulVal & RTC_CALR_DATE_Msk) >> RTC_CALR_DATE_Pos;
            ulDec = (ulTmp >> BCD_SHIFT) * BCD_FACTOR + (ulTmp & BCD_MASK);
            break;
        case CAL_WDAY:
            ulTmp = (ulVal & RTC_CALR_DAY_Msk) >> RTC_CALR_DAY_Pos;
            ulDec = (ulTmp >> BCD_SHIFT) * BCD_FACTOR + (ulTmp & BCD_MASK);
            break;
        case CAL_HOUR:
            ulTmp = (ulVal & RTC_TIMR_HOUR_Msk) >> RTC_TIMR_HOUR_Pos;
            ulDec = (ulTmp >> BCD_SHIFT) * BCD_FACTOR + (ulTmp & BCD_MASK);
            break;
        case CAL_MINUTE:
            ulTmp = (ulVal & RTC_TIMR_MIN_Msk) >> RTC_TIMR_MIN_Pos;
            ulDec = (ulTmp >> BCD_SHIFT) * BCD_FACTOR + (ulTmp & BCD_MASK);
            break;
        case CAL_SECOND:
            ulTmp = (ulVal & RTC_TIMR_SEC_Msk) >> RTC_TIMR_SEC_Pos;
            ulDec = (ulTmp >> BCD_SHIFT) * BCD_FACTOR + (ulTmp & BCD_MASK);
            break;
        default:
            ulDec = 0;
            break;
    }

    return ulDec;
}


/**
 * @brief   Set RTC time.
 *
 * @param   None.
 *
 * @retval  None.
 *
 * @note    This operation must be atomic if
 *          the scheduler is running.
 */
static bool RTC_bSetTime(Calendar *pxCal)
{
    uint32_t ulBcdTime;
    uint32_t ulBcdDate;
    Rtc     *rtc       = RTC;
    bool     bStatus   = true;
    
    /* Prepare time/calendar values */
    ulBcdDate = ulDec2Bcd(pxCal->date.year,    CAL_YEAR)
              | ulDec2Bcd(pxCal->date.month,   CAL_MONTH)
              | ulDec2Bcd(pxCal->date.day,     CAL_DAY)
              | ulDec2Bcd(pxCal->date.weekDay, CAL_WDAY);

    ulBcdTime = ulDec2Bcd(pxCal->time.hour,    CAL_HOUR)
              | ulDec2Bcd(pxCal->time.minutes, CAL_MINUTE)
              | ulDec2Bcd(pxCal->time.seconds, CAL_SECOND);

    /* Wait for periodic event */
    while ((rtc->RTC_SR & RTC_SR_SEC_SECEVENT) == RTC_SR_SEC_NO_SECEVENT)
    {
        ;
    }

    OS_INT_Disable();
    
    RTC_vStop();
    
    /* Wait until command acknowledged */
    while ((rtc->RTC_SR & RTC_SR_ACKUPD) == RTC_SR_ACKUPD_FREERUN)
    {
        ;
    }
    rtc->RTC_SCCR = RTC_SCCR_ACKCLR;
    
    /* Update time/calendar values */
    rtc->RTC_CALR = ulBcdDate;
    rtc->RTC_TIMR = ulBcdTime;
    
    RTC_vStart();

    OS_INT_Enable();
    
    /* Check for errors */
    if (rtc->RTC_VER != 0 || ((rtc->RTC_SR & RTC_SR_TDERR_CORRECT) != 0))
    {
        bStatus = false;
    }
    
    return bStatus;
}


/**
 * @brief   Start RTC.
 *
 * @param   None.
 *
 * @retval  None.
 */
static void RTC_vStart(void)
{
    RTC->RTC_CR &= ~(RTC_CR_UPDCAL | RTC_CR_UPDTIM);
}


/**
 * @brief   Stop RTC.
 *
 * @param   None.
 *
 * @retval  None.
 */
static void RTC_vStop(void)
{
    RTC->RTC_CR = RTC_CR_UPDCAL | RTC_CR_UPDTIM;
}


/**
 * @brief   Supervision and control for RTC.
 *
 * @param   pvArg   Unused.
 *
 * @retval  None.
 */
void RTC_vTask(void *pvArg)
{
    (void)pvArg;
    uint32_t   ret;
    uint32_t   ulEvent;
    Calendar   xCal;

    /* Set time for testing purposes */
    xCal.date.year    = 2020;
    xCal.date.month   = 9;
    xCal.date.day     = 13;
    xCal.time.hour    = 20;
    xCal.time.minutes = 25;
    xCal.time.seconds = 0;
    //RTC_bSetTime(&xCal);
    
    while (1)
    {
        ulEvent = OS_TASKEVENT_GetBlocked(0xFFFFFFFF);
        if (ulEvent == RTC_SEC_PERIOD)
        {
            RTC_bGetTime(&xCal);
        }
        if (ulEvent == RTC_SET_TIME)
        {
            //ret = OS_QUEUE_GetPtr(&tsQ, (void **)&xCal);
            //assert(ret == 0);

            /* Disable RTC second period IRQ to avoid race condition */
            //RTC->RTC_IDR = RTC_IDR_SECDIS;
            //RTC_bSetTime(&xCal);
            //RTC->RTC_IER = RTC_IER_SECEN;
        }
        if (ulEvent == RTC_RETURN_TIME)
        {
            //ret = OS_QUEUE_Put(&tsQ, &xCal, sizeof(xCal));
            //assert(ret == 0);
        }
    }
}


/**
 * @brief   Get RTC time in human readable format.
 *
 * @param   calendar  Pointer to input calendar.
 *
 * @retval  status    Time ok/not ok.
 */
static bool RTC_bGetTime(Calendar *pxCal)
{
    bool     bStatus                      = false;
    uint32_t ulBcdDate[STABLE_READ_COUNT] = { 0 };
    uint32_t ulBcdTime[STABLE_READ_COUNT] = { 0 };

    /* First read */
    ulBcdDate[0] = RTC->RTC_CALR;
    ulBcdTime[0] = RTC->RTC_TIMR;

    /* Do up to 2 more reads */
    for (uint32_t i = 1; i < STABLE_READ_COUNT; i++)
    {
        ulBcdDate[i] = RTC->RTC_CALR;
        ulBcdTime[i] = RTC->RTC_TIMR;

        /* Compare to previous */
        if ((ulBcdDate[i] == ulBcdDate[i - 1])
        &&  (ulBcdTime[i] == ulBcdTime[i - 1]))
        {
            /* Do sanity check */
            if ((ulBcdDate[i] != 0) || (ulBcdTime[i] != 0))
            {
                /* Convert BCD time to human readable format */
                pxCal->date.year     = ulBcd2Dec(ulBcdDate[i], CAL_YEAR);
                pxCal->date.month    = ulBcd2Dec(ulBcdDate[i], CAL_MONTH);
                pxCal->date.day      = ulBcd2Dec(ulBcdDate[i], CAL_DAY);
                pxCal->date.weekDay  = ulBcd2Dec(ulBcdDate[i], CAL_WDAY);
                pxCal->time.hour     = ulBcd2Dec(ulBcdTime[i], CAL_HOUR);
                pxCal->time.minutes  = ulBcd2Dec(ulBcdTime[i], CAL_MINUTE);
                pxCal->time.seconds  = ulBcd2Dec(ulBcdTime[i], CAL_SECOND);
                bStatus = true;
            }

            break;
        }
    }

    return bStatus;
}


/**
 * @brief     Current time returned is packed into a DWORD value.
 *
 *            The bit field is as follows:
 *
 *            bit31:25    Year from 1980 (0..127)
 *            bit24:21    Month (1..12)
 *            bit20:16    Day in month(1..31)
 *            bit15:11    Hour (0..23)
 *            bit10:5     Minute (0..59)
 *            bit4:0      Second (0..59)
 *
 * @param     None.
 *
 * @return    ulTime      Current time.
 */
//DWORD get_fattime(void)
uint32_t get_fattime(void)
{
/*
    Calendar    xCal;
    uint32_t    ulTime;
    uint32_t    ulStatus;

    ulStatus = RTC_bGetTime(&xCal);
    assert(ulStatus == true);

    ulTime = ((xCal.date.year - 1980) << 25)
           |  (xCal.date.month        << 21)
           |  (xCal.date.day          << 16)
           |  (xCal.time.hour         << 11)
           |  (xCal.time.minutes      <<  5)
           |  (xCal.time.seconds      <<  0);

    return ulTime;
*/
    return 0;
}
