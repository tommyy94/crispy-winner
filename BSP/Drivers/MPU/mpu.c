#include <stdbool.h>
#include "mpu.h"
#include "same70.h"
#include "SEGGER_RTT_Conf.h"


#define MPU_ZONE_CNT    (16u)


bool MPU_Config(void)
{
    bool              ret                  = false;
    ARM_MPU_Region_t  regTbl[MPU_ZONE_CNT] =
    {
        {
            /* Flash */
            ARM_MPU_RBAR(0, 0x00400000),
            ARM_MPU_RASR_EX(false, ARM_MPU_AP_RO, ARM_MPU_ACCESS_NORMAL(ARM_MPU_CACHEP_WB_WRA, ARM_MPU_CACHEP_WB_WRA, false), false, ARM_MPU_REGION_SIZE_1MB)
        },
        {
            /* SRAM */
            ARM_MPU_RBAR(1, 0x20400000),
            ARM_MPU_RASR_EX(true, ARM_MPU_AP_FULL, ARM_MPU_ACCESS_NORMAL(ARM_MPU_CACHEP_WB_WRA, ARM_MPU_CACHEP_WB_WRA, false), false, ARM_MPU_REGION_SIZE_256KB)
        },
        {
            /* Peripherals */
            ARM_MPU_RBAR(2, 0x40000000),
            ARM_MPU_RASR_EX(true, ARM_MPU_AP_FULL, ARM_MPU_ACCESS_DEVICE(true), false, ARM_MPU_REGION_SIZE_512MB)
        },
        {
            /* SDRAM */
            ARM_MPU_RBAR(3, 0x70000000),
            ARM_MPU_RASR_EX(true, ARM_MPU_AP_FULL, ARM_MPU_ACCESS_NORMAL(ARM_MPU_CACHEP_NOCACHE, ARM_MPU_CACHEP_NOCACHE, true), false, ARM_MPU_REGION_SIZE_2MB)
        }
    };

    if (MPU->TYPE != 0)
    {
        uint32_t i;

        ARM_MPU_Disable();
    
        for (i = 0; i < 4; i++)
        {
            ARM_MPU_SetRegionEx(i, regTbl[i].RBAR, regTbl[i].RASR);
        }

        while (i < MPU_ZONE_CNT)
        {
            ARM_MPU_ClrRegion(i++);
        }

        ARM_MPU_Enable(ARM_MPU_AP_NONE);

        ret = true;
    }

    return ret;
}
