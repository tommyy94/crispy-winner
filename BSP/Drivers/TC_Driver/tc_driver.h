/** @file */

#ifndef TC_DRIVER
#define TC_DRIVER

#include "same70.h"
#include "fpa.h"

/**
 * Convert microseconds to TC0 channel 0 timer ticks.
 *
 * TC0 f_slck configured to 18.75MHz.
 *
 * \f{equation}{
 * period = 1\ tick \div f\_slck
 *        = 0.053us
 * \f}
 *
 * Avoid division, multiply instead:
 * \f{equation}{
 * mul = 1 \div period = 18.867924528
 * \f}
 *
 * In 32_9 format:
 * \f{equation}{
 * mul_s32_9 = mul \times (1 << 9)
 *           = 9660
 * \f}
 *
 * @param[in]   us
 */
#define TC0_CH0_US_TO_TICKS(us)   (FPA_MUL_UI32((us), 9660, 9))

/**
 * Convert microseconds to TC0 channel 0 timer ticks.
 *
 * TC0 f_slck configured to 18.75MHz.
 *
 * \f{equation}{
 * period = 1\ tick \div f\_slck
 *        = 0.053us
 * \f}
 *
 * In 32_9 format:
 * \f{equation}{
 * mul_s32_9 = period \times (1 << 9)
 *           = 27
 * \f}
 *
 * @param[in]   us
 */
#define TC0_CH0_TICKS_TO_US(us)   (FPA_MUL_UI32((us), 27, 9))


/**
 * Each instance has 3 channels,
 * totaling 12 channels in the device.
 */
typedef enum
{   /** TCx channel 0 */
    TC_CHANNEL_0 = 0,
    /** TCx channel 1 */
    TC_CHANNEL_1,
    /** TCx channel 2 */
    TC_CHANNEL_2,
    /** TCx channel count */
    TC_CHANNEL_COUNT
} TC_Channel_t;


void      TC_InstallIrqHandler(
    Tc            *pTc,
    TC_Channel_t   ch,
    void          *pfIsr);
void      TC0_Init(void);
uint32_t  TC_ReadCounter(
    Tc            *pTc,
    TC_Channel_t   ch);
void      TC_Start(
    Tc            *pTc,
    TC_Channel_t   ch);
void      TC_Stop(
    Tc            *pTc,
    TC_Channel_t   ch);
void TC_Delay(
    Tc            *pTc,
    TC_Channel_t   ch,
    uint32_t       ticks);

#endif /* TC_DRIVER */
