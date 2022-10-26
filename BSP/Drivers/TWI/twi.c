/* Device includes */
#include <same70.h>
#include <stdbool.h>
#include <stdio.h>

/* RTOS includes */
#include "RTOS.h"

/* User includes */
#include "twi.h"
#include "io.h"
#include "system.h"
#include "logWriter.h"
#include "pmc_driver.h"


#define TWI0_PORT       (PIOA)
#define PIN_TWCK0       (1 << 4u)
#define PIN_TWD0        (1 << 3u)


static OS_TIMER_EX twiTmr;
uint32_t xferStatus = TWI_SUCCESS;


static void    TWI0_IO_Init(void);
static void    TWI_ReleaseSlave(Twihs *pTwi);
static void    TWI_SetMasterMode(Twihs *pTwi);
static void    TWI_SetSlaveMode(Twihs *pTwi);
static void    TWI_Write(Twihs *pTwi, const uint32_t sAddr, TWI_Msg *pMsg, uint32_t  *const pStatus);
static void    TWI_Read(Twihs *pTwi, const uint32_t sAddr, TWI_Msg *pMsg, uint32_t  *const pStatus);
static void    TWI_FlushTHR(Twihs *pTwi);
static void    TWI_AbortXfer(TWI_Adapter *pAdap);
static void    TWI_TimeoutCallback(uint32_t *const pStatus);


__STATIC_INLINE void TWI_WriteTHR(Twihs *pTwi, uint8_t *const pByte, uint32_t  *const pStatus);
__STATIC_INLINE void TWI_ReadRHR(Twihs *pTwi, uint8_t *const pByte, uint32_t  *const pStatus);
__STATIC_INLINE void TWI_WriteCR(Twihs *pTwi, uint32_t mask);


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
 *   xTwiAdap.pMsg[0].buf    = &buf1;
 *   xTwiAdap.pMsg[0].len     = 1;
 *   xTwiAdap.pMsg[0].flags   = TWI_WRITE;
 *   xTwiAdap.pMsg[1].buf    = &buf2;
 *   xTwiAdap.pMsg[1].len     = 1;
 *   xTwiAdap.pMsg[1].flags   = TWI_READ;
 *   TWI_vXfer(&xTwiAdap, 2);
 */
void TWI0_Init(void)
{
    TWI0_IO_Init();

    /**
     * Enable TWI0 clock gating
     * - TWI0 clock = Peripheral clock / 2
     *              = 150 MHz / 2 = 75 MHz
     */
    PMC_PeripheralClockEnable(ID_TWIHS0);

    TWIHS0->TWIHS_CWGR = TWIHS_CWGR_CKDIV(1) | TWIHS_CWGR_CHDIV(90) | TWIHS_CWGR_CLDIV(90);
    /* These seem to be missing */
#define TWIHS_CR_FIFODIS  (1u << 29)
#define TWIHS_CR_FIFOEN   (1u << 28)
#define TWIHS_CR_THRCLR   (1u << 24)
    TWI_WriteCR(TWIHS0, TWIHS_CR_FIFOEN | TWIHS_CR_THRCLR);

    TWIHS0->TWIHS_IER = TWIHS_IER_ARBLST | TWIHS_IER_UNRE | TWIHS_IER_OVRE;

    TWI_SetMasterMode(TWIHS0);

    NVIC_ClearPendingIRQ(TWIHS0_IRQn);
    NVIC_SetPriority(TWIHS0_IRQn, TWIHS0_IRQ_PRIO);
    NVIC_EnableIRQ(TWIHS0_IRQn);
}


/**
 * @brief   Transfer messages.
 *
 * @param   pAdap       Pointer to TWI adapter.
 *
 * @param   count       Transfer count.
 *
 * @param   timeoutMs   Timeout in milliseconds.
 *
 * @retval  status      Xfer status.
 */
uint32_t TWI_Xfer(TWI_Adapter      *pAdap,
                  const uint32_t    count,
                  const uint32_t    timeoutMs)
{
    uint32_t status = TWI_SUCCESS;

    /* Sanity check */
    assert((pAdap->pInst == TWIHS0)
        || (pAdap->pInst == TWIHS1)
        || (pAdap->pInst == TWIHS2));
 
    OS_TIMER_CreateEx(&twiTmr, (void *)TWI_TimeoutCallback, timeoutMs, &status);
    OS_TIMER_StartEx(&twiTmr);

    for (uint32_t i = 0; i < count; i++)
    {
        assert((pAdap->msgArr[i].flags & TWI_WRITE)
            || (pAdap->msgArr[i].flags & TWI_READ));

        if (pAdap->msgArr[i].flags & TWI_READ)
        {
            TWI_Read(pAdap->pInst,
                     pAdap->addr,
                     &(pAdap->msgArr[i]),
                     &status);
        }
        else
        {
            if (i < (count - 1))
            {
                pAdap->msgArr[i].flags |= TWI_SR;
            }
            TWI_Write(pAdap->pInst,
                      pAdap->addr,
                      &(pAdap->msgArr[i]),
                      &status);
        }

        if (status != TWI_SUCCESS)
        {
            TWI_AbortXfer(pAdap);
            Journal_vWriteError(I2C_ERROR);
            goto endXfer;
        }
    }

    while (((pAdap->pInst->TWIHS_SR & TWIHS_SR_TXCOMP) == 0)
        && (status == TWI_SUCCESS))
    {
        ; /* Wait until transmission complete */
    }

endXfer:
    //TWI_FlushTHR(pAdap->pInst);
    OS_TIMER_DeleteEx(&twiTmr);

    return status;
}


/*
 * @brief   TWI transfer timeout callback.
 *
 * @param   pStatus     Pointer to store timeout information.
 *
 * @return  None.
 */
static void TWI_TimeoutCallback(uint32_t *const pStatus)
{
    *pStatus = TWI_TIMEOUT;
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
    /* Reset hardware */
    TWI_SetMasterMode(pAdap->pInst);
    TWI_ReleaseSlave(pAdap->pInst);
}


/**
 * @brief   Write Transmit Holding Register.
 *
 * @param   pTwi     TWI instance pointer.
 *
 * @param   pByte    Pointer to byte to write.
 *
 * @param   pStatus  pStatus
 *
 * @retval  In pStatus pointer.
 */
__STATIC_INLINE void TWI_WriteTHR(Twihs             *pTwi,
                                  uint8_t  *const    pByte,
                                  uint32_t *const    pStatus)
{
    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    pTwi->TWIHS_THR = TWIHS_THR_TXDATA(*pByte);
    while (((pTwi->TWIHS_SR & TWIHS_SR_TXRDY) == 0)
        && (*pStatus == TWI_SUCCESS))
    {
        ; /* Wait until buffer empty */    
    }
}


/**
 * @brief   Read Receive Holding Register.
 *
 * @param   pTwi     TWI instance pointer.
 *
 * @param   pByte    Pointer where byte is read.
 *
 * @param   pStatus  pStatus
 *
 * @retval  In pStatus pointer.
 */
__STATIC_INLINE void TWI_ReadRHR(Twihs             *pTwi,
                                 uint8_t  *const    pByte,
                                 uint32_t *const    pStatus)
{
    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    while (((pTwi->TWIHS_SR & TWIHS_SR_RXRDY) == 0)
        && (*pStatus == TWI_SUCCESS))
    {
        ; /* Wait until buffer full */    
    }
    *pByte = pTwi->TWIHS_RHR & TWIHS_RHR_RXDATA_Msk;
}


/**
 * @brief   Write control register.
 *
 * @param   pAdap   Pointer to TWI adapter.
 *
 * @param   mask    Mask to write.
 *
 * @retval  None.
 */
__STATIC_INLINE void TWI_WriteCR(Twihs *pTwi, uint32_t mask)
{
    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    pTwi->TWIHS_CR = mask;
    __DMB();
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
 * @param   sAddr    Slave address to write.
 *
 * @param   pMsg     Message buffer.
 *
 * @param   pStatus  pStatus
 *
 * @retval  In pStatus pointer.
 */
static void TWI_Write(
    Twihs             *pTwi,
    const   uint32_t   sAddr,
    TWI_Msg           *pMsg,
    uint32_t *const    pStatus)
{

    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));

    /* START bit sent automatically when writing */
    pTwi->TWIHS_MMR &= ~TWIHS_MMR_MREAD;
    __DMB();
    pTwi->TWIHS_MMR |= TWIHS_MMR_DADR(sAddr);
    __DMB();

    for (uint32_t i = 0; i < pMsg->len; i++)
    {
        TWI_WriteTHR(pTwi, &pMsg->buf[i], pStatus);
        if (*pStatus == TWI_TIMEOUT)
        {
            break;
        }
    }

    /* Don't send stop if repeated start set */
    if ((pMsg->flags & TWI_SR) == 0)
    {
        TWI_WriteCR(pTwi, TWIHS_CR_STOP);
    }
}


/**
 * @brief   Read TWI.
 *
 * @param   pTwi     TWI instance pointer.
 *
 * @param   sAddr    Slave address to read.
 *
 * @param   pMsg     Receive buffer.
 *
 * @param   pStatus  pStatus
 *
 * @retval  In pStatus pointer.
 */
static void TWI_Read(
    Twihs            *pTwi,
    const uint32_t    sAddr,
    TWI_Msg          *pMsg,
    uint32_t *const   pStatus)
{

    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    
    pTwi->TWIHS_MMR |= TWIHS_MMR_DADR(sAddr) | TWIHS_MMR_MREAD;
    __DMB();

    if (pMsg->len == 1)
    {
        /* Send START and STOP on single byte reads.
         * Must send STOP before last byte transfer
         * to avoid trigger another transfer.
         */
        TWI_WriteCR(pTwi, TWIHS_CR_START | TWIHS_CR_STOP);
        TWI_ReadRHR(pTwi, &pMsg->buf[0], pStatus);
    }
    else
    {
        TWI_WriteCR(pTwi, TWIHS_CR_START);
        
        for (uint32_t i = 0; i < pMsg->len; i++)
        {
            if (i == (pMsg->len - 1))
            {
                /*
                 * Must send STOP before last byte transfer
                 * to avoid trigger another transfer.
                 */
                TWI_WriteCR(pTwi, TWIHS_CR_STOP);
            }
            TWI_ReadRHR(pTwi, &pMsg->buf[i], pStatus);
            if (*pStatus != TWI_TIMEOUT)
            {
                break;        
            }
        }
    }
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
