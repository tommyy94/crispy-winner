/** @file */

#include "sdram.h"
#include "trace.h"


#define RELAXED_TIMINGS

#define BYTE_TO_MBIT(x)   (((x) << 3) >> 20)


static void IS42S16100F_Init(SDRAMC_Descriptor_t *pDesc);


/**
 * Calculate SDRAMC Configuration register value.
 *
 * @param[in] dev       SDRAM device.
 *
 * @param[in] clkFreq   Master clock frequency.
 *
 * @retval    ret       SDRAM_OK if init pass.
 */
int32_t SDRAM_Init(SDRAM_Device_t dev, uint32_t clkFreq)
{
    int32_t             ret;
    SDRAMC_Descriptor_t desc;

    TRACE_INFO("SDRAM > Initializing...");

    if (dev == IS42S16100F)
    {
        IS42S16100F_Init(&desc);
    }
    else
    {
        return SDRAM_INVALID_CHIP;
    }

    ret = SDRAMC_Init(&desc, clkFreq);
    if (ret == SDRAM_OK)
    {
        TRACE_INFO(
            " done!\r\n"
            "\r\n"
            "##### IS42S16100F 200 MHz SDRAM #####\r\n"
            "Size:          %u Mb\r\n"
            "Bus width:     %u-bit\r\n"
            "Row bits:      %u\r\n"
            "Column bits:   %u\r\n"
            "Banks:         %u\r\n"
            "CAS:           %u\r\n"
            "Timing parameters (cycles):\r\n"
            "  tWR:  %u\r\n"
            "  tRC:  %u\r\n"
            "  tRFC: %u\r\n"
            "  tRCD: %u\r\n"
            "  tRAS: %u\r\n"
            "  tRP:  %u\r\n"
            "  tXSR: %u\r\n"
            "  tMRD: %u\r\n",
            BYTE_TO_MBIT(desc.size),
            desc.dataBusWidth,
            desc.rowBits,
            desc.columnBits,
            desc.banks,
            desc.cas,
            desc.timings.twr,
            desc.timings.trc,
            desc.timings.trfc,
            desc.timings.trcd,
            desc.timings.tras,
            desc.timings.trp,
            desc.timings.txsr,
            desc.timings.tmrd);
    }
    else
    {
        TRACE_INFO("fail!\r\n");
    }

    return ret;
}


/**
 * Configure ISSI IS42S16100F SDRAM.
 *
 * @param[in] pDesc  Pointer to SDRAM descriptor.
 */
static void IS42S16100F_Init(SDRAMC_Descriptor_t *pDesc)
{
    /* IS42S16100F-5BL mounted on board
     * => Obsolote, replaced by IS42S16100H
     * 200 MHz SDRAM
     */
    pDesc->isLpSdr          = false;
    pDesc->size             = 0x00200000;

    /* Parameters */
    pDesc->dataBusWidth     = 16;
    pDesc->columnBits       = 8;
    pDesc->rowBits          = 11;
    pDesc->banks            = 2;
    pDesc->cas              = 3;

#ifdef RELAXED_TIMINGS
    /* Timing parameters (relaxed) */
    pDesc->timings.twr      = 5;
    pDesc->timings.trc      = 13;
    pDesc->timings.trfc     = 13;
    pDesc->timings.trcd     = 5;
    pDesc->timings.tras     = 9;
    pDesc->timings.trp      = 5,
    pDesc->timings.txsr     = 15;
    pDesc->timings.tmrd     = 2;
#else
    /* Timing parameters (datasheet) */
    pDesc->timings.twr      = 2; 
    pDesc->timings.trc      = 10; 
    pDesc->timings.trfc     = 10; 
    pDesc->timings.trcd     = 3; 
    pDesc->timings.tras     = 7; 
    pDesc->timings.trp      = 3; 
    pDesc->timings.txsr     = 8;
    pDesc->timings.tmrd     = 2;
#endif /* RELAXED_TIMINGS */
}
