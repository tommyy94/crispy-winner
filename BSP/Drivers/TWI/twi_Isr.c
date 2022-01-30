#include <same70.h>
#include "twi.h"
#include "twi_util.h"
#include "RTOS.h"
#include "logWriter.h"


#define TWI_ERR_MASK    (TWIHS_SR_ARBLST \
                         | TWIHS_SR_UNRE \
                         | TWIHS_SR_OVRE)


extern OS_MAILBOX         twiMbox;
extern OS_SEMAPHORE       twiSema;


extern void   TWIHS0_IRQHandler(void);
static void   TWIHS0_Handler_vEndXfer(uint32_t *pulCnt);


/**
 * @brief   TWIHS0 IRQ handler.
 *
 * @param   None.
 *
 * @retval  None.
 */
void TWIHS0_IRQHandler(void)
{
    uint32_t        ret;
    uint32_t        ulStatus;
    static uint32_t ulCnt      = 0;
    static TWI_Msg *pxMsg      = NULL;

    OS_INT_Enter();

    ulStatus = TWIHS0->TWIHS_SR;
    assert((ulStatus & TWI_ERR_MASK) == 0);
    if ((ulStatus & TWI_ERR_MASK) == 0)
    {
        /* Get a new message if not in middle of xfer */
        if (pxMsg == NULL)
        {
            ret = OS_MAILBOX_Get(&twiMbox, &pxMsg);
            if (ret != 0)
            {
                __BKPT();
            }
            assert(ret == 0);
        }

        if (pxMsg != NULL)
        {
            if ((ulStatus & TWIHS_SR_TXRDY) && (TWIHS0->TWIHS_IMR & TWIHS_IMR_TXRDY))
            {
                /* Should be in master write mode here */
                assert((TWIHS0->TWIHS_MMR & TWIHS_MMR_MREAD) == 0);

                if ((ulStatus & TWIHS_SR_NACK) == 0)
                {
                    /* Perform some last byte special handling */
                    if (ulCnt >= (pxMsg->ulLen - 1))
                    {
                        TWIHS0->TWIHS_IDR = TWIHS_IDR_TXRDY;
                        __DMB();
                        TWI_vWriteTHR(TWIHS0, pxMsg->pucBuf[ulCnt]);

                        /* Skip STOP if Repeated START */
                        if ((pxMsg->ulFlags & TWI_SR) != TWI_SR)
                        {
                            TWI_vWriteCR(TWIHS0, TWIHS_CR_STOP);
                        }

                        TWIHS0_Handler_vEndXfer(&ulCnt);

                        /* Reset msg pointer for next xfer */
                        pxMsg = NULL;

                        /* Error checking already done,
                         * must reset copy of the status register
                         * to avoid derefencing NULL pointer.
                         */
                        ulStatus = 0;
                    }
                    else
                    {
                        TWI_vWriteTHR(TWIHS0, pxMsg->pucBuf[ulCnt++]);
                    }
                }
                else
                {
                    //Journal_vWriteError(I2C_ERROR);

                    TWI_vWriteCR(TWIHS0, TWIHS_CR_STOP);
                    TWIHS0_Handler_vEndXfer(&ulCnt);

                    /* Reset msg pointer for next xfer */
                    pxMsg = NULL;

                    /* Error checking already done,
                     * must reset copy of the status register
                     * to avoid derefencing NULL pointer.
                     */
                    ulStatus = 0;
                }
            }

            if ((ulStatus & TWIHS_SR_RXRDY) && (TWIHS0->TWIHS_IMR & TWIHS_IMR_RXRDY)) 
            {
                TWIHS0->TWIHS_IDR = TWIHS_IDR_RXRDY;
                __DMB();

                /* Should be in master read mode here */
                assert((TWIHS0->TWIHS_MMR & TWIHS_MMR_MREAD) != 0);

                if (ulCnt < (pxMsg->ulLen - 1))
                {
                    pxMsg->pucBuf[ulCnt++] = TWI_ucReadRHR(TWIHS0);
                    __DMB();
                    TWIHS0->TWIHS_IER = TWIHS_IER_RXRDY;
                }
                else /* Do last byte handling */
                {
                    /* Send STOP before reading the last byte to avoid
                     * initiating a unnecessary transaction.
                     */
                    TWI_vWriteCR(TWIHS0, TWIHS_CR_STOP);
                    pxMsg->pucBuf[ulCnt] = TWI_ucReadRHR(TWIHS0);

                    /* Reset msg pointer for next xfer */
                    pxMsg = NULL;

                    TWIHS0_Handler_vEndXfer(&ulCnt);
                }
            }
        }
    }

    OS_INT_Leave();
}


/**
 * @brief   End TWI transaction.
 *
 * @param   pulCnt        Counter pointer.
 *
 * @param   pxTaskWoken   Store force context switch status.
 *
 * @retval  None.
 */
static void TWIHS0_Handler_vEndXfer(uint32_t *pulCnt)
{
    /* No penalty for disabling both IRQs */
    TWIHS0->TWIHS_IDR = TWIHS_IDR_TXRDY | TWIHS_IDR_RXRDY;

    /* Signal subscriber */
    OS_SEMAPHORE_Give(&twiSema);

    *pulCnt = 0;
}
