#ifndef TWI_UTIL_H
#define TWI_UTIL_H
#include <same70.h>
#include "logWriter.h"

#define TWI_ERR_MASK    (TWIHS_SR_ARBLST \
                         | TWIHS_SR_UNRE \
                         | TWIHS_SR_OVRE)



/**
 * @brief   Write Transmit Holding Register.
 *
 * @param   pxTwi     TWI instance pointer.
 *
 * @param   ucByte    Write value.
 */
__STATIC_INLINE void TWI_vWriteTHR(Twihs *pxTwi, const uint8_t ucByte)
{
    assert((pxTwi == TWIHS0) || (pxTwi == TWIHS1) || (pxTwi == TWIHS2));
    pxTwi->TWIHS_THR = TWIHS_THR_TXDATA(ucByte);
}


/**
 * @brief   Read Receive Holding Register.
 *
 * @param   pxTwi     TWI instance pointer.
 *
 * @retval  RHR       Register value.
 */
__STATIC_INLINE uint8_t TWI_ucReadRHR(Twihs *pxTwi)
{
    assert((pxTwi == TWIHS0) || (pxTwi == TWIHS1) || (pxTwi == TWIHS2));
    return pxTwi->TWIHS_RHR;
}


/**
 * @brief   Write control register.
 *
 * @param   pxAdap    Pointer to TWI adapter.
 *
 * @param   ulMask    Mask to write.
 *
 * @retval  None.
 */
__STATIC_INLINE void TWI_vWriteCR(Twihs *pxTwi, uint32_t ulMask)
{
    assert((pxTwi == TWIHS0) || (pxTwi == TWIHS1) || (pxTwi == TWIHS2));
    pxTwi->TWIHS_CR = ulMask;
    __DMB();
}


#endif /* TWI_UTIL_H */
