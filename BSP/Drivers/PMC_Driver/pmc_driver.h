#ifndef PMC_DRIVER_H
#define PMC_DRIVER_H

#include <stdint.h>
#include "io.h"


typedef enum
{
    PCK_SOURCE_SLOW_CLK = 0,
    PCK_SOURCE_MAIN_CLK,
    PCK_SOURCE_PLLA_CLK,
    PCK_SOURCE_UPLL_CLK,
    PCK_SOURCE_MCK_CLK,
    PCK_SOURCE_AUDIO_CLK,
    PCK_SOURCE_CNT
} PCK_Source_t;


void PMC_PeripheralClockEnable(const uint32_t pid);
void PMC_PeripheralClockDisable(const uint32_t pid);
void PMC_ProgrammableClockInit(
    uint32_t      const   n,
    uint8_t       const   pres,
    PCK_Source_t  const   src);
void PMC_ProgrammableClockEnable(const uint32_t n);
void PMC_ProgrammableClockDisable(const uint32_t n);

#endif /* PMC_DRIVER_H */
