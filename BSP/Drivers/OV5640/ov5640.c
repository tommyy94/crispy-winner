/** @file */

#include "same70.h"
#include "io.h"
#include "ov5640.h"
#include "ov5640_i2c_layer.h"
#include <stdbool.h>
#include "err.h"
#include "io.h"
#include "pmc_driver.h"


#define OV5640_PORT       (PIOB)
#define OV5640_RST_PIN    (1u << 0)
#define OV5640_PWDN_PIN   (1u << 1)
#define OV5640_XCLK_PIN   (1u << 6)


static void OV5640_ConfigureIO(void);
static void OV5640_AssertReset(void);
static void OV5640_ReleaseReset(void);
static void OV5640_EnterPowerDown(void);
static void OV5640_ExitPowerDown(void);


/**
 * Configure OV5640 I/O pins.
 */
static void OV5640_ConfigureIO(void)
{
    IO_ConfigureOutput(OV5640_PORT, OV5640_PWDN_PIN, OV5640_PWDN_PIN);
    IO_ConfigureOutput(OV5640_PORT, OV5640_RST_PIN, OV5640_RST_PIN);
    IO_SetPeripheralFunction(PIOA, OV5640_XCLK_PIN, IO_PERIPH_B);

    /* Assert reset to ensure everything is set up correctly.
     * Release reset when done.
     */
    OV5640_AssertReset();

    /* Disable low power mode */
    OV5640_ExitPowerDown();

    OV5640_ReleaseReset();
}


/**
 * Assert RESETB pin.
 * RESETB is active LOW with internal pull-up resistor.
 */
static void OV5640_AssertReset(void)
{
    IO_ClearOutput(OV5640_PORT, OV5640_RST_PIN);
}


/**
 * Assert RESETB pin.
 * RESETB is Active LOW with internal pull-up resistor.
 */
static void OV5640_ReleaseReset(void)
{
    IO_SetOutput(OV5640_PORT, OV5640_RST_PIN);
}


/**
 * Asserts PWDN pin and enters power down mode.
 * PWDN is active HIGH with internal pull-up resistor.
 */
static void OV5640_EnterPowerDown(void)
{
    IO_SetOutput(OV5640_PORT, OV5640_PWDN_PIN);
}


/**
 * Releases PWDN pin and exits power down mode.
 * PWDN is active HIGH with internal pull-up resistor.
 */
static void OV5640_ExitPowerDown(void)
{
    IO_ClearOutput(OV5640_PORT, OV5640_PWDN_PIN);
}


/**
 * Initialize OV5640. 
 * Configures the IO lines, I2C lines and clock input.
 */
void OV5640_Init(void)
{
    bool ret;

    /* Configure the host controller to feed clock input to OV5640
     * MCK (System Clock) = 150 MHz
     * 150MHz / 6 = 25MHz
     */
    PMC_ProgrammableClockInit(0, 5, PCK_SOURCE_MCK_CLK);
    PMC_ProgrammableClockEnable(0);

    OV5640_ConfigureIO();
    OV5640_ExitPowerDown();
    OV5640_ReleaseReset();

    ret = OV5640_VerifyChip();
    assert(ret == true);
}
