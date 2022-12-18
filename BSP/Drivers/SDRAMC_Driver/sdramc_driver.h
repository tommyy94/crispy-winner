/** @file */

#ifndef SDRAMC_DRIVER_H
#define SDRAMC_DRIVER_H


typedef struct
{
    uint32_t columnBits;                     // Number of Column Bits
    uint32_t rowBits;                        // Number of Row Bits
    uint32_t nb;                             // Number of Banks
    uint32_t dataBusWidth;                   // Data Bus Width
    uint32_t cas;                            // CAS Latency (tCAC)
    uint32_t writeRecoveryDelay;             // Write Recovery Delay (tDPL / tWR)
    uint32_t rowCycleDelay_RowRefreshCycle;  // Row Cycle Delay and Row Refresh Cycle (tRC & tRFC)
    uint32_t rowPrechargeDelay;              // Row Precharge Delay (tRP)
    uint32_t rowColumnDelay;                 // Row to Column Delay (tRCD)
    uint32_t activePrechargeDelay;           // Active to Precharge Delay (tRAS)
    uint32_t exitSelfRefreshActiveDelay;     // Exit Self Refresh to Active Delay (tXSR / tXSRD / tSRFX)
    uint32_t bk1;                            // bk1 addr
} SDRAMC_Config_t;

typedef struct
{
    SDRAMC_Config_t cfg;

} SDRAMC_Memory_t;

#define SDRAMC_DBUS_WIDTH_16    (SDRAMC_CR_DBW) /* Only 16-bit support */


void SDRAMC_Init(SDRAMC_Memory_t *pMem, uint32_t clkFreq);

#endif /* SDRAMC_DRIVER_H */
