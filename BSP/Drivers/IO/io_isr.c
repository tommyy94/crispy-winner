#include <stdint.h>
#include "io.h"
#include "same70.h"
#include "RTOS.h"
#include "system.h"
#include "logWriter.h"


#define PIO_INST_COUNT  (5u)


typedef void (*IO_Callback)(void);

static IO_Callback   *handlerTbl[IOn];
static IRQn_Type      IO_Pio2NVICn(Pio *pio);
static uint32_t       piodFirstIrqN           = 0;
static uint32_t       piodLastIrqN            = 0;

void PIOD_IRQHandler(void);

void PIOD_IRQHandler(void)
{
    uint32_t      status;
    IO_Callback   pioCb;

    OS_INT_Enter();

    status  = PIOD->PIO_ISR;

    for (uint32_t i = piodFirstIrqN; i <= piodLastIrqN; i++)
    {
        if ((status & (1u << i)) != 0)
        {
            pioCb = (IO_Callback)handlerTbl[i];
            if (pioCb != NULL)
            {
                pioCb();
            }
        }
    }

    OS_INT_Leave();
}


/*
 * @brief Install PIO IRQ handler.
 *
 * @param line    IO line.
 *
 * @param pfIsr   Pointer to handler to install.
 */
void IO_InstallIrqHandler(uint32_t const   line,
                          void            *pfIsr)
{
    assert(line  <= IOn);
    assert(pfIsr != NULL);

    handlerTbl[line] = pfIsr;

    if (line >= piodFirstIrqN)
    {
        piodFirstIrqN = line;
    }

    if (line >= piodLastIrqN)
    {
        piodLastIrqN = line;
    }
}


/*
 * @brief   Configure PIO IRQ line.
 *
 * @param   pio     Pointer to PIO instance.
 *
 * @param   sense   Edge/level type.
 *
 * @param   mask    Pins to configure.
 *
 * @return  None.
 */
IRQn_Type IO_ConfigureIRQ(Pio         *pio,
                          IO_Sense_t   sense,
                          uint32_t     mask)
{
    IRQn_Type irqn;

    assert(PORT_IS_PIO(pio));
    assert(sense < IO_SENSE_COUNT);

    switch (sense)
    {
        case IO_SENSE_RISE:
            pio->PIO_AIMER  = mask;
            pio->PIO_ESR    = mask;
            pio->PIO_REHLSR = mask;
            break;
        case IO_SENSE_FALL:
            pio->PIO_AIMER  = mask;
            pio->PIO_ESR    = mask;
            pio->PIO_FELLSR = mask;
            break;
        case IO_SENSE_HIGH:
            pio->PIO_AIMER  = mask;
            pio->PIO_LSR    = mask;
            pio->PIO_REHLSR = mask;
            break;
        case IO_SENSE_LOW:
            pio->PIO_AIMER  = mask;
            pio->PIO_LSR    = mask;
            pio->PIO_FELLSR = mask;
            break;
        case IO_SENSE_RISE_FALL:
            pio->PIO_AIMDR  = mask;
            pio->PIO_ESR    = mask;
            break;
        case IO_SENSE_COUNT:
            /* Should not get here */
        default:
            break;
    }

    pio->PIO_IER |= mask;
    
    irqn = IO_Pio2NVICn(pio);

    NVIC_ClearPendingIRQ(irqn);
    NVIC_SetPriority(irqn, IO_IRQ_PRIO);
    NVIC_EnableIRQ(irqn);

    return irqn;
}


/*
 * @brief   Convert PIO instance to NVIC IRQn.
 *
 * @param   pio   PIO instance pointer.
 *
 * @return  irqn  NVIC IRQn.
 */
static IRQn_Type IO_Pio2NVICn(Pio *pio)
{
    assert(PORT_IS_PIO(pio));
    
    IRQn_Type const irqnTbl[PIO_INST_COUNT] =
    {
        PIOA_IRQn, PIOB_IRQn, PIOC_IRQn, PIOD_IRQn, PIOE_IRQn
    };
    Pio *const pioTbl[PIO_INST_COUNT] =
    {
        PIOA, PIOB, PIOC, PIOD, PIOE
    };
    
    IRQn_Type irqn  = PERIPH_COUNT_IRQn;

    for (uint32_t i = 0; i< PIO_INST_COUNT; i++)
    {
        if (pio == pioTbl[i])
        {
            irqn = irqnTbl[i];
            break;
        }
    }

    return irqn;
}


/*
 * @brief   Enable IRQ.
 *
 * @param   pio   Pointer to PIO instance.
 *
 * @param   mask  IO mask to enable.
 *
 * @return  None.
 */
void IO_EnableIRQ(Pio *pio, uint32_t mask)
{
    assert(PORT_IS_PIO(pio));
    pio->PIO_IER = mask;
}


/*
 * @brief   Disable IRQ.
 *
 * @param   pio   Pointer to PIO instance.
 *
 * @param   mask  IO mask to disable.
 *
 * @return  None.
 */
void IO_DisableIRQ(Pio *pio, uint32_t mask)
{
    assert(PORT_IS_PIO(pio));
    pio->PIO_IDR = mask;
}


/*
 * @brief   Get IRQ source.
 *
 * @param   pio   Pointer to PIO instance.
 *
 * @param   mask  IO mask to disable.
 *
 * @return  None.
 */
IO_PinLevel_t IO_GetPinLevel(Pio *pio, uint32_t pin)
{
    assert(PORT_IS_PIO(pio));
    
    if (pio->PIO_PDSR & IO_MASK(pin))
    {
        return IO_PIN_HIGH;
    }
    else
    {
        return IO_PIN_LOW;
    }
}
