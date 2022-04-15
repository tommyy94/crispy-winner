#include <same70.h>
#include "twi.h"


extern uint32_t xferStatus;
extern void   TWIHS0_IRQHandler(void);


/**
 * @brief   TWIHS0 IRQ handler.
 *
 * @param   None.
 *
 * @retval  None.
 */
void TWIHS0_IRQHandler(void)
{
    uint32_t status;

    status = TWIHS0->TWIHS_SR;
    if ((status & TWIHS_SR_ARBLST))
    {
        xferStatus |= TWI_ARB_LOST;
    }
    if ((status & TWIHS_SR_UNRE))
    {
        xferStatus |= TWI_UNDERRUN;
    }
    if ((status & TWIHS_SR_OVRE))
    {
        xferStatus |= TWI_OVERRUN;
    }
}
