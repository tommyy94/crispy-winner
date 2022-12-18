/** @file */

#include "same70.h"
#include "sdramc_driver.h"
#include "pmc_driver.h"
#include "delay.h"
#include <stdio.h>


#define EBI_SDRAMC_ADDR                   (0x70000000u)
#define BOARD_SDRAM_BUSWIDTH              (16)
#define AUTO_REFRESH_CYCLES               (8)
#define EXTENDED_LOAD_MODE_REGISTER_CMD   (0xCAFE)


static uint32_t SDRAMC_CalculateCR(SDRAMC_Memory_t *pMem);


/**
 * Calculate SDRAMC Configuration register value.
 *
 * @param   pMem  Pointer to SDRAM memory.
 *
 * @retval  cr    Configuration register value.
 */
static uint32_t SDRAMC_CalculateCR(SDRAMC_Memory_t *pMem)
{
    uint32_t cr = 0;

    cr |= pMem->cfg.columnBits    & SDRAMC_CR_NC_Msk;
    cr |= pMem->cfg.rowBits       & SDRAMC_CR_NR_Msk;
    cr |= pMem->cfg.nb            & SDRAMC_CR_NB;
    cr |= pMem->cfg.cas           & SDRAMC_CR_CAS_Msk;
    cr |= pMem->cfg.dataBusWidth  & SDRAMC_CR_DBW;
    cr |= SDRAMC_CR_TWR(pMem->cfg.writeRecoveryDelay);
    cr |= SDRAMC_CR_TRC_TRFC(pMem->cfg.rowCycleDelay_RowRefreshCycle);
    cr |= SDRAMC_CR_TRP(pMem->cfg.rowPrechargeDelay);
    cr |= SDRAMC_CR_TRCD(pMem->cfg.rowColumnDelay);
    cr |= SDRAMC_CR_TRAS(pMem->cfg.activePrechargeDelay);
    cr |= SDRAMC_CR_TXSR(pMem->cfg.exitSelfRefreshActiveDelay);

    return cr;
}


/**
 * Initialize SDRAM controller.
 *
 * @param   pMem      Pointer to SDRAM memory.
 *
 * @param   clkFreg   CPU clock frequency.
 */
void SDRAMC_Init(SDRAMC_Memory_t *pMem, uint32_t clkFreq)
{
    uint32_t tmp;

    puts("SDRAM > ... Initializing...");

    PMC_PeripheralClockEnable(ID_SDRAMC);

    /* Step 1.
     * Program the SDRAM device feature into the Configuration Register.
     */
    SDRAMC->SDRAMC_CR = SDRAMC_CalculateCR(pMem);

    /* Step 2.
     * For low-power SDRAM, temperature-compensated self refresh (TCSR),
     * drive strength (DS) and partial array self refresh (PASR) must be set
     * in the Low-power Register.
     */
    SDRAMC->SDRAMC_LPR = 0;

    /* Step 3.
     * Program the memory type into the Memory Device register.
     */
    SDRAMC->SDRAMC_MDR = SDRAMC_MDR_MD_SDRAM;

    /* Step 4.
     * 200us delay to precede any signal toggle.
     */
    delayUs(200);

    /* Step 5.
     * Issue a NOP command into Mode register.
     * Must perform write to any SDRAM address to acknowledge the command.
     * This enables the clock driving the SDRAM.
     */
    SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_NOP;
    *(uint16_t *)(EBI_SDRAMC_ADDR) = 0x0000;

    /* Step 6.
     * Issue precharge command to all SDRAM banks.
     * Must perform write to any SDRAM address to acknowledge the command.
     */
    SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_ALLBANKS_PRECHARGE;
    *(uint16_t *)(EBI_SDRAMC_ADDR) = 0x0000;

    /* Need some delay after precharge */
    delayUs(200);

    /* Step 7.
     * Provide 8 Auto-Refresh cycles. Need to program the auto refresh
     * command into Mode register. The device is set to idle state and
     * must perform 8 auto refresh cycles.
     */
    for (uint32_t i = 1; i <= AUTO_REFRESH_CYCLES; i++)
    {
        SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_AUTO_REFRESH;
        *(uint16_t *)(EBI_SDRAMC_ADDR + 0) = i;
    }

    /* Step 8.
     * Configure CAS latency and burst length. Perform write to acknowledge
     * this command. The write address must be chosen so that BA[1] is set
     * to 1 and BA[0] is set to 0.
     */
    SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_LOAD_MODEREG;
    *(uint16_t *)(EBI_SDRAMC_ADDR + 0x22) = EXTENDED_LOAD_MODE_REGISTER_CMD;

    /* Step 9.
     * Set EMRS.
     */
    SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_EXT_LOAD_MODEREG;
    *((uint16_t *)(EBI_SDRAMC_ADDR + (1 << pMem->cfg.bk1))) = 0;

    /* Step 10.
     * Set normal mode and perform write to acknowledge this command.
     */
    SDRAMC->SDRAMC_MR = SDRAMC_MR_MODE_NORMAL;
    *(uint16_t *)(EBI_SDRAMC_ADDR) = 0x0000;

    /* Step 11.
     * Write refresh rate.
     */
    tmp = clkFreq / 1000u;
    tmp *= 15625u;
    tmp /= 1000000u;
    SDRAMC->SDRAMC_TR = SDRAMC_TR_COUNT(tmp);

    puts("SDRAM > ... Init done");
}
