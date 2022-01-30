#include <same70.h>
#include "pwm_driver.h"
#include "pio.h"
#include "logWriter.h"


static void     PWM_IO_Init(void);
static uint16_t PWM_GetChannelDutyCycle(float dutyPer);


/* Could use 2-20 kHz signal for driving DC motors.
 * Center-aligned mode seems to be the best for H-bridge.
 */
void PWM_Init(void)
{
    uint16_t chDuty;

    PWM_IO_Init();

    /**
     * Enable PWM clock gating
     * - PWM0 & PWM1 clock = 75 MHz
     */
    PMC->PMC_PCR = PMC_PCR_CMD | PMC_PCR_PID(ID_PWM0) | PMC_PCR_EN;
    PMC->PMC_PCR = PMC_PCR_CMD | PMC_PCR_PID(ID_PWM1) | PMC_PCR_EN;
                                  
    /* clkA & clkB = 75 MHz / 16 = 4,68 MHz => center-aligned so 2,34 MHz */
    PWM0->PWM_CLK = PWM_CLK_PREA_CLK_DIV16 | PWM_CLK_DIVA_PREA;
    PWM1->PWM_CLK = PWM_CLK_PREA_CLK_DIV16 | PWM_CLK_DIVA_PREA;
    
    /* Center-aligned, clk A as source */
    PWM0->PWM_CH_NUM[PWM_CHANNEL0].PWM_CMR = PWM_CMR_CPRE_CLKA | PWM_CMR_CALG;
    PWM0->PWM_CH_NUM[PWM_CHANNEL1].PWM_CMR = PWM_CMR_CPRE_CLKA | PWM_CMR_CALG;
    PWM0->PWM_CH_NUM[PWM_CHANNEL3].PWM_CMR = PWM_CMR_CPRE_CLKA | PWM_CMR_CALG;
    PWM1->PWM_CH_NUM[PWM_CHANNEL0].PWM_CMR = PWM_CMR_CPRE_CLKA | PWM_CMR_CALG;

    /* period = (2 * 2^PREx * CPRD * DIVx) / Pclk
     *        = (2 * 16 * 1175 * 1) / 75 MHz
     *        = 0,0005s
     *
     * Channel period for desired frequency:
     * f(T) = (T * (75 * 10^6)) / 32
     */
    PWM0->PWM_CH_NUM[PWM_CHANNEL0].PWM_CPRD = PWM_CPRD_CPRD(freq_tbl[PWM_FREQUENCY]);
    PWM0->PWM_CH_NUM[PWM_CHANNEL1].PWM_CPRD = PWM_CPRD_CPRD(freq_tbl[PWM_FREQUENCY]);
    PWM0->PWM_CH_NUM[PWM_CHANNEL3].PWM_CPRD = PWM_CPRD_CPRD(freq_tbl[PWM_FREQUENCY]);
    PWM1->PWM_CH_NUM[PWM_CHANNEL0].PWM_CPRD = PWM_CPRD_CPRD(freq_tbl[PWM_FREQUENCY]);
    
    /* Duty cycle = ((period / 2) - 1 / channel_clk x CDTY) / (period / 2)
     *            = ((0,0005s / 2) - 1 / 4,68 MHz x CDTY) / (0,0005s / 2)
     * 
     * Converting desired duty cycle to CDTY:
     *    D = ((0,0005s / 2) - 1 / 4,68 MHz x CDTY) / (0,0005s / 2)
     * => D = ((0,00025) - 1 / (4,68 MHz x CDTY) / (0,00025)      | x 0.00025
     * => 0.00025 x D = 0.00025 - 1 / (4,68 MHz x CDTY)           | - 0.00025
     * => 0.00025 x D - 0.00025 = -1 / (4,68 MHz x CDTY)          | x 4,68 MHz
     * => 1170 x CDTY x D - 1170 x CDTY = -1                      | / 1170(D - 1)
     * => CDTY = -(1 / 1170(D - 1))
     *
     * The result is in the divisor, so:
     *    f(D)    = (-((1170 * D) - 1170))
     */
    chDuty = PWM_GetChannelDutyCycle(50.0);
    PWM0->PWM_CH_NUM[PWM_CHANNEL0].PWM_CDTY = PWM_CDTY_CDTY(chDuty);
    PWM0->PWM_CH_NUM[PWM_CHANNEL1].PWM_CDTY = PWM_CDTY_CDTY(chDuty);
    PWM0->PWM_CH_NUM[PWM_CHANNEL3].PWM_CDTY = PWM_CDTY_CDTY(chDuty);
    PWM1->PWM_CH_NUM[PWM_CHANNEL0].PWM_CDTY = PWM_CDTY_CDTY(chDuty);

    /* Enable PWM channels */
    PWM0->PWM_ENA = PWM_ENA_CHID3 | PWM_ENA_CHID1 | PWM_ENA_CHID0;
    PWM1->PWM_ENA = PWM_ENA_CHID0;
}


/* PWM0 channels:
 *    PA0:  PWM0_CH0+
 *    PA2:  PWM0_CH1+
 *    PB4:  PWM0_CH2+
 *    PA17: PWM0_CH3+
 */
static void PWM_IO_Init(void)
{    
    /*
     * _______________________________________
     * |            |           |             |
     * |  ABCDSR1   |  ABCDSR2  | Peripheral  |
     * |------------+-----------+-------------|
     * |     0      |     0     |     A       |
     * |     1      |     0     |     B       |
     * |     0      |     1     |     C       |
     * |     1      |     1     |     D       |
     * |____________|___________|_____________|
     */

    /* Select peripheral A for PA2 & PA0 */
    PIOA->PIO_ABCDSR[0] &= ~(PIO_ABCDSR_P2 | PIO_ABCDSR_P0);
    PIOA->PIO_ABCDSR[1] &= ~(PIO_ABCDSR_P2 | PIO_ABCDSR_P0);
    
    /* Select peripheral C for PA17 & PA12 */
    PIOA->PIO_ABCDSR[0] &= ~PIO_ABCDSR_P17 | PIO_ABCDSR_P12;
    PIOA->PIO_ABCDSR[1] |=  PIO_ABCDSR_P17 | PIO_ABCDSR_P12;

    /* Set peripheral function */
    PIOA->PIO_PDR |= PIO_ABCDSR_P17 | PIO_ABCDSR_P12
                  |  PIO_ABCDSR_P2  | PIO_ABCDSR_P0;

    /* Enabling the pullup distorts the signal */
    //PIO_ConfigurePull(PIOA, pioaMask, PIO_PULLUP);
}


void PWM_UpdateDutyCycle(Pwm *pwm, PWM_Channel ch, uint32_t ulDutyCycle)
{
  assert(ch < PWM_CHANNEL_COUNT);

  pwm->PWM_CH_NUM[ch].PWM_CDTYUPD = PWM_CDTYUPD_CDTYUPD(ulDutyCycle);
}


void PWM_UpdateFrequency(Pwm *pwm, PWM_Channel ch, Frequency freq)
{
  assert(ch < PWM_CHANNEL_COUNT);
  pwm->PWM_CH_NUM[ch].PWM_CPRDUPD = PWM_CPRDUPD_CPRDUPD(freq_tbl[freq]);
}


static uint16_t PWM_GetChannelDutyCycle(float dutyPer)
{
  /* Step can be calculated as follows:
   *    f(D)    = (-((1170 * D) - 1170))
   * => f(0.99) = 11.7
   */
  const float step = 11.7;

  /* Calculate positive duty cycle */
  dutyPer = 100.0 - dutyPer;
  return (uint16_t)(dutyPer * step);
}


/* These functions could be used when accelerating/decelerating
 * the device.
 *
 * NOTE: Might have to use a lookup table for the values,
 * to avoid DMA interrupt between each update.
 */
void PWM_IncrementDutyCycle(Pwm *pwm, PWM_Channel ch, uint32_t steps)
{
  assert(ch < PWM_CHANNEL_COUNT);
  uint16_t curDuty;
  const float step = 11.7;

  curDuty  = pwm->PWM_CH_NUM[ch].PWM_CDTY & PWM_CDTY_CDTY_Msk;
  curDuty += (uint16_t)(step * steps); /* CDTY rounded down */
  pwm->PWM_CH_NUM[ch].PWM_CDTYUPD = PWM_CDTYUPD_CDTYUPD(curDuty);
}


/* Decelerating could be done by inverting the PWM polarity and
 * updating the registers. The register value should be swapped to
 * the beginning/end of the lookup table.
 */
void PWM_DecrementDutyCycle(Pwm *pwm, PWM_Channel ch, uint32_t steps)
{
  assert(ch < PWM_CHANNEL_COUNT);
  uint16_t curDuty;
  const float step = 11.7;

  curDuty  = pwm->PWM_CH_NUM[ch].PWM_CDTY & PWM_CDTY_CDTY_Msk;
  curDuty -= (uint16_t)(step * steps); /* CDTY rounded down */
  pwm->PWM_CH_NUM[ch].PWM_CDTYUPD = PWM_CDTYUPD_CDTYUPD(curDuty);
}


void PWM_Enable(Pwm *pwm, PWM_Channel ch)
{
    //assert(ch <= PWM_DIS_CHID3_Pos);
    //pwm->PWM_ENA = (0x1U << ch) & PWM_ENA;
}


void PWM_Disable(Pwm *pwm, PWM_Channel ch)
{
    //assert(ch <= PWM_DIS_CHID3_Pos);
    //pwm->PWM_DIS = (0x1U << ch) & PWM_DIS;
}
