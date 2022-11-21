#ifndef TC_DRIVER
#define TC_DRIVER

#include "same70.h"


typedef enum
{
    /* Each instance has 3 channels,
     * totaling 12 channels in the device.
     */
    TC_CHANNEL_0 = 0,
    TC_CHANNEL_1,
    TC_CHANNEL_2,
    TC_CHANNEL_COUNT
} TC_Channel_t;


void      TC_InstallIrqHandler(
    Tc            *pTc,
    TC_Channel_t   ch,
    void          *pfIsr);
void      TC0_Ch0_Init(void);
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
    uint32_t       us);

#endif /* TC_DRIVER */
