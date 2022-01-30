#include <stdbool.h>
#include "same70.h"
#include "RTOS.h"
#include "BSP.h"

/* Driver includes */
#include "spi_driver.h"
//#include "sdcard.h"
#include "rtc_driver.h"
#include "pwm_driver.h"
#include "twi.h"
#include "dma.h"
#include "twi.h"

/* ATWINC3400 */
//#include "common/include/nm_common.h"


void BSP_Init(void)
{
    /* Initialize communications */
    DMA_Init();
    SPI0_Init();
    TWI0_vInit();

    /* Initialize motor control */
    //PWM_Init();

    /* Start RTC last so it won't notify non-existent task */
    RTC_vInit();

    //nm_bsp_init();
}
