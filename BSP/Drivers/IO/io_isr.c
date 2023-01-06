#include <stdint.h>
#include "io.h"
#include "same70.h"
#include "RTOS.h"
#include "system.h"
#include "err.h"


#define PIO_INST_COUNT  (5u)


typedef void (*IO_Callback)(void);

static IO_Callback   *pioaHandlerTbl[IOn];
static IO_Callback   *piobHandlerTbl[IOn];
static IO_Callback   *piocHandlerTbl[IOn];
static IO_Callback   *piodHandlerTbl[IOn];
static IO_Callback   *pioeHandlerTbl[IOn];
static IRQn_Type      IO_Pio2NVICn(Pio *pio);
static uint32_t       pioaFirstIrqN           = 31;
static uint32_t       pioaLastIrqN            = 0;
static uint32_t       piobFirstIrqN           = 31;
static uint32_t       piobLastIrqN            = 0;
static uint32_t       piocFirstIrqN           = 31;
static uint32_t       piocLastIrqN            = 0;
static uint32_t       piodFirstIrqN           = 31;
static uint32_t       piodLastIrqN            = 0;
static uint32_t       pioeFirstIrqN           = 31;
static uint32_t       pioeLastIrqN            = 0;

void PIOA_IRQHandler(void);
void PIOB_IRQHandler(void);
void PIOC_IRQHandler(void);
void PIOD_IRQHandler(void);
void PIOE_IRQHandler(void);


/**
 * PIOA IRQ handler.
 */
void PIOA_IRQHandler(void)
{
    uint32_t      status;
    uint32_t      mask;
    IO_Callback   pioCb;

    OS_INT_Enter();

    status  = PIOA->PIO_ISR;
    mask    = PIOA->PIO_IMR;

    for (uint32_t i = pioaFirstIrqN; i <= pioaLastIrqN; i++)
    {
        if (((status & mask) & (1u << i)))
        {
            pioCb = (IO_Callback)pioaHandlerTbl[i];
            if (pioCb != NULL)
            {
                pioCb();
            }
        }
    }

    OS_INT_Leave();
}


/**
 * PIOB IRQ handler.
 */
void PIOB_IRQHandler(void)
{
    uint32_t      status;
    uint32_t      mask;
    IO_Callback   pioCb;

    OS_INT_Enter();

    status  = PIOB->PIO_ISR;
    mask    = PIOB->PIO_IMR;

    for (uint32_t i = piobFirstIrqN; i <= piobLastIrqN; i++)
    {
        if (((status & mask) & (1u << i)))
        {
            pioCb = (IO_Callback)piobHandlerTbl[i];
            if (pioCb != NULL)
            {
                pioCb();
            }
        }
    }

    OS_INT_Leave();
}


/**
 * PIOC IRQ handler.
 */
void PIOC_IRQHandler(void)
{
    uint32_t      status;
    uint32_t      mask;
    IO_Callback   pioCb;

    OS_INT_Enter();

    status  = PIOC->PIO_ISR;
    mask    = PIOC->PIO_IMR;

    for (uint32_t i = piocFirstIrqN; i <= piocLastIrqN; i++)
    {
        if (((status & mask) & (1u << i)))
        {
            pioCb = (IO_Callback)piocHandlerTbl[i];
            if (pioCb != NULL)
            {
                pioCb();
            }
        }
    }

    OS_INT_Leave();
}


/**
 * PIOD IRQ handler.
 */
void PIOD_IRQHandler(void)
{
    uint32_t      status;
    uint32_t      mask;
    IO_Callback   pioCb;

    OS_INT_Enter();

    status  = PIOD->PIO_ISR;
    mask    = PIOD->PIO_IMR;

    for (uint32_t i = piodFirstIrqN; i <= piodLastIrqN; i++)
    {
        if (((status & mask) & (1u << i)))
        {
            pioCb = (IO_Callback)piodHandlerTbl[i];
            if (pioCb != NULL)
            {
                pioCb();
            }
        }
    }

    OS_INT_Leave();
}


/**
 * PIOE IRQ handler.
 */
void PIOE_IRQHandler(void)
{
    uint32_t      status;
    uint32_t      mask;
    IO_Callback   pioCb;

    OS_INT_Enter();

    status  = PIOE->PIO_ISR;
    mask    = PIOE->PIO_IMR;

    for (uint32_t i = pioeFirstIrqN; i <= pioeLastIrqN; i++)
    {
        if (((status & mask) & (1u << i)))
        {
            pioCb = (IO_Callback)pioeHandlerTbl[i];
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
 * @param pio     Pointer to PIO instance.
 *
 * @param line    IO line.
 *
 * @param pfIsr   Pointer to handler to install.
 */
void IO_InstallIrqHandler(
    Pio             *pio,
    uint32_t const   line,
    void            *pfIsr)
{
    assert(IS_PIO(pio));
    assert(line  <= IOn);
    assert(pfIsr != NULL);

    if (pio == PIOA)
    {
        pioaHandlerTbl[line] = pfIsr;

        if (line <= pioaFirstIrqN)
        {
            pioaFirstIrqN = line;
        }

        if (line >= pioaLastIrqN)
        {
            pioaLastIrqN = line;
        }
    }
    else if (pio == PIOB)
    {
        piobHandlerTbl[line] = pfIsr;

        if (line <= piobFirstIrqN)
        {
            piobFirstIrqN = line;
        }

        if (line >= piobLastIrqN)
        {
            piobLastIrqN = line;
        }
    }
    else if (pio == PIOC)
    {
        piocHandlerTbl[line] = pfIsr;

        if (line <= piocFirstIrqN)
        {
            piocFirstIrqN = line;
        }

        if (line >= piocLastIrqN)
        {
            piocLastIrqN = line;
        }
    }
    else if (pio == PIOD)
    {
        piodHandlerTbl[line] = pfIsr;

        if (line <= piodFirstIrqN)
        {
            piodFirstIrqN = line;
        }

        if (line >= piodLastIrqN)
        {
            piodLastIrqN = line;
        }
    }
    else if (pio == PIOE)
    {
        pioeHandlerTbl[line] = pfIsr;

        if (line <= pioeFirstIrqN)
        {
            pioeFirstIrqN = line;
        }

        if (line >= pioeLastIrqN)
        {
            pioeLastIrqN = line;
        }
    }
    else
    {
        /* Should not end here,
         * already asserted.
         */
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

    assert(IS_PIO(pio));
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
            break;
        default:
            /* Should not get here */
            break;
    }

    pio->PIO_IER |= mask;
    
    irqn = IO_Pio2NVICn(pio);

    if (NVIC_GetEnableIRQ(irqn) == 0)
    {
        NVIC_ClearPendingIRQ(irqn);
        NVIC_SetPriority(irqn, IO_IRQ_PRIO);
        NVIC_EnableIRQ(irqn);
    }

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
    assert(IS_PIO(pio));
    
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
    assert(IS_PIO(pio));
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
    assert(IS_PIO(pio));
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
    assert(IS_PIO(pio));
    
    if (pio->PIO_PDSR & IO_MASK(pin))
    {
        return IO_PIN_HIGH;
    }
    else
    {
        return IO_PIN_LOW;
    }
}
