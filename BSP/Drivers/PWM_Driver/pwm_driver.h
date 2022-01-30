#ifndef PWM_H_
#define PWM_H_

#include <same70.h>


typedef enum
{
    FREQ_2KHZ = 0,
    FREQ_3KHZ,
    FREQ_4KHZ,
    FREQ_5KHZ,
    FREQ_6KHZ,
    FREQ_7KHZ,
    FREQ_8KHZ,
    FREQ_9KHZ,
    FREQ_10KHZ,
    FREQ_11KHZ,
    FREQ_12KHZ,
    FREQ_13KHZ,
    FREQ_14KHZ,
    FREQ_15KHZ,
    FREQ_16KHZ,
    FREQ_17KHZ,
    FREQ_18KHZ,
    FREQ_19KHZ,
    FREQ_20KHZ,
    FREQ_COUNT
} Frequency;


typedef enum
{
    PWM_CHANNEL0 = 0,
    PWM_CHANNEL1,
    PWM_CHANNEL2,
    PWM_CHANNEL3,
    PWM_CHANNEL_COUNT
} PWM_Channel;

#define PWM_FREQUENCY (FREQ_2KHZ)


#define DUTY_CYCLE_COUNT  (100u)

extern const uint16_t dutyCycle[DUTY_CYCLE_COUNT];
extern const uint16_t freq_tbl[FREQ_COUNT];


void PWM_Init(void);
void PWM_UpdateDutyCycle(Pwm *pwm, PWM_Channel ch, uint32_t ulDutyCycle);
void PWM_UpdateFrequency(Pwm *pwm, PWM_Channel ch, Frequency freq);
void PWM_IncrementDutyCycle(Pwm *pwm, PWM_Channel ch, uint32_t steps);
void PWM_DecrementDutyCycle(Pwm *pwm, PWM_Channel ch, uint32_t steps);
void PWM_Enable(Pwm *pwm, PWM_Channel ch);
void PWM_Disable(Pwm *pwm, PWM_Channel ch);


#endif /* PWM_H_ */
