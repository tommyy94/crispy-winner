/* Device includes */
#include <same70.h>
#include <stdbool.h>
#include <stdio.h>

/* RTOS includes */
#include "RTOS.h"

/* User includes */
#include "twi.h"
#include "twi_util.h"
#include "io.h"
#include "system.h"
#include "err.h"
#include "pmc_driver.h"


#define TWI0_PORT       (PIOA)
#define PIN_TWCK0       (1 << 4u)
#define PIN_TWD0        (1 << 3u)


extern OS_MAILBOX         twiMbox;
extern OS_SEMAPHORE       twiSema;


static void    TWI0_IO_Init(void);
static void    TWI_ReleaseSlave(Twihs *pTwi);
static void    TWI_SetMasterMode(Twihs *pTwi);
static void    TWI_SetSlaveMode(Twihs *pTwi);
static bool    TWI_Write(Twihs *pTwi, const uint32_t target, TWI_Msg *msgArr);
static bool    TWI_Read(Twihs *pTwi, const uint32_t target, TWI_Msg *msgArr);
static void    TWI_FlushTHR(Twihs *pTwi);
static void    TWI_AbortXfer(TWI_Adapter *pAdap);


/**
 * @brief   Initialize TWI0.
 *
 * @param   None.
 *
 * @retval  None.
 *
 * Example:
 *
 *   #define ADDR_MPU6050    (0x68)
 *   TWI_Adapter xTwiAdap;
 *   uint8_t     buf1            = { 0x75 };
 *   uint8_t     buf2            = { 0x00 };
 *
 *   xTwiAdap.pInst             = TWIHS0;
 *   xTwiAdap.addr             = ADDR_MPU6050;
 *   xTwiAdap.msgArr[0].pucBuf    = &buf1;
 *   xTwiAdap.msgArr[0].len     = 1;
 *   xTwiAdap.msgArr[0].flags   = TWI_WRITE;
 *   xTwiAdap.msgArr[1].pucBuf    = &buf2;
 *   xTwiAdap.msgArr[1].len     = 1;
 *   xTwiAdap.msgArr[1].flags   = TWI_READ;
 *   TWI_vXfer(&xTwiAdap, 2);
 */
static void TWI0_InitHW(void)
{
    TWIHS0->TWIHS_CWGR = TWIHS_CWGR_CKDIV(1) | TWIHS_CWGR_CHDIV(90) | TWIHS_CWGR_CLDIV(90);
    /* These seem to be missing */
    #define TWIHS_CR_FIFODIS  (1u << 29)
    #define TWIHS_CR_FIFOEN   (1u << 28)
    #define TWIHS_CR_THRCLR   (1u << 24)
    TWI_WriteCR(TWIHS0, TWIHS_CR_FIFOEN | TWIHS_CR_THRCLR);

    TWIHS0->TWIHS_IER = TWIHS_IER_ARBLST | TWIHS_IER_UNRE | TWIHS_IER_OVRE;

    TWI_SetMasterMode(TWIHS0);
}

void TWI0_Init(void)
{
    TWI0_IO_Init();

    /**
     * Enable TWI0 clock gating
     * - TWI0 clock = Peripheral clock / 2
     *              = 150 MHz / 2 = 75 MHz
     */
    PMC_PeripheralClockEnable(ID_TWIHS0);

    TWI0_InitHW();

    NVIC_ClearPendingIRQ(TWIHS0_IRQn);
    NVIC_SetPriority(TWIHS0_IRQn, TWIHS0_IRQ_PRIO);
    NVIC_EnableIRQ(TWIHS0_IRQn);
}


/**
 * @brief   Transfer messages.
 *
 * @param   pAdap    Pointer to TWI adapter.
 *
 * @param   count   Transfer count.
 *
 * @retval  ret       Xfer status.
 */
bool TWI_Xfer(TWI_Adapter *pAdap, const uint32_t count)
{
    bool ret = false;

    /* Sanity check */
    assert((pAdap->pInst == TWIHS0)
              || (pAdap->pInst == TWIHS1)
              || (pAdap->pInst == TWIHS2));

    for (uint32_t k = 0; k < count; k++)
    {
        assert((pAdap->msgArr[k].flags == TWI_WRITE)
                  || (pAdap->msgArr[k].flags == TWI_READ));

        if (pAdap->msgArr[k].flags == TWI_READ)
        {
            ret = TWI_Read(pAdap->pInst,
                            pAdap->addr,
                            &(pAdap->msgArr[k]));
        }
        else
        {
            if (k < (count - 1))
            {
                pAdap->msgArr[k].flags |= TWI_SR;
            }
            ret = TWI_Write(pAdap->pInst,
                             pAdap->addr,
                             &(pAdap->msgArr[k]));
        }

        if (ret == false)
        {
            TWI_AbortXfer(pAdap);
            err_report(I2C_ERROR);
            printf("TWI @ 0x%08x reset\r\n", pAdap->pInst);
            goto Cleanup;
        }
    }

    while ((pAdap->pInst->TWIHS_SR & TWIHS_SR_TXCOMP) == 0)
    {
        ; /* Wait until transmission complete */
    }

Cleanup:
    TWI_FlushTHR(pAdap->pInst);

    return ret;
}


/*
 * @brief   Reset TWI hardware and RTOS
 *          signaling.
 *
 * @param   pAdap    Pointer to TWI adapter.
 *
 * @return  None.
 */
static void TWI_AbortXfer(TWI_Adapter *pAdap)
{
    /* Reset bus */
    TWIHS0->TWIHS_IDR = TWIHS_IDR_TXRDY | TWIHS_IDR_RXRDY;
    TWI_SetMasterMode(pAdap->pInst);
    TWI_ReleaseSlave(pAdap->pInst);

    /* Reset hardware */
    TWI_WriteCR(TWIHS0, TWIHS_CR_SWRST);

    /* Clear signaling */
    OS_MAILBOX_Clear(&twiMbox);
    OS_SEMAPHORE_SetValue(&twiSema, 0);
}


/**
 * @brief   Configure TWI0 pins as peripheral function and
 *          enable internal pullups.
 *
 * @param   None.
 *
 * @retval  None.
 */
static void TWI0_IO_Init(void)
{
    IO_ConfigurePull(TWI0_PORT, PIN_TWCK0 | PIN_TWD0, IO_PULLUP);
    IO_SetPeripheralFunction(TWI0_PORT, PIN_TWCK0 | PIN_TWD0, IO_PERIPH_A);
}


/**
 * @brief   Write TWI.
 *
 * @param   pTwi     TWI instance pointer.
 *
 * @param   target  Device address to write.
 *
 * @param   msgArr     Message buffer.
 *
 * @retval  xRet      Write success/failure.
 */
static bool TWI_Write(
    Twihs             *pTwi,
    const   uint32_t   target,
    TWI_Msg           *msgArr)
{
    uint32_t ret;

    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));

    ret = OS_MAILBOX_Put(&twiMbox, &msgArr);
    assert(ret == 0);

    /* START bit sent automatically when writing */
    pTwi->TWIHS_MMR &= ~TWIHS_MMR_MREAD;
    pTwi->TWIHS_MMR |= TWIHS_MMR_DADR(target);

    /* Enabling IRQ starts xfer and begin waiting until xfer done */
    pTwi->TWIHS_IER = TWIHS_IER_TXRDY;
    ret = OS_SEMAPHORE_TakeTimed(&twiSema, 100);
    assert(ret != 0);

    assert((pTwi->TWIHS_SR & TWI_ERR_MASK) == 0);

    return (bool)ret;
}


/**
 * @brief   Read TWI.
 *
 * @param   pTwi     TWI instance pointer.
 *
 * @param   target  Device address to read.
 *
 * @param   msgArr     Receive buffer.
 *
 * @retval  xRet      Read success/failure.
 */
static bool TWI_Read(
    Twihs           *pTwi,
    const uint32_t   target,
    TWI_Msg         *msgArr)
{
    uint32_t    ret;
    uint32_t    mask       = TWIHS_CR_START;

    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));

    ret = OS_MAILBOX_Put(&twiMbox, &msgArr);
    assert(ret == 0);

    pTwi->TWIHS_MMR |= TWIHS_MMR_DADR(target) | TWIHS_MMR_MREAD;

    /* START & STOP on single byte read */
    if (msgArr->len == 1)
    {
        mask |= TWIHS_CR_STOP;
    }
    TWI_WriteCR(pTwi, mask);

    /* Enabling IRQ starts xfer and begin waiting until xfer done */
    pTwi->TWIHS_IER = TWIHS_IER_RXRDY;
    ret = OS_SEMAPHORE_TakeTimed(&twiSema, 100);
    assert(ret != 0);

    assert((pTwi->TWIHS_SR & TWI_ERR_MASK) == 0);

    return (bool)ret;
}


/**
 * @brief   Manually generate 9 clock pulses to release the line.
 *
 * @param   None.
 *
 * @return  None.
 */
static void TWI_ReleaseSlave(Twihs *pTwi)
{
    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    TWI_WriteCR(pTwi, TWIHS_CR_CLEAR | TWIHS_CR_THRCLR);
}

/**
 * @brief   Set TWI master mode.
 *
 * @param   pTwi   Pointer to TWI instance.
 *
 * @retval  None.
 */
static void TWI_SetMasterMode(Twihs *pTwi)
{
    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    TWI_WriteCR(TWIHS0, TWIHS_CR_MSEN | TWIHS_CR_SVDIS);
}


/**
 * @brief   Set TWI slave mode.
 *
 * @param   pTwi   Pointer to TWI instance.
 *
 * @retval  None.
 */
static void TWI_SetSlaveMode(Twihs *pTwi)
{
    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    TWI_WriteCR(TWIHS0, TWIHS_CR_MSDIS | TWIHS_CR_SVEN);
}


/**
 * @brief   Flush transmit holding register.
 *
 * @param   pTwi   Pointer to TWIHS instance.
 *
 * @retval  None.
 */
static void TWI_FlushTHR(Twihs *pTwi)
{
    TWI_WriteCR(pTwi, TWIHS_CR_THRCLR);
}
