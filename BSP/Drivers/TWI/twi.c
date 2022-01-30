/* Device includes */
#include <same70.h>
#include <stdbool.h>
#include <stdio.h>

/* RTOS includes */
#include "RTOS.h"

/* User includes */
#include "twi.h"
#include "twi_util.h"
#include "port.h"
#include "system.h"
#include "logWriter.h"


#define TWI0_PORT       (PIOA)
#define PIN_TWCK0       (1 << 4u)
#define PIN_TWD0        (1 << 3u)


extern OS_MAILBOX         twiMbox;
extern OS_SEMAPHORE       twiSema;


static void    TWI0_IO_vInit(void);
static void    TWI_vReleaseSlave(Twihs *pxTwi);
static void    TWI_vSetMasterMode(Twihs *pxTwi);
static void    TWI_vSetSlaveMode(Twihs *pxTwi);
static bool    TWI_bWrite(Twihs *pxTwi, const uint32_t ulTarget, TWI_Msg *pxMsg);
static bool    TWI_bRead(Twihs *pxTwi, const uint32_t ulTarget, TWI_Msg *pxMsg);
static void    TWI_vFlushTHR(Twihs *pxTwi);
static void    TWI_AbortXfer(TWI_Adapter *pxAdap);


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
 *   xTwiAdap.pxInst             = TWIHS0;
 *   xTwiAdap.ulAddr             = ADDR_MPU6050;
 *   xTwiAdap.pxMsg[0].pucBuf    = &buf1;
 *   xTwiAdap.pxMsg[0].ulLen     = 1;
 *   xTwiAdap.pxMsg[0].ulFlags   = TWI_WRITE;
 *   xTwiAdap.pxMsg[1].pucBuf    = &buf2;
 *   xTwiAdap.pxMsg[1].ulLen     = 1;
 *   xTwiAdap.pxMsg[1].ulFlags   = TWI_READ;
 *   TWI_vXfer(&xTwiAdap, 2);
 */
void TWI0_vInit(void)
{
    TWI0_IO_vInit();

    /**
     * Enable TWI0 clock gating
     * - TWI0 clock = Peripheral clock / 2
     *              = 150 MHz / 2 = 75 MHz
     */
    //PMC->PMC_PCR |= PMC_PCR_CMD | PMC_PCR_PID(TWIHS0_CLOCK_ID) | PMC_PCR_EN;
    /* Maybe works ??? */
    PMC->PMC_PCR |= PMC_PCR_CMD | PMC_PCR_PID(ID_TWIHS0) | PMC_PCR_EN;

    //TWIHS0->TWIHS_CWGR = TWIHS_CWGR_CKDIV(1) | TWIHS_CWGR_CHDIV(43) | TWIHS_CWGR_CLDIV(43);
    TWIHS0->TWIHS_CWGR = TWIHS_CWGR_CKDIV(1) | TWIHS_CWGR_CHDIV(153) | TWIHS_CWGR_CLDIV(153);
    /* These seem to be missing */
    #define TWIHS_CR_FIFODIS  (1u << 29)
    #define TWIHS_CR_FIFOEN   (1u << 28)
    #define TWIHS_CR_THRCLR   (1u << 24)
    TWI_vWriteCR(TWIHS0, TWIHS_CR_FIFOEN | TWIHS_CR_THRCLR);

    TWIHS0->TWIHS_IER = TWIHS_IER_ARBLST | TWIHS_IER_UNRE | TWIHS_IER_OVRE;

    TWI_vSetMasterMode(TWIHS0);

    NVIC_ClearPendingIRQ(TWIHS0_IRQn);
    NVIC_SetPriority(TWIHS0_IRQn, TWIHS0_IRQ_PRIO);
    NVIC_EnableIRQ(TWIHS0_IRQn);
}


/**
 * @brief   Transfer messages.
 *
 * @param   pxAdap    Pointer to TWI adapter.
 *
 * @param   ulCount   Transfer count.
 *
 * @retval  ret       Xfer status.
 */
bool TWI_Xfer(TWI_Adapter *pxAdap, const uint32_t ulCount)
{
    bool ret;

    /* Sanity check */
    assert((pxAdap->pxInst == TWIHS0)
              || (pxAdap->pxInst == TWIHS1)
              || (pxAdap->pxInst == TWIHS2));

    for (uint32_t ulK = 0; ulK < ulCount; ulK++)
    {
        assert((pxAdap->pxMsg[ulK].ulFlags == TWI_WRITE)
                  || (pxAdap->pxMsg[ulK].ulFlags == TWI_READ));

        if (pxAdap->pxMsg[ulK].ulFlags == TWI_READ)
        {
            ret = TWI_bRead(pxAdap->pxInst,
                            pxAdap->ulAddr,
                            &(pxAdap->pxMsg[ulK]));
        }
        else
        {
            if (ulK < (ulCount - 1))
            {
                pxAdap->pxMsg[ulK].ulFlags |= TWI_SR;
            }
            ret = TWI_bWrite(pxAdap->pxInst,
                             pxAdap->ulAddr,
                             &(pxAdap->pxMsg[ulK]));
        }

        if (ret == false)
        {
            TWI_AbortXfer(pxAdap);
            //Journal_vWriteError(I2C_ERROR);
        }
    }

    while ((pxAdap->pxInst->TWIHS_SR & TWIHS_SR_TXCOMP) == 0)
    {
        ; /* Wait until transmission complete */
    }

    TWI_vFlushTHR(pxAdap->pxInst);

    return ret;
}


/*
 * @brief   Reset TWI hardware and RTOS
 *          signaling.
 *
 * @param   pxAdap    Pointer to TWI adapter.
 *
 * @return  None.
 */
static void TWI_AbortXfer(TWI_Adapter *pxAdap)
{
    /* Reset hardware */
    TWIHS0->TWIHS_IDR = TWIHS_IDR_TXRDY | TWIHS_IDR_RXRDY;
    TWI_vSetMasterMode(pxAdap->pxInst);
    TWI_vReleaseSlave(pxAdap->pxInst);

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
static void TWI0_IO_vInit(void)
{
    PIO_ConfigurePull(TWI0_PORT, PIN_TWCK0 | PIN_TWD0, PIO_PULLUP);
    PIO_vSetPeripheralFunction(TWI0_PORT, PIN_TWCK0 | PIN_TWD0, PIO_PERIPH_A);
}


/**
 * @brief   Write TWI.
 *
 * @param   pxTwi     TWI instance pointer.
 *
 * @param   ulTarget  Device address to write.
 *
 * @param   pxMsg     Message buffer.
 *
 * @retval  xRet      Write success/failure.
 */
static bool TWI_bWrite(
    Twihs             *pxTwi,
    const   uint32_t   ulTarget,
    TWI_Msg           *pxMsg)
{
    uint32_t ret;

    assert((pxTwi == TWIHS0) || (pxTwi == TWIHS1) || (pxTwi == TWIHS2));

    ret = OS_MAILBOX_Put(&twiMbox, &pxMsg);
    assert(ret == 0);

    /* START bit sent automatically when writing */
    pxTwi->TWIHS_MMR &= ~TWIHS_MMR_MREAD;
    __DMB();
    pxTwi->TWIHS_MMR |= TWIHS_MMR_DADR(ulTarget);
    __DMB();

    /* Enabling IRQ starts xfer and begin waiting until xfer done */
    pxTwi->TWIHS_IER = TWIHS_IER_TXRDY;
    ret = OS_SEMAPHORE_TakeTimed(&twiSema, 100);
    assert(ret != 0);

    assert((pxTwi->TWIHS_SR & TWI_ERR_MASK) == 0);

    return (bool)ret;
}


/**
 * @brief   Read TWI.
 *
 * @param   pxTwi     TWI instance pointer.
 *
 * @param   ulTarget  Device address to read.
 *
 * @param   pxMsg     Receive buffer.
 *
 * @retval  xRet      Read success/failure.
 */
static bool TWI_bRead(
    Twihs           *pxTwi,
    const uint32_t   ulTarget,
    TWI_Msg         *pxMsg)
{
    uint32_t    ret;
    uint32_t    ulMask       = TWIHS_CR_START;

    assert((pxTwi == TWIHS0) || (pxTwi == TWIHS1) || (pxTwi == TWIHS2));

    ret = OS_MAILBOX_Put(&twiMbox, &pxMsg);
    assert(ret == 0);

    pxTwi->TWIHS_MMR |= TWIHS_MMR_DADR(ulTarget) | TWIHS_MMR_MREAD;
    __DMB();

    /* START & STOP on single byte read */
    if (pxMsg->ulLen == 1)
    {
        ulMask |= TWIHS_CR_STOP;
    }
    TWI_vWriteCR(pxTwi, ulMask);

    /* Enabling IRQ starts xfer and begin waiting until xfer done */
    pxTwi->TWIHS_IER = TWIHS_IER_RXRDY;
    ret = OS_SEMAPHORE_TakeTimed(&twiSema, 100);
    assert(ret != 0);

    assert((pxTwi->TWIHS_SR & TWI_ERR_MASK) == 0);

    return (bool)ret;
}


/**
 * @brief   Manually generate 9 clock pulses to release the line.
 *
 * @param   None.
 *
 * @return  None.
 */
static void TWI_vReleaseSlave(Twihs *pxTwi)
{
    assert((pxTwi == TWIHS0) || (pxTwi == TWIHS1) || (pxTwi == TWIHS2));
    TWI_vWriteCR(pxTwi, TWIHS_CR_CLEAR | TWIHS_CR_THRCLR);
}

/**
 * @brief   Set TWI master mode.
 *
 * @param   pxTwi   Pointer to TWI instance.
 *
 * @retval  None.
 */
static void TWI_vSetMasterMode(Twihs *pxTwi)
{
    assert((pxTwi == TWIHS0) || (pxTwi == TWIHS1) || (pxTwi == TWIHS2));
    TWI_vWriteCR(TWIHS0, TWIHS_CR_MSEN | TWIHS_CR_SVDIS);
}


/**
 * @brief   Set TWI slave mode.
 *
 * @param   pxTwi   Pointer to TWI instance.
 *
 * @retval  None.
 */
static void TWI_vSetSlaveMode(Twihs *pxTwi)
{
    assert((pxTwi == TWIHS0) || (pxTwi == TWIHS1) || (pxTwi == TWIHS2));
    TWI_vWriteCR(TWIHS0, TWIHS_CR_MSDIS | TWIHS_CR_SVEN);
}


/**
 * @brief   Flush transmit holding register.
 *
 * @param   pxTwi   Pointer to TWIHS instance.
 *
 * @retval  None.
 */
static void TWI_vFlushTHR(Twihs *pxTwi)
{
    TWI_vWriteCR(pxTwi, TWIHS_CR_THRCLR);
}
