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
#include "io.h"
#include "mpu.h"


void BSP_Init(void)
{
    MPU_Config();

    /* Enable PORT and PIO clock gating */
    IO_Init();

    /* Initialize communications */
    DMA_Init();
    TWI0_Init();

    /* Initialize motor control */
    PWM_Init();

    /* Start RTC last so it won't notify non-existent task */
    RTC_vInit();
}
