#include <same70.h>
#include "twi.h"
#include "twi_util.h"
#include "RTOS.h"
#include "err.h"


#define TWI_ERR_MASK    (TWIHS_SR_ARBLST \
                         | TWIHS_SR_UNRE \
                         | TWIHS_SR_OVRE)


extern OS_MAILBOX         twiMbox;
extern OS_SEMAPHORE       twiSema;


extern void   TWIHS0_IRQHandler(void);
static void   TWIHS0_Handler_EndXfer(uint32_t *pCnt);


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
    uint32_t        status;
    static uint32_t cnt      = 0;
    static TWI_Msg *pMsg      = NULL;

    OS_INT_Enter();

    status = TWIHS0->TWIHS_SR;
    assert((status & TWI_ERR_MASK) == 0);
    if ((status & TWI_ERR_MASK) == 0)
    {
        /* Get a new message if not in middle of xfer */
        if (pMsg == NULL)
        {
            ret = OS_MAILBOX_Get(&twiMbox, &pMsg);
            assert(ret == 0);
        }

        if (pMsg != NULL)
        {
            if ((status & TWIHS_SR_TXRDY) && (TWIHS0->TWIHS_IMR & TWIHS_IMR_TXRDY))
            {
                /* Should be in master write mode here */
                assert((TWIHS0->TWIHS_MMR & TWIHS_MMR_MREAD) == 0);

                if ((status & TWIHS_SR_NACK) == 0)
                {
                    /* Perform some last byte special handling */
                    if (cnt >= (pMsg->len - 1))
                    {
                        TWIHS0->TWIHS_IDR = TWIHS_IDR_TXRDY;
                        TWI_WriteTHR(TWIHS0, pMsg->pBuf[cnt]);

                        /* Skip STOP if Repeated START */
                        if ((pMsg->flags & TWI_SR) != TWI_SR)
                        {
                            TWI_WriteCR(TWIHS0, TWIHS_CR_STOP);
                        }

                        TWIHS0_Handler_EndXfer(&cnt);

                        /* Reset msg pointer for next xfer */
                        pMsg = NULL;

                        /* Error checking already done,
                         * must reset copy of the status register
                         * to avoid derefencing NULL pointer.
                         */
                        status = 0;
                    }
                    else
                    {
                        TWI_WriteTHR(TWIHS0, pMsg->pBuf[cnt++]);
                    }
                }
                else
                {
                    err_report(I2C_ERROR);

                    TWI_WriteCR(TWIHS0, TWIHS_CR_STOP);
                    TWIHS0_Handler_EndXfer(&cnt);

                    /* Reset msg pointer for next xfer */
                    pMsg = NULL;

                    /* Error checking already done,
                     * must reset copy of the status register
                     * to avoid derefencing NULL pointer.
                     */
                    status = 0;
                }
            }

            if ((status & TWIHS_SR_RXRDY) && (TWIHS0->TWIHS_IMR & TWIHS_IMR_RXRDY)) 
            {
                TWIHS0->TWIHS_IDR = TWIHS_IDR_RXRDY;

                /* Should be in master read mode here */
                assert((TWIHS0->TWIHS_MMR & TWIHS_MMR_MREAD) != 0);

                if (cnt < (pMsg->len - 1))
                {
                    pMsg->pBuf[cnt++] = TWI_ReadRHR(TWIHS0);
                    TWIHS0->TWIHS_IER = TWIHS_IER_RXRDY;
                }
                else /* Do last byte handling */
                {
                    /* Send STOP before reading the last byte to avoid
                     * initiating a unnecessary transaction.
                     */
                    TWI_WriteCR(TWIHS0, TWIHS_CR_STOP);
                    pMsg->pBuf[cnt] = TWI_ReadRHR(TWIHS0);

                    /* Reset msg pointer for next xfer */
                    pMsg = NULL;

                    TWIHS0_Handler_EndXfer(&cnt);
                }
            }
        }
    }

    OS_INT_Leave();
}


/**
 * @brief   End TWI transaction.
 *
 * @param   pCnt        Counter pointer.
 *
 * @retval  None.
 */
static void TWIHS0_Handler_EndXfer(uint32_t *pCnt)
{
    /* No penalty for disabling both IRQs */
    TWIHS0->TWIHS_IDR = TWIHS_IDR_TXRDY | TWIHS_IDR_RXRDY;

    /* Signal subscriber */
    OS_SEMAPHORE_Give(&twiSema);

    *pCnt = 0;
}
