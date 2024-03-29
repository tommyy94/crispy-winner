/** @file */

#include "tc_driver.h"
#include "pmc_driver.h"
#include "system.h"
#include "same70.h"
#include "RTOS.h"
#include "err.h"
#include "fpa.h"


#define IS_TC(x)          (((x) == TC0) || \
                           ((x) == TC1) || \
                           ((x) == TC2) || \
                           ((x) == TC3))


typedef void (*TC_Callback)(void);
static TC_Callback *handlerTbl[TC_CHANNEL_COUNT];

extern void TC0_IRQHandler(void);


/**
 * @brief   Initialize TC0 channels 0 and 1 timers.
 *
 * @param   None.
 *
 * @retval  None. 
 */
void TC0_Init(void)
{
    PMC_PeripheralClockEnable(ID_TC0);
    PMC_PeripheralClockEnable(ID_TC1);

    /*
     * 150 MHz MCK
     * Select 32 prescaler
     *
     * f_slck = 150MHz / 8
     *                = 18.75MHz
     */
    TC0->TC_CHANNEL[TC_CHANNEL_0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK2;
    TC0->TC_CHANNEL[TC_CHANNEL_1].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK2;
}


/**
 * @brief   Start TC timer.
 *
 * @param   pTc   Pointer to TC instance.
 *
 * @param   ch    TC channel.
 *
 * @retval  None. 
 */
void TC_Start(
    Tc              *pTc,
    TC_Channel_t     ch)
{
    assert(IS_TC(pTc));
    assert(ch < TC_CHANNEL_COUNT);

    /* Enable clock and start the timer */
    pTc->TC_CHANNEL[ch].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
    
    while (!(pTc->TC_CHANNEL[ch].TC_SR & TC_SR_CLKSTA))
    {
        ; /* Wait until clock enabled */
    }
}


/**
 * @brief   Stop TC timer.
 *
 * @param   pTc   Pointer to TC instance.
 *
 * @param   ch    TC channel.
 *
 * @retval  None. 
 */
void TC_Stop(
    Tc              *pTc,
    TC_Channel_t     ch)
{
    assert(IS_TC(pTc));
    assert(ch < TC_CHANNEL_COUNT);

    /* Disable clock stopping the timer */
    pTc->TC_CHANNEL[ch].TC_CCR = TC_CCR_CLKDIS;

    while (pTc->TC_CHANNEL[ch].TC_SR & TC_SR_CLKSTA)
    {
        ; /* Wait until clock disabled */
    }
}


/**
 * @brief   Perform delay in microsecond accuracy.
 *
 * @param   pTc   Pointer to TC instance.
 *
 * @param   ch    TC channel.
 *
 * @param   ticks Timer ticks.
 *
 * @retval  None. 
 */
void TC_Delay(
    Tc              *pTc,
    TC_Channel_t     ch,
    uint32_t         ticks)
{

    TC_Start(pTc, ch);

    /* Wait until ticks passed */
    while ((pTc->TC_CHANNEL[ch].TC_CV & 0xFFFF) < ticks)
    {
        if (pTc->TC_CHANNEL[ch].TC_SR & TC_SR_COVFS)
        {
            err_report(TC_ERROR);
            break;
        }
    }

    TC_Stop(pTc, ch);
}


/**
 * @brief   TC0 channel 0 IRQ handler.
 *
 * @param   None.
 *
 * @retval  None. 
 */
void TC0_IRQHandler(void)
{
    uint32_t    status;
    uint32_t    mask;
    TC_Callback cb;

    OS_INT_Enter();

    mask = TC0->TC_CHANNEL[TC_CHANNEL_0].TC_IMR;

    status = TC0->TC_CHANNEL[TC_CHANNEL_0].TC_SR & mask;
    if (status & TC_SR_COVFS)
    {
        /* Counter overflow */
        cb = (TC_Callback)handlerTbl[TC_CHANNEL_0];
        if (cb != NULL)
        {
            cb();
        }
    }

    OS_INT_Leave();
}


/*
 * @brief Install TC IRQ handler.
 *
 * @param ch      TC channel.
 *
 * @param pfIsr   Pointer to handler to install.
 */
void TC_InstallIrqHandler(
    Tc              *pTc,
    TC_Channel_t     ch,
    void            *pfIsr)
{
    uint32_t chOffset;

    assert(IS_TC(pTc));
    assert(ch    < TC_CHANNEL_COUNT);
    assert(pfIsr != NULL);

    /* Each instance has 3 channels,
     * totaling 12 channels in the device.
     */
    if (pTc == TC0)
    {
        chOffset = 0;
    }
    else if (pTc == TC1)
    {
        chOffset = 3;
    }
    else if (pTc == TC2)
    {
        chOffset = 6;
    }
    else /* if (pTc == TC3) */
    {
        chOffset = 9;
    }

    handlerTbl[ch + chOffset] = pfIsr;
}


/**
 * @brief   Read TC timer.
 *
 * @param   pTc   Pointer to TC instance.
 *
 * @param   ch    TC channel.
 *
 * @retval  TC_CV TC counter value.
 */
uint32_t TC_ReadCounter(
    Tc            *pTc,
    TC_Channel_t   ch)
{
    uint32_t ticks;

    assert(IS_TC(pTc));
    assert(ch < TC_CHANNEL_COUNT);

    /* Register is 32-bit, but the counter is 16-bit */
    ticks = pTc->TC_CHANNEL[ch].TC_CV & 0xFFFF;
    return ticks;
}
