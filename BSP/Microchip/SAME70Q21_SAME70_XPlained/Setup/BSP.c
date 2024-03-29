#include <stdbool.h>
#include "same70.h"
#include "RTOS.h"
#include "BSP.h"

/* Driver includes */
#include "spi_driver.h"
#include "rtc_driver.h"
#include "twi.h"
#include "dma.h"
#include "twi.h"
#include "io.h"
#include "mpu.h"
#include "tc_driver.h"
#include "sdram.h"
#include "isi_driver.h"


void BSP_Init(void)
{
    MPU_Config();

    /* Master clock is CPU clock divided by 2 */
    if (SDRAM_Init(IS42S16100F, SystemCoreClock >> 1) != SDRAM_OK)
    {
        OS_TASK_Terminate(NULL);
    }

    /* Enable PORT and PIO clock gating */
    IO_Init();

    /* Initialize communications */
    DMA_Init();
    TWI0_Init();

    /* Initialize timer */
    TC0_Init();

    /* Start RTC last so it won't notify non-existent task */
    RTC_Init();
}
