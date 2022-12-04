#include "same70.h"
#include "pmc_driver.h"
#include "err.h"


#define PCK_COUNT   (7)


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


/*
 * Initialize programmable clock n.
 *
 * @param   n     Peripheral clock number.
 *
 * @param   pres  Selected clock is divided by pres+1.
 *
 * @param   src   Clock source.
 *
 * @return  None.
 */
void PMC_ProgrammableClockInit(
    uint32_t      const   n,
    uint8_t       const   pres,
    PCK_Source_t  const   src)
{
    assert(n <= PCK_COUNT);

    PMC->PMC_PCK[n] = PMC_PCK_PRES(pres) | PMC_PCK_CSS(src);
}


/*
 * Enable programmable clock n.
 *
 * @param   n     Peripheral clock number.
 *
 * @return  None.
 */
void PMC_ProgrammableClockEnable(const uint32_t n)
{
    assert(n <= PCK_COUNT);
    PMC->PMC_SCER = PMC_SCER_PCK0 << n;
}


/*
 * Disable programmable clock n.
 *
 * @param   n     Peripheral clock number.
 *
 * @return  None.
 */
void PMC_ProgrammableClockDisable(const uint32_t n)
{
    assert(n <= PCK_COUNT);
    PMC->PMC_SCDR = PMC_SCDR_PCK0 << n;
}
