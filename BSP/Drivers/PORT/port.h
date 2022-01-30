#pragma once
#include <same70.h>


typedef enum
{
    PIO_PULLUP,
    PIO_PULLDOWN
} Pio_PullType;


typedef enum
{
    PIO_EDGE_IRQ,
    PIO_LEVEL_IRQ
} PIO_IrqType;


typedef enum
{
    PIO_LEVEL_NEGATIVE,
    PIO_LEVEL_POSITIVE
} PIO_IrqLevel;

typedef enum
{
    PIO_DIR_IN = 0,
    PIO_DIR_OUT
}
PIO_Dir;


typedef enum
{
    PIO_PERIPH_A = 0,
    PIO_PERIPH_B,
    PIO_PERIPH_C,
    PIO_PERIPH_D,
} PIO_PeriphFunc;


void PIO_DisableIRQ(Pio *pio, uint32_t pin);
void PIO_EnableIRQ(Pio *pio, uint32_t pin);
void PIO_ConfigurePull(Pio *pio, uint32_t mask, Pio_PullType pull);
void PIO_ConfigureIRQ(Pio *pio, PIO_IrqType type, PIO_IrqLevel level, uint32_t mask);
void PIOD_ISR(void) __attribute__((weak));
void PIO_vSetPeripheralFunction(Pio *pxPio, const uint32_t ulMask, const PIO_PeriphFunc xFunc);
void PIO_vSetIoFunction(Pio *pxPio, const uint32_t ulMask, const PIO_Dir xDir);
void PIO_vSetOutput(Pio *pxPio, const uint32_t ulMask);
void PIO_vClearOutput(Pio *pxPio, const uint32_t ulMask);
