/** @file */

#ifndef SDRAMC_DRIVER_H
#define SDRAMC_DRIVER_H

#include <stdint.h>
#include <stdbool.h>


typedef struct
{
    bool     isLpSdr;         /**< Low Power SDRAM                    */
    uint32_t size;            /**< Size in bytes                      */

    /* Parameters                                                     */
    uint32_t columnBits;      /**< Number of Column Bits              */
    uint32_t rowBits;         /**< Number of Row Bits                 */
    uint32_t banks;           /**< Number of Banks                    */
    uint32_t dataBusWidth;    /**< Data Bus Width                     */
    uint32_t cas;             /**< CAS Latency (tCAS)                 */

    /* Timing parameters                                              */
    struct
    {
        uint32_t twr;         /**< Write Recovery Delay               */
        uint32_t trc;         /**< Row Cycle Delay                    */
        uint32_t trfc;        /**< Row Refresh Cycle                  */
        uint32_t trp;         /**< Row Precharge Delay                */
        uint32_t trcd;        /**< Row to Column Delay                */
        uint32_t tras;        /**< Active to Precharge Delay          */
        uint32_t txsr;        /**< Exit Self Refresh to Active Delay  */
        uint32_t tmrd;        /**< Load Mode Register Command to
                               *   Active or Refresh Command          */
    } timings;
} SDRAMC_Descriptor_t;


int32_t SDRAMC_Init(SDRAMC_Descriptor_t *pDesc, uint32_t clkFreq);

#endif /* SDRAMC_DRIVER_H */
