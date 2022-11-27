#ifndef TWI_UTIL_H
#define TWI_UTIL_H
#include <same70.h>
#include "err.h"

#define TWI_ERR_MASK    (TWIHS_SR_ARBLST \
                         | TWIHS_SR_UNRE \
                         | TWIHS_SR_OVRE)



/**
 * @brief   Write Transmit Holding Register.
 *
 * @param   pTwi     TWI instance pointer.
 *
 * @param   byte    Write value.
 */
__STATIC_INLINE void TWI_WriteTHR(Twihs *pTwi, const uint8_t byte)
{
    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    pTwi->TWIHS_THR = TWIHS_THR_TXDATA(byte);
}


/**
 * @brief   Read Receive Holding Register.
 *
 * @param   pTwi     TWI instance pointer.
 *
 * @retval  RHR       Register value.
 */
__STATIC_INLINE uint8_t TWI_ReadRHR(Twihs *pTwi)
{
    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    return pTwi->TWIHS_RHR;
}


/**
 * @brief   Write control register.
 *
 * @param   pxAdap    Pointer to TWI adapter.
 *
 * @param   mask    Mask to write.
 *
 * @retval  None.
 */
__STATIC_INLINE void TWI_WriteCR(Twihs *pTwi, uint32_t mask)
{
    assert((pTwi == TWIHS0) || (pTwi == TWIHS1) || (pTwi == TWIHS2));
    pTwi->TWIHS_CR = mask;
}


#endif /* TWI_UTIL_H */

