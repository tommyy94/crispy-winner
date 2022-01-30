/* System includes */
#include <stdlib.h>
#include <stdbool.h>

/* User includes */
#include "throttle.h"
#include "pwm_driver.h"
#include "logWriter.h"

/* RTOS includes */
#include "RTOS.h"


#define THROTTLE_TIMEOUT_MS   (20ul)
#define THROTTLE_RESOLUTION   (UINT16_MAX / 2)

#define DUTY_CYCLE_MAX        (0x494)
#define DUTY_SCALE            ((DUTY_CYCLE_MAX << 14) / UINT16_MAX)

#define DEADZONE_MIN          (THROTTLE_RESOLUTION - (THROTTLE_RESOLUTION * 0.025))
#define DEADZONE_MAX          (THROTTLE_RESOLUTION + (THROTTLE_RESOLUTION * 0.025))


typedef enum
{
    THROTTLE_CH_0 = 0,
    THROTTLE_CH_1,
    THROTTLE_CH_2,
    THROTTLE_CH_3,
    THROTTLE_CH_CNT
} eThrottleChannel;


typedef struct
{
    Pwm             *pxPwm[PWM_CHANNEL_COUNT];
    PWM_Channel      eCh[PWM_CHANNEL_COUNT];
} xChannelMap;

typedef struct
{
    uint16_t usX;
    uint16_t usY;
} xAxisStruct;

static xChannelMap xChMap =
{
    { PWM0,         PWM0,         PWM0,         PWM1         },
    { PWM_CHANNEL0, PWM_CHANNEL1, PWM_CHANNEL3, PWM_CHANNEL0 }
};


extern OS_QUEUE          throttleQ;


static void     vThrottle(xAxisStruct xAxis);
static uint32_t ulScale(uint16_t usAxis);
static void     vEnableThrottle(const eThrottleChannel eCh);
static void     vDisableThrottle(const eThrottleChannel eCh);
static bool     bCheckDeadZone(const uint16_t usAxis);
static uint32_t ulSetSteer(const uint32_t ulThrottle,
                           const uint16_t usSteer);
static void     vSetThrottle(const eThrottleChannel eCh,
                             const uint32_t ulThrottle);
static void     vUpdateThrottle(const uint16_t usJoyPos,
                                const uint16_t usRightThrottle,
                                const uint16_t usLeftThrottle);

/**
 * @brief   Task responsible for controlling the DC motors.
 *
 * @param   pvArg   Pointer to the task parameter.
 *
 * @return  None.
 */
void throttle_vTask(void *pvArg)
{
    uint32_t      ret;
    char          param[sizeof(xAxisStruct)];
    xAxisStruct   xAxis;
    (void)pvArg;

    while (1)
    {
        ret = OS_QUEUE_GetPtrTimed(&throttleQ, (void **)&param, 10);
        if (ret > 0)
        {
            xAxis.usX = (param[1] << 8) | param[0];
            xAxis.usY = (param[3] << 8) | param[2];

            vThrottle(xAxis);
        }
        else
        {
            for (eThrottleChannel eCh = THROTTLE_CH_0; eCh < THROTTLE_CH_CNT; eCh++)
            {
                vDisableThrottle(eCh);
            }
            Journal_vWriteError(THROTTLE_TIMEOUT);
        }
    }
}


/**
 * @brief   Writer analog axis values to digital PWM
 *          channels to control the DC motors.
 *
 * @param   xAxis   Axis values.
 *
 * @return  None.
 */
static void vThrottle(xAxisStruct xAxis)
{
    uint32_t usLeftThrottle;
    uint32_t usRightThrottle;

    if (bCheckDeadZone(xAxis.usX) == false)
    {
        /* Need to scale duty cycle value (Datasheet Chapter 51.7.41) */
        usLeftThrottle  = (ulScale(xAxis.usX) * DUTY_SCALE) >> 14;
        usRightThrottle = usLeftThrottle;

        /* Calculate steering if joystick moved */
        if (bCheckDeadZone(xAxis.usY) == false)
        {
            /* Positive Y axis means the joystick is turned left.
             * We have to reduce the left motor throttle to steer left.
             * and reduce the right side motor throttle to steer right.
             */
            if (xAxis.usY > THROTTLE_RESOLUTION)
            {
                usLeftThrottle = ulSetSteer(usLeftThrottle, xAxis.usY);
            }
            else
            {
                usRightThrottle = ulSetSteer(usRightThrottle, xAxis.usY);
            }
        }

        vUpdateThrottle(xAxis.usX, usRightThrottle, usLeftThrottle);
    }
    else
    {
        for (eThrottleChannel eCh = THROTTLE_CH_0; eCh < THROTTLE_CH_CNT; eCh++)
        {
            vDisableThrottle(eCh);
        }
    }
}


/**
 * @brief   Calculate steer modifier for a axis.
 *
 * @param   ulThrottle  Throttle value to modify.
 *
 * @param   usSteer     Steer modifier.
 *
 * @param   Steer modified throttle value.
 */
static uint32_t ulSetSteer(const uint32_t ulThrottle, const uint16_t usSteer)
{
    uint32_t fSteerPer = (ulScale(usSteer) << 14) / UINT16_MAX;
    
    return (uint32_t)(ulThrottle * fSteerPer) >> 14;
}

/**
 * @brief   Downsample axis
 *
 *          Axis value is halved to separate the direction
 *          effectively downsampling the signal. The lower
 *          half is then mirrored so the results correspond
 *          to the joystick value. Finally the signal needs
 *          to be oversampled so we can fit it to a digital
 *          channel later. See below:
 *
 *                          |
 *      What we get:        |   What we want:
 *                          |
 *            y=2^16-1      |               y=2^8-1
 *          A               |             A
 *          |               |             |
 *  x=0     |    x=2^16-1   |   x=2^8-1   |      x=2^8-1
 *     <----+---->          |        <----+---->
 *          |               |             |\
 *          |               |             | x,y=0
 *          v               |             v
 *            y=0           |               y=2^8-1
 *                          |
 *
 * @param   usAxis  Axis to scale.
 *
 * @return  Scaled axis value.
 */
static uint32_t ulScale(uint16_t usAxis)
{    
    if (usAxis < THROTTLE_RESOLUTION)
    {
        usAxis = ~usAxis;
    }

    return (uint32_t)((usAxis - THROTTLE_RESOLUTION) << 1);
}


/**
 * @brief   Set the channel throttle value.
 *
 * @param   eCh         Target channel.
 *
 * @param   ulThrottle  Throttle value to set.
 *
 * @return  None.
 */
static void vSetThrottle(const eThrottleChannel eCh, const uint32_t ulThrottle)
{
    assert(eCh < THROTTLE_CH_CNT);
    assert(ulThrottle  <= DUTY_CYCLE_MAX);
    PWM_UpdateDutyCycle(xChMap.pxPwm[eCh], xChMap.eCh[eCh], ulThrottle);
}


/**
 * @brief   Enable channel throttle.
 *
 * @param   eCh Channel to enable.
 *
 * @return None.
 */
static void vEnableThrottle(const eThrottleChannel eCh)
{
    assert(eCh < THROTTLE_CH_CNT);
    PWM_Enable(xChMap.pxPwm[eCh], xChMap.eCh[eCh]);
}


/**
 * @brief   Disable channel throttle.
 *
 * @param   eCh Channel to disable.
 *
 * @return  None.
 */
static void vDisableThrottle(const eThrottleChannel eCh)
{
    assert(eCh < THROTTLE_CH_CNT);
    PWM_Disable(xChMap.pxPwm[eCh], xChMap.eCh[eCh]);
}


/**
 * @brief   Update motor throttle.
 *
 * @param   usJoyPos        Joystick position.
 *
 * @param   usRightThrottle Right side channel throttle value.
 *
 * @param   usLeftThrottle  Left side channel throttle value.
 *
 * @return  None.
 */
static void vUpdateThrottle(const uint16_t usJoyPos,
                            const uint16_t usRightThrottle,
                            const uint16_t usLeftThrottle)
{
    /* Go forward if MSB set (joystick over center) else backward. */
    if (usJoyPos > THROTTLE_RESOLUTION)
    {
        vDisableThrottle(THROTTLE_CH_2);
        vDisableThrottle(THROTTLE_CH_3);
        vSetThrottle(THROTTLE_CH_0, usRightThrottle);
        vSetThrottle(THROTTLE_CH_1, usLeftThrottle);
        vEnableThrottle(THROTTLE_CH_0);
        vEnableThrottle(THROTTLE_CH_1);
    }
    else
    {
        vDisableThrottle(THROTTLE_CH_0);
        vDisableThrottle(THROTTLE_CH_1);
        vSetThrottle(THROTTLE_CH_2, usRightThrottle);
        vSetThrottle(THROTTLE_CH_3, usLeftThrottle);
        vEnableThrottle(THROTTLE_CH_2);
        vEnableThrottle(THROTTLE_CH_3);
    }
}


/**
 * @brief   Check if joystick in the dead zone aka
 *          it is not actively controlled.
 *
 * @param   usAxis    Axis to check.
 *
 * @return  bDeadZone Is axis in the dead zone?
 */
static bool bCheckDeadZone(const uint16_t usAxis)
{
    bool bDeadZone = false;
    if ((usAxis < DEADZONE_MAX) && (usAxis > DEADZONE_MIN))
    {
        bDeadZone = true;
    }
    return bDeadZone;
}
