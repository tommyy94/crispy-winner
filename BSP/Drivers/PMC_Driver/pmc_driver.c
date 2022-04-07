#include "same70.h"
#include "pmc_driver.h"
#include "logWriter.h"


/*
 * @brief   Enable peripheral clock.
 *
 * @param   pid   Peripheral ID.
 *
 * @return  None.
 */
void PMC_PeripheralClockEnable(const uint32_t pid)
{
    assert(pid < ID_PERIPH_COUNT);

    /* Peripheral Clock Enable Register is
     * divided into two 32-bit registers.
     * 
     * PCER0 = PID0...PID31
     * PCER1 = PID32...PID63
     */
    if (pid < 32)
    {
        PMC->PMC_PCER0 |= (1 << pid);
    }
    else if (pid < 64)
    {
        PMC->PMC_PCER1 |= (1 << (pid - 32));
    }
    else
    {

    }
}


/*
 * @brief   Disable peripheral clock.
 *
 * @param   pid   Peripheral ID.
 *
 * @return  None.
 */
void PMC_PeripheralClockDisable(const uint32_t pid)
{
    assert(pid < ID_PERIPH_COUNT);

    /* Peripheral Clock Disable Register is
     * divided into two 32-bit registers.
     * 
     * PCDR0 = PID0...PID31
     * PCDR1 = PID32...PID63
     */
    if (pid < 32)
    {
        PMC->PMC_PCDR0 |= (1 << pid);
    }
    else if (pid < 64)
    {
        PMC->PMC_PCDR1 |= (1 << (pid - 32));
    }
    else
    {

    }
}
