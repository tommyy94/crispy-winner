#include "same70.h"
#include "isi_driver.h"
#include "io.h"
static void ISI_IO_Init(void);


/**
 * Initialize Image Sensor Interface.
 */
int32_t ISI_Init(void)
{

    ISI_IO_Init();

    return 0;
}


/**
 * Initialize Image Sensor Interface
 * IO pins.
 */
static void ISI_IO_Init(void)
{
    uint32_t mask;

    /* Initialize data pins */
    mask = PIO_PA5B_ISI_D4 | PIO_PA9B_ISI_D3;
    IO_SetPeripheralFunction(PIOA, mask, IO_PERIPH_B);
    mask = PIO_PD28D_ISI_D9 | PIO_PD27D_ISI_D8
         | PIO_PD12D_ISI_D6 | PIO_PD11D_ISI_D5
         | PIO_PD21D_ISI_D1 | PIO_PD22D_ISI_D0;
    IO_SetPeripheralFunction(PIOD, mask, IO_PERIPH_D);
    IO_SetPeripheralFunction(PIOB, PIO_PB3D_ISI_D2, IO_PERIPH_D);

    /* Sync and clock signals */
    mask = PIO_PD25D_ISI_VSYNC | PIO_PD24D_ISI_HSYNC;
    IO_SetPeripheralFunction(PIOD, mask, IO_PERIPH_D);

    /* Clock signal */
    IO_SetPeripheralFunction(PIOA, PIO_PA24D_ISI_PCK, IO_PERIPH_D);
}
