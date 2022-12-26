/** @file */

#include "same70.h"
#include "sdramc_driver.h"
#include "sdram.h"
#include "pmc_driver.h"
#include "delay.h"
#include "io.h"
#include "system.h"
#include "err.h"


#define SDRAMC_REFRESH_TIMER_COUNT(clk)   ((clk) / 1000u * 15625u / 1000000u)

#define EBI_SDRAMC_ADDR                   (0x70000000u)
#define CMD_DELAY_US                      (200u)
#define SDRAMC_MEMTEST_PATTERN            (0xCAFE)


static void     SDRAMC_IO_Init(void);
static uint32_t SDRAMC_CalculateCR(SDRAMC_Descriptor_t *pDesc);
static void     SDRAMC_SendCommand(uint32_t cmd);
static void     SDRAMC_SendCommandExt(uint32_t cmd, uint32_t offset);
static int32_t  SDRAMC_MemTest(uint32_t base, uint32_t size);


/**
 * Calculate SDRAMC Configuration register value.
 *
 * @param[in] pDesc  Pointer to SDRAM descriptor.
 *
 * @retval    cr    Configuration register value.
 */
static uint32_t SDRAMC_CalculateCR(SDRAMC_Descriptor_t *pDesc)
{
    uint32_t cr = 0;

    switch (pDesc->rowBits)
    {
        case 11:
            cr |= SDRAMC_CR_NR_ROW11;
            break;
        case 12:
            cr |= SDRAMC_CR_NR_ROW12;
            break;
        case 13:
            cr |= SDRAMC_CR_NR_ROW13;
            break;
        default:
            assert(false);
            break;
    }

    switch (pDesc->rowBits)
    {
        case 8:
            cr |= SDRAMC_CR_NC_COL8;
            break;
        case 9:
            cr |= SDRAMC_CR_NC_COL9;
            break;
        case 10:
            cr |= SDRAMC_CR_NC_COL10;
            break;
        case 11:
            cr |= SDRAMC_CR_NC_COL11;
            break;
        default:
            assert(false);
            break;
    }

    switch (pDesc->banks)
    {
        case 2:
            cr |= SDRAMC_CR_NB_BANK2;
            break;
        case 4:
            cr |= SDRAMC_CR_NB_BANK4;
            break;
        default:
            assert(false);
            break;
    }

    switch (pDesc->dataBusWidth)
    {
        case 16:
            cr |= SDRAMC_CR_DBW;
            break;
        case 32:
            /* Do nothing */
            break;
        default:
            assert(false);
            break;
    }

    switch (pDesc->cas)
    {
        case 1:
            cr |= SDRAMC_CR_CAS_LATENCY1;
            break;
        case 2:
            cr |= SDRAMC_CR_CAS_LATENCY2;
            break;
        case 3:
            cr |= SDRAMC_CR_CAS_LATENCY3;
            break;
        default:
            assert(false);
            break;
    }

    cr |= SDRAMC_CR_TWR(pDesc->timings.twr);
    if (pDesc->timings.trc > pDesc->timings.trfc)
    {
        cr |= SDRAMC_CR_TRC_TRFC(pDesc->timings.trc);
    }
    else
    {
        cr |= SDRAMC_CR_TRC_TRFC(pDesc->timings.trfc);
    }
    cr |= SDRAMC_CR_TRP(pDesc->timings.trp);
    cr |= SDRAMC_CR_TRCD(pDesc->timings.trcd);
    cr |= SDRAMC_CR_TRAS(pDesc->timings.tras);
    cr |= SDRAMC_CR_TXSR(pDesc->timings.txsr);

    return cr;
}


/**
 * Configure SDRAMC IO pins.
 */
static void SDRAMC_IO_Init(void)
{
    uint32_t mask;

    /* Data pins D0-D16 */
    mask = PIO_PC7A_D7 | PIO_PC6A_D6 | PIO_PC5A_D5 | PIO_PC4A_D4
         | PIO_PC3A_D3 | PIO_PC2A_D2 | PIO_PC1A_D1 | PIO_PC0A_D0;
    IO_SetPeripheralFunction(PIOC, mask, IO_PERIPH_A);
    mask = PIO_PE5A_D13 | PIO_PE4A_D12 | PIO_PE3A_D11
         | PIO_PE2A_D10 | PIO_PE1A_D9  | PIO_PE0A_D8;
    IO_SetPeripheralFunction(PIOE, mask, IO_PERIPH_A);
    IO_SetPeripheralFunction(PIOA, PIO_PA16A_D15 | PIO_PA15A_D14, IO_PERIPH_A);

    /* Address pins A2-A11 AND SDA10, A0-A1 not connected */
    mask = PIO_PC29A_A11 | PIO_PC28A_A10 | PIO_PC27A_A9| PIO_PC26A_A8
         | PIO_PC25A_A7 | PIO_PC24A_A6 | PIO_PC23A_A5 | PIO_PC22A_A4
         | PIO_PC21A_A3 | PIO_PC20A_A2;
    IO_SetPeripheralFunction(PIOC, mask, IO_PERIPH_A);
    IO_SetPeripheralFunction(PIOD, PIO_PD13C_SDA10, IO_PERIPH_C);

    /* Bank address */
    IO_SetPeripheralFunction(PIOA, PIO_PA20C_BA0, IO_PERIPH_C);

    /* Data mask enable signals */
    IO_SetPeripheralFunction(PIOC, PIO_PC18A_NBS0, IO_PERIPH_A);
    IO_SetPeripheralFunction(PIOD, PIO_PD15C_NBS1, IO_PERIPH_C);

    /* Latency parameters */
    IO_SetPeripheralFunction(PIOD, PIO_PD17C_CAS | PIO_PD16C_RAS, IO_PERIPH_C);

    /* Clock signals */
    IO_SetPeripheralFunction(PIOD, PIO_PD23C_SDCK | PIO_PD14C_SDCKE, IO_PERIPH_C);
    
    /* Control signals */
    IO_SetPeripheralFunction(PIOD, PIO_PD29C_SDWE, IO_PERIPH_C);
    IO_SetPeripheralFunction(PIOC, PIO_PC15A_SDCS, IO_PERIPH_A);
}


/**
 * Send command to SDRAM.
 *
 * @param[in] cmd   SDRAM command.
 */
static void SDRAMC_SendCommand(uint32_t cmd)
{
    SDRAMC->SDRAMC_MR = cmd & SDRAMC_MR_MODE_Msk;
    (void)SDRAMC->SDRAMC_MR;
    __DMB();

    /* Perform a write to the SDRAM to acknowledge the command */
    *(uint16_t *)EBI_SDRAMC_ADDR = 0x0;
}


/**
 * Send extended command to SDRAM.
 *
 * @param[in] cmd     SDRAM command.
 *
 * @param     offset  Offset to SDRAM command.
 */
static void SDRAMC_SendCommandExt(
    uint32_t cmd,
    uint32_t offset)
{
    SDRAMC->SDRAMC_MR = cmd & SDRAMC_MR_MODE_Msk;
    (void)SDRAMC->SDRAMC_MR;
    __DMB();

    /* Perform a write to the SDRAM to acknowledge the command */
    *((uint16_t *)(EBI_SDRAMC_ADDR + offset)) = 0x00;
}


/**
 * Test to ensure SDRAM is configured properly.
 *
 * @param[in] base    SDRAM base address.
 *
 * @param[in] size    SDRAM size.
 *
 * @retval    SDRAM_OK/SDRAM_MEMTEST_ERR
 */
static int32_t SDRAMC_MemTest(uint32_t base, uint32_t size)
{
    uint16_t  val;
    uint16_t *ptr ;

    ptr = (uint16_t *)(base + size);
    for (uint32_t i = (size / sizeof(uint16_t)); i > 0; i -= 8)
    {
        *(--ptr) = SDRAMC_MEMTEST_PATTERN;
        *(--ptr) = SDRAMC_MEMTEST_PATTERN;
        *(--ptr) = SDRAMC_MEMTEST_PATTERN;
        *(--ptr) = SDRAMC_MEMTEST_PATTERN;
        *(--ptr) = SDRAMC_MEMTEST_PATTERN;
        *(--ptr) = SDRAMC_MEMTEST_PATTERN;
        *(--ptr) = SDRAMC_MEMTEST_PATTERN;
        *(--ptr) = SDRAMC_MEMTEST_PATTERN;
    }

    ptr = (uint16_t *)(base + size);
    for (uint32_t i = (size / sizeof(uint16_t)); i > 0; i--)
    {
        val = *(--ptr);
        if (val != SDRAMC_MEMTEST_PATTERN)
        {
            return SDRAM_MEMTEST_ERR;
        }
    }

    return SDRAM_OK;
}


/**
 * Initialize SDRAM controller.
 *
 * @param[in]   pDesc     Pointer to SDRAM descriptor.
 *
 * @param[in]   clkFreq   Master clock frequency.
 *
 * @retval   ret       SDRAMC_OK if success.
 */
int32_t SDRAMC_Init(SDRAMC_Descriptor_t *pDesc, uint32_t clkFreq)
{
    uint32_t bk1;

    PMC_PeripheralClockEnable(ID_SDRAMC);

    MATRIX->CCFG_SMCNFCS |= CCFG_SMCNFCS_SDRAMEN;

    SDRAMC_IO_Init();
    
    /* bk1 = 1(M0)+8(Col)+11(Row)+1(BK1): */
    if (pDesc->banks == 2)
    {
        bk1 = 1 + pDesc->columnBits + pDesc->rowBits + 1;
    }
    else /* if (pDesc->banks == 4) */
    {
        bk1 = 1 + pDesc->columnBits + pDesc->rowBits + 2;
    }

    /* Step 1.
     * Program the SDRAM device feature into the Configuration Register.
     */
    SDRAMC->SDRAMC_CR = SDRAMC_CalculateCR(pDesc);

    SDRAMC->SDRAMC_CFR1 = SDRAMC_CFR1_UNAL | SDRAMC_CFR1_TMRD(pDesc->timings.tmrd);

    /* Step 2.
     * For low-power SDRAM, temperature-compensated self refresh (TCSR),
     * drive strength (DS) and partial array self refresh (PASR) must be set
     * in the Low-power Register.
     */
    if (pDesc->isLpSdr)
    {
       SDRAMC->SDRAMC_LPR = SDRAMC_LPR_LPCB_SELF_REFRESH
                          | SDRAMC_LPR_PASR(0)  /* Full refresh     */
                          | SDRAMC_LPR_TCSR(0)  /* No compensation  */
                          | SDRAMC_LPR_DS(2)    /* Quarter          */
                          | SDRAMC_LPR_TIMEOUT_LP_LAST_XFER_128;
    }

    /* Step 3.
     * Program the memory type into the Memory Device register.
     */
    SDRAMC->SDRAMC_MDR = SDRAMC_MDR_MD_SDRAM;

    /* Step 4.
     * 200us delay to precede any signal toggle.
     */
    delayUs(CMD_DELAY_US);

    /* Step 5.
     * Issue a NOP command into Mode register.
     * Must perform write to any SDRAM address to acknowledge the command.
     * This enables the clock driving the SDRAM.
     */
    SDRAMC_SendCommand(SDRAMC_MR_MODE_NOP);

    /* Step 6.
     * Issue precharge command to all SDRAM banks.
     * Must perform write to any SDRAM address to acknowledge the command.
     */
    SDRAMC_SendCommand(SDRAMC_MR_MODE_ALLBANKS_PRECHARGE);

    /* Need some delay after precharge */
    delayUs(CMD_DELAY_US);

    /* Step 7.
     * Provide 8 Auto-Refresh cycles. Need to program the auto refresh
     * command into Mode register. The device is set to idle state and
     * must perform 8 auto refresh cycles.
     */
    SDRAMC_SendCommand(SDRAMC_MR_MODE_AUTO_REFRESH);
    SDRAMC_SendCommand(SDRAMC_MR_MODE_AUTO_REFRESH);
    SDRAMC_SendCommand(SDRAMC_MR_MODE_AUTO_REFRESH);
    SDRAMC_SendCommand(SDRAMC_MR_MODE_AUTO_REFRESH);
    SDRAMC_SendCommand(SDRAMC_MR_MODE_AUTO_REFRESH);
    SDRAMC_SendCommand(SDRAMC_MR_MODE_AUTO_REFRESH);
    SDRAMC_SendCommand(SDRAMC_MR_MODE_AUTO_REFRESH);
    SDRAMC_SendCommand(SDRAMC_MR_MODE_AUTO_REFRESH);

    /* Step 8.
     * Configure CAS latency and burst length. Perform write to acknowledge
     * this command. The write address must be chosen so that BA[1] is set
     * to 1 and BA[0] is set to 0.
     */
    SDRAMC_SendCommandExt(SDRAMC_MR_MODE_LOAD_MODEREG, 0x00);

    /* Step 9.
     * Set EMRS if device is LPSDRAM. The write address must be chosen so
     * that BA[1] is set to 1 and BA[0] is set to 0.
     */
    if (pDesc->isLpSdr)
    {
        SDRAMC_SendCommandExt(SDRAMC_MR_MODE_EXT_LOAD_MODEREG, 1 << bk1);
    }

    /* Step 10.
     * Set normal mode and perform write to acknowledge this command.
     */
    SDRAMC_SendCommand(SDRAMC_MR_MODE_NORMAL);

    /* Step 11.
     * Write the refresh rate.
     */
    SDRAMC->SDRAMC_TR = SDRAMC_TR_COUNT(SDRAMC_REFRESH_TIMER_COUNT(clkFreq));

    return SDRAMC_MemTest(EBI_SDRAMC_ADDR, pDesc->size);
}
