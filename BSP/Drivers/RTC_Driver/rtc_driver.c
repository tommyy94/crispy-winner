/** @file */

/* Device vendor includes */
#include <same70.h>

/* System includes */
#include <stdbool.h>

/* RTOS includes */
#include "RTOS.h"

/* Application includes */
#include "rtc_driver.h"
#include "system.h"
#include "err.h"


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


static uint32_t dec2Bcd(uint32_t value, uint32_t mask);
static uint32_t bcd2Dec(uint32_t value, uint32_t mask);
static bool     RTC_SetTime(Calendar_t *pCal);
static bool     RTC_GetTime(Calendar_t *pCal);
static void     RTC_Start(void);
static void     RTC_Stop(void);


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
void RTC_Init(void)
{
    /* Disable RTC interrupts */
    RTC->RTC_IDR = RTC_IDR_TDERRDIS | RTC_IDR_CALDIS
                 | RTC_IDR_TIMDIS   | RTC_IDR_ALRDIS
                 | RTC_IDR_ACKDIS;

    RTC_Stop();

    RTC->RTC_SCCR = RTC_SCCR_TDERRCLR | RTC_SCCR_CALCLR
                  | RTC_SCCR_TIMCLR   | RTC_SCCR_SECCLR
                  | RTC_SCCR_ALRCLR   | RTC_SCCR_ACKCLR;

    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, RTC_IRQ_PRIO);
    NVIC_EnableIRQ(RTC_IRQn);

    /* Enable second periodic interrupt */
    RTC->RTC_IER = RTC_IER_SECEN;
    
    RTC_Start();
}


/**
 * @brief   Send second period signal to RTC_Task.
 *
 * @param   None.
 *
 * @retval  None.
 */
void RTC_IRQHandler(void)
{
    uint32_t status;

    OS_INT_Enter();

    /* Read status register */
    status = RTC->RTC_SR;

    /* Process IRQ */
    if ((status & RTC_SR_SEC_SECEVENT) != 0)
    {
        OS_TASKEVENT_Set(&rtcTCB, RTC_SEC_PERIOD);
    }

    /* Clear all status flags */
    RTC->RTC_SCCR = status;

    OS_INT_Leave();
}


/**
 * @brief   Convert decimal calendar value to BCD.
 *          Resulting BCD is shifted to correct
 *          RTC register bit field.
 *
 * @param   val   Value to convert.
 *
 * @retval  mask  Century/year/month/day/wday/hour/min/sec.
 */
static uint32_t dec2Bcd(uint32_t val, uint32_t mask)
{
    uint32_t bcd;

    switch(mask)
    {
        case CAL_CENTURY:
            bcd = ((val / BCD_FACTOR / BCD_FACTOR / BCD_FACTOR)  << (RTC_CALR_CENT_Pos + BCD_SHIFT)
                |   ((val / BCD_FACTOR / BCD_FACTOR) % BCD_FACTOR) << RTC_CALR_CENT_Pos);
            break;
        case CAL_YEAR:
            bcd = (((val / BCD_FACTOR) % BCD_FACTOR) << (RTC_CALR_YEAR_Pos + BCD_SHIFT))
                |   ((val % BCD_FACTOR) << RTC_CALR_YEAR_Pos);
            break;
        case CAL_MONTH:
            bcd = ((val / BCD_FACTOR) << (RTC_CALR_MONTH_Pos + BCD_SHIFT))
                |   ((val % BCD_FACTOR) <<  RTC_CALR_MONTH_Pos);
            break;
        case CAL_DAY:
            bcd = ((val / BCD_FACTOR) << (RTC_CALR_DATE_Pos + BCD_SHIFT))
                |   ((val % BCD_FACTOR) <<  RTC_CALR_DATE_Pos);
            break;
        case CAL_WDAY:
            bcd = ((val / BCD_FACTOR) << (RTC_CALR_DAY_Pos + BCD_SHIFT))
                |   ((val % BCD_FACTOR) <<  RTC_CALR_DAY_Pos);
            break;
        case CAL_HOUR:
            bcd = ((val / BCD_FACTOR) << (RTC_TIMR_HOUR_Pos + BCD_SHIFT))
                |   ((val % BCD_FACTOR) <<  RTC_TIMR_HOUR_Pos);
            break;
        case CAL_MINUTE:
            bcd = ((val / BCD_FACTOR) << (RTC_TIMR_MIN_Pos + BCD_SHIFT))
                |  ((val % BCD_FACTOR)  <<  RTC_TIMR_MIN_Pos);
            break;
        case CAL_SECOND:
            bcd = ((val / BCD_FACTOR) << (RTC_TIMR_SEC_Pos + BCD_SHIFT))
                |   ((val % BCD_FACTOR) <<  RTC_TIMR_SEC_Pos);
            break;
        default:
            bcd = 0;
            break;
    }

    return bcd;
}


/**
 * @brief   Convert BCD calendar value to decimal.
 *
 * @param   val   Value to convert.
 *
 * @retval  mask  Century/year/month/day/wday/hour/min/sec.
 */
static uint32_t bcd2Dec(uint32_t val, uint32_t mask)
{
    uint32_t dec;
    uint32_t tmp;

    switch(mask)
    {
        case CAL_CENTURY:
            tmp = (val & RTC_CALR_CENT_Msk) >> RTC_CALR_CENT_Pos;
            dec = (tmp >> BCD_SHIFT) * BCD_FACTOR + (tmp & BCD_MASK);
            break;
        case CAL_YEAR:
            tmp = (val & RTC_CALR_CENT_Msk) >> RTC_CALR_CENT_Pos;
            dec = (tmp >> BCD_SHIFT) * BCD_FACTOR + (tmp & BCD_MASK);
            tmp = (val & RTC_CALR_YEAR_Msk) >> RTC_CALR_YEAR_Pos;
            dec = (dec * BCD_FACTOR * BCD_FACTOR) + (tmp >> BCD_SHIFT) * BCD_FACTOR + (tmp & BCD_MASK);
            break;
        case CAL_MONTH:
            tmp = (val & RTC_CALR_MONTH_Msk) >> RTC_CALR_MONTH_Pos;
            dec = (tmp >> BCD_SHIFT) * BCD_FACTOR + (tmp & BCD_MASK);
            break;
        case CAL_DAY:
            tmp = (val & RTC_CALR_DATE_Msk) >> RTC_CALR_DATE_Pos;
            dec = (tmp >> BCD_SHIFT) * BCD_FACTOR + (tmp & BCD_MASK);
            break;
        case CAL_WDAY:
            tmp = (val & RTC_CALR_DAY_Msk) >> RTC_CALR_DAY_Pos;
            dec = (tmp >> BCD_SHIFT) * BCD_FACTOR + (tmp & BCD_MASK);
            break;
        case CAL_HOUR:
            tmp = (val & RTC_TIMR_HOUR_Msk) >> RTC_TIMR_HOUR_Pos;
            dec = (tmp >> BCD_SHIFT) * BCD_FACTOR + (tmp & BCD_MASK);
            break;
        case CAL_MINUTE:
            tmp = (val & RTC_TIMR_MIN_Msk) >> RTC_TIMR_MIN_Pos;
            dec = (tmp >> BCD_SHIFT) * BCD_FACTOR + (tmp & BCD_MASK);
            break;
        case CAL_SECOND:
            tmp = (val & RTC_TIMR_SEC_Msk) >> RTC_TIMR_SEC_Pos;
            dec = (tmp >> BCD_SHIFT) * BCD_FACTOR + (tmp & BCD_MASK);
            break;
        default:
            dec = 0;
            break;
    }

    return dec;
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
static bool RTC_SetTime(Calendar_t *pCal)
{
    uint32_t bcdTime;
    uint32_t bcdDate;
    bool     status   = true;
    
    /* Prepare time/calendar values */
    bcdDate = dec2Bcd(pCal->date.year,    CAL_YEAR)
            | dec2Bcd(pCal->date.month,   CAL_MONTH)
            | dec2Bcd(pCal->date.day,     CAL_DAY)
            | dec2Bcd(pCal->date.weekDay, CAL_WDAY);

    bcdTime = dec2Bcd(pCal->time.hour,    CAL_HOUR)
            | dec2Bcd(pCal->time.minutes, CAL_MINUTE)
            | dec2Bcd(pCal->time.seconds, CAL_SECOND);

    /* Wait for periodic event */
    while ((RTC->RTC_SR & RTC_SR_SEC_SECEVENT) == RTC_SR_SEC_NO_SECEVENT)
    {
        ;
    }

    OS_INT_Disable();
    
    RTC_Stop();
    
    /* Wait until command acknowledged */
    while ((RTC->RTC_SR & RTC_SR_ACKUPD) == RTC_SR_ACKUPD_FREERUN)
    {
        ;
    }
    RTC->RTC_SCCR = RTC_SCCR_ACKCLR;
    
    /* Update time/calendar values */
    RTC->RTC_CALR = bcdDate;
    RTC->RTC_TIMR = bcdTime;
    
    RTC_Start();

    OS_INT_Enable();
    
    /* Check for errors */
    if (RTC->RTC_VER != 0 || ((RTC->RTC_SR & RTC_SR_TDERR_CORRECT) != 0))
    {
        status = false;
    }
    
    return status;
}


/**
 * @brief   Start RTC.
 *
 * @param   None.
 *
 * @retval  None.
 */
static void RTC_Start(void)
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
static void RTC_Stop(void)
{
    RTC->RTC_CR = RTC_CR_UPDCAL | RTC_CR_UPDTIM;
}


/**
 * @brief   Supervision and control for RTC.
 *
 * @param   pArg   Unused.
 *
 * @retval  None.
 */
void RTC_Task(void *pArg)
{
    (void)pArg;
    //uint32_t   ret;
    uint32_t   evt;
    Calendar_t cal;

    /* Set time for testing purposes */
    cal.date.year    = 2020;
    cal.date.month   = 9;
    cal.date.day     = 13;
    cal.time.hour    = 20;
    cal.time.minutes = 25;
    cal.time.seconds = 0;
    RTC_SetTime(&cal);
    
    while (1)
    {
        evt = OS_TASKEVENT_GetBlocked(0xFFFFFFFF);
        if (evt == RTC_SEC_PERIOD)
        {
            RTC_GetTime(&cal);
        }
        if (evt == RTC_SET_TIME)
        {
            //ret = OS_QUEUE_GetPtr(&tsQ, (void **)&cal);
            //assert(ret == 0);

            /* Disable RTC second period IRQ to avoid race condition */
            //RTC->RTC_IDR = RTC_IDR_SECDIS;
            //RTC_SetTime(&cal);
            //RTC->RTC_IER = RTC_IER_SECEN;

            //OS_QUEUE_Purge(&tsQ);
        }
        if (evt == RTC_RETURN_TIME)
        {
            //ret = OS_QUEUE_Put(&tsQ, &cal, sizeof(cal));
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
static bool RTC_GetTime(Calendar_t *pCal)
{
    bool     status                      = false;
    uint32_t bcdDate[STABLE_READ_COUNT] = { 0 };
    uint32_t bcdTime[STABLE_READ_COUNT] = { 0 };

    /* First read */
    bcdDate[0] = RTC->RTC_CALR;
    bcdTime[0] = RTC->RTC_TIMR;

    /* Do up to 2 more reads */
    for (uint32_t i = 1; i < STABLE_READ_COUNT; i++)
    {
        bcdDate[i] = RTC->RTC_CALR;
        bcdTime[i] = RTC->RTC_TIMR;

        /* Compare to previous */
        if ((bcdDate[i] == bcdDate[i - 1])
        &&  (bcdTime[i] == bcdTime[i - 1]))
        {
            /* Do sanity check */
            if ((bcdDate[i] != 0) || (bcdTime[i] != 0))
            {
                /* Convert BCD time to human readable format */
                pCal->date.year     = bcd2Dec(bcdDate[i], CAL_YEAR);
                pCal->date.month    = bcd2Dec(bcdDate[i], CAL_MONTH);
                pCal->date.day      = bcd2Dec(bcdDate[i], CAL_DAY);
                pCal->date.weekDay  = bcd2Dec(bcdDate[i], CAL_WDAY);
                pCal->time.hour     = bcd2Dec(bcdTime[i], CAL_HOUR);
                pCal->time.minutes  = bcd2Dec(bcdTime[i], CAL_MINUTE);
                pCal->time.seconds  = bcd2Dec(bcdTime[i], CAL_SECOND);
                status = true;
            }

            break;
        }
    }

    return status;
}
