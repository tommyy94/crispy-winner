#include <stdbool.h>
#include "same70.h"
#include "RTOS.h"
#include "BSP.h"

/* Driver includes */
#include "spi_driver.h"
#include "rtc_driver.h"
#include "pwm_driver.h"
#include "twi.h"
#include "dma.h"
#include "twi.h"
#include "io.h"
#include "mpu.h"
#include "tc_driver.h"
#include "sdramc_driver.h"


void BSP_Init(void)
{
    /* IS42S16100F-5BL mounted on board
     * => Obsolote, replaced by IS42S16100H
     * 200 MHz SDRAM
     * 3.3V
     * 11 Row bits
     * 8 Column bits  
     */
    SDRAMC_Memory_t sdram =
    {
        /* Parameters */
        .cfg.dataBusWidth                   = SDRAMC_DBUS_WIDTH_16,
        .cfg.columnBits                     = SDRAMC_CR_NC_COL8,
        .cfg.rowBits                        = SDRAMC_CR_NR_ROW11,
        .cfg.nb                             = SDRAMC_CR_NB_BANK2,
        /* Timing parameters */
        .cfg.cas                            = SDRAMC_CR_CAS_LATENCY3,
        .cfg.writeRecoveryDelay             = 2,
        .cfg.rowCycleDelay_RowRefreshCycle  = 10,
        .cfg.rowColumnDelay                 = 3,
        .cfg.activePrechargeDelay           = 7,
        .cfg.rowPrechargeDelay              = 3,
        .cfg.exitSelfRefreshActiveDelay     = 0,
        /* Block1 is at the bit 21, 1(M0)+8(Col)+11(Row)+1(BK1): */
        .cfg.bk1                            = 21
    };

    SDRAMC_Init(&sdram, SystemCoreClock);

    MPU_Config();

    /* Enable PORT and PIO clock gating */
    IO_Init();

    /* Initialize communications */
    DMA_Init();
    TWI0_Init();

    /* Initialize motor control */
    //PWM_Init();

    /* Initialize timer */
    TC0_Init();

    /* Start RTC last so it won't notify non-existent task */
    RTC_Init();
}
