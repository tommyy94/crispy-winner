/** @file */

#ifndef SDRAM_H
#define SDRAM_H

#include "sdramc_driver.h"


enum
{
    SDRAM_OK = 0,       /**< SDRAM OK                   */
    SDRAM_SR_ERR,       /**< Self-refresh error         */
    SDRAM_MEMTEST_ERR,  /**< Memory test error          */
    SDRAM_INVALID_CHIP  /**< Invalid chip configuration */
};


typedef enum
{
    IS42S16100F = 0
} SDRAM_Device_t;


int32_t SDRAM_Init(SDRAM_Device_t dev, uint32_t clkFreq);

#endif /* SDRAM_H */
