#include <stdlib.h>
#include <same70.h>
#include "RTOS.h"
#include "io.h"
#include "err.h"
#include "system.h"
#include "pmc_driver.h"


/*
 * @brief   Enable clock gating.
 *
 * @param   pio     PIO instance pointer.
 *
 * @param   mask    Pin mask to configure.
 *
 * @param   pull    IO_PULLUP/IO_PULLDOWN.
 *
 * @return  None.
 */
void IO_Init(void)
{
   PMC_PeripheralClockEnable(ID_PIOA);
   PMC_PeripheralClockEnable(ID_PIOB);
   PMC_PeripheralClockEnable(ID_PIOD);
}


/*
 * @brief   Configure internal pullup/pulldown resistors.
 *
 * @param   pio     PIO instance pointer.
 *
 * @param   mask    Pin mask to configure.
 *
 * @param   pull    IO_PULLUP/IO_PULLDOWN.
 *
 * @return  None.
 */
void IO_ConfigurePull(Pio          *pio,
                      uint32_t      mask,
                      IO_PullType   pull)
{
    assert(IS_PIO(pio));
    assert((pull == IO_PULLUP) || pull == IO_PULLDOWN);

    if (pull == IO_PULLUP)
    {
        pio->PIO_MDER   = mask;
        pio->PIO_PPDDR  = mask;
        pio->PIO_PUER   = mask;
    }
    else /* if PUO_PULLDOWN */
    {
        pio->PIO_MDER   = mask;
        pio->PIO_PPDER  = mask;
        pio->PIO_PUDR   = mask;
    }
}


/**
 * @brief   Set peripheral function. See truth table below.
 *
 * @param   pio   Pointer to target peripheral.
 *
 * @param   mask  IO mask.
 *
 * @param   func  Peripheral function A, B, C, or D.
 *
 * @return  None.
 *
 * @note
 *
 *  ______________________________________
 * |            |           |             |
 * |  ABCDSR1   |  ABCDSR2  | Peripheral  |
 * |------------+-----------+-------------|
 * |     0      |     0     |     A       |
 * |     1      |     0     |     B       |
 * |     0      |     1     |     C       |
 * |     1      |     1     |     D       |
 * |____________|___________|_____________|
 */
void IO_SetPeripheralFunction(Pio                *pio,
                              const uint32_t      mask,
                              const IO_PeriphFunc func)
{
    assert(IS_PIO(pio));
    assert((func == IO_PERIPH_A)
        || (func == IO_PERIPH_B)
        || (func == IO_PERIPH_C)
        || (func == IO_PERIPH_D));

    switch (func)
    {
        case IO_PERIPH_A:
            pio->PIO_ABCDSR[0] &= ~mask;
            pio->PIO_ABCDSR[1] &= ~mask;
            break;
        case IO_PERIPH_B:
            pio->PIO_ABCDSR[0] |=  mask;
            pio->PIO_ABCDSR[1] &= ~mask;
            break;
        case IO_PERIPH_C:
            pio->PIO_ABCDSR[0] &= ~mask;
            pio->PIO_ABCDSR[1] |=  mask;
            break;
        case IO_PERIPH_D:
            pio->PIO_ABCDSR[0] |=  mask;
            pio->PIO_ABCDSR[1] |=  mask;
            break;
        default:
            break;
    }
    
    pio->PIO_PDR = mask;
}


/*
 * @brief Configure IO line as output.
 *
 * @param   pio         Pointer to PIO instance.
 *
 * @param   pinMask     Mask to configure.
 *
 * @param   driveMask   LOW = 0, HIGH = 1
 *
 * @return  None.
 */
void IO_ConfigureOutput(Pio   *const      pio,
                        const  uint32_t   pinMask,
                        const  uint32_t   driveMask)
{
    assert(IS_PIO(pio));

    pio->PIO_PER     = pinMask;
    pio->PIO_OER     = pinMask;
    pio->PIO_DRIVER |= driveMask;
}


/*
 * @brief   Configure all selected PORTx pins.
 *
 * @param   group     PORT group.
 *
 * @param   pinMask   Pins to configure.
 *
 * @param   pullMask  Enable pullup/down for pins to configure.
 *
 * @retval  None.
 */
void IO_ConfigureInput(Pio   *const      pio,      /* Port pointer     */
                       const uint32_t    pinMask,  /* Pin mask         */
                       const uint32_t    pullMask, /* Pull mask        */
                       const uint32_t    pullDir)  /* Pull-down/up     */
{
    pio->PIO_ODR     = pinMask;
    pio->PIO_MDER    = pinMask & pullMask;
    
    if (pullDir == IO_PULLUP)
    {
        pio->PIO_PPDDR  = pullMask;
        pio->PIO_PUER   = pullMask;
    }
    else /* if IO_PULLDOWN */
    {
        pio->PIO_PPDER  = pullMask;
        pio->PIO_PUDR   = pullMask;
    }
}


/*
 * @brief   Atomic output set.
 *
 * @param   group     PORT group.
 *
 * @param   pinMask   Pins to set HIGH.
 *
 * @retval  None.
 */
void IO_SetOutput(Pio *pio, const uint32_t mask)
{
    assert(IS_PIO(pio));
    pio->PIO_SODR = mask;
}


/*
 * @brief   Atomic output clear.
 *
 * @param   group     PORT group.
 *
 * @param   pinMask   Pins to set LOW.
 *
 * @retval  None.
 */
void IO_ClearOutput(Pio *pio, const uint32_t mask)
{
    assert(IS_PIO(pio));
    pio->PIO_CODR = mask;
}
