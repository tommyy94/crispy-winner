#include <same70.h>
#include "port.h"
#include "logWriter.h"


void PIOD_Handler(void)
{
    Pio *piod = PIOD;

    /* P28 magic number could be replaced */
    if ((piod->PIO_ISR & PIO_ISR_P28) != 0)
    {
        PIOD_ISR();
    }
}


/* Might want to add non-driver code into the handler
 * so the handler has weak linking.
 */
void PIOD_ISR(void)
{

}


void PIO_EnableIRQ(Pio *pio, uint32_t mask)
{
    assert((pio == PIOA) || (pio == PIOB) || (pio == PIOC) || (pio == PIOD) || (pio == PIOE));
    pio->PIO_IER = mask;
}


void PIO_DisableIRQ(Pio *pio, uint32_t mask)
{
    assert((pio == PIOA) || (pio == PIOB) || (pio == PIOC) || (pio == PIOD) || (pio == PIOE));
    pio->PIO_IDR = mask;
}


void PIO_ConfigurePull(Pio *pio, uint32_t mask, Pio_PullType pull)
{
    assert((pio == PIOA) || (pio == PIOB) || (pio == PIOC) || (pio == PIOD) || (pio == PIOE));
    assert((pull == PIO_PULLUP) || pull == PIO_PULLDOWN);

    if (pull == PIO_PULLUP)
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


void PIO_ConfigureIRQ(Pio *pio, PIO_IrqType type, PIO_IrqLevel level, uint32_t mask)
{
    assert((pio == PIOA) || (pio == PIOB) || (pio == PIOC) || (pio == PIOD) || (pio == PIOE));
    assert((type == PIO_EDGE_IRQ) || (type == PIO_LEVEL_IRQ));
    assert((level == PIO_LEVEL_POSITIVE) || (level == PIO_LEVEL_NEGATIVE));

    if (type == PIO_EDGE_IRQ)
    {
        pio->PIO_ESR |=  mask;
        pio->PIO_LSR &= ~mask;
    }
    else /* if PIO_LEVEL_IRQ */
    {
        pio->PIO_LSR |=  mask;
        pio->PIO_ESR &= ~mask;
    }

    if (level == PIO_LEVEL_NEGATIVE)
    {
        pio->PIO_FELLSR |=  mask;
        pio->PIO_REHLSR &= ~mask;
    }
    else /* if PIO_LEVEL_POSITIVE */
    {
        pio->PIO_REHLSR |=  mask;
        pio->PIO_FELLSR &= ~mask;
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
void PIO_vSetPeripheralFunction(Pio *pxPio, const uint32_t ulMask, const PIO_PeriphFunc xFunc)
{
    assert((pxPio == PIOA) || (pxPio == PIOB) || (pxPio == PIOC) || (pxPio == PIOD)|| (pxPio == PIOE));
    assert((xFunc == PIO_PERIPH_A) || (xFunc == PIO_PERIPH_B) || (xFunc == PIO_PERIPH_C) || (xFunc == PIO_PERIPH_D));

    switch (xFunc)
    {
        case PIO_PERIPH_A:
            pxPio->PIO_ABCDSR[0] &= ~ulMask;
            pxPio->PIO_ABCDSR[1] &= ~ulMask;
            break;
        case PIO_PERIPH_B:
            pxPio->PIO_ABCDSR[0] |=  ulMask;
            pxPio->PIO_ABCDSR[1] &= ~ulMask;
            break;
        case PIO_PERIPH_C:
            pxPio->PIO_ABCDSR[0] &= ~ulMask;
            pxPio->PIO_ABCDSR[1] |=  ulMask;
            break;
        case PIO_PERIPH_D:
            pxPio->PIO_ABCDSR[0] |=  ulMask;
            pxPio->PIO_ABCDSR[1] |=  ulMask;
            break;
        default:
            break;
    }
    
    pxPio->PIO_PDR = ulMask;
}


void PIO_vSetIoFunction(Pio *pxPio, const uint32_t ulMask, const PIO_Dir xDir)
{
    assert((pxPio == PIOA) || (pxPio == PIOB) || (pxPio == PIOC) || (pxPio == PIOD)|| (pxPio == PIOE));

    pxPio->PIO_PER = ulMask;
    pxPio->PIO_OER = xDir;
}


void PIO_vSetOutput(Pio *pxPio, const uint32_t ulMask)
{
    assert((pxPio == PIOA) || (pxPio == PIOB) || (pxPio == PIOC) || (pxPio == PIOD)|| (pxPio == PIOE));
    pxPio->PIO_SODR = ulMask;
}


void PIO_vClearOutput(Pio *pxPio, const uint32_t ulMask)
{
    assert((pxPio == PIOA) || (pxPio == PIOB) || (pxPio == PIOC) || (pxPio == PIOD)|| (pxPio == PIOE));
    pxPio->PIO_CODR = ulMask;
}


