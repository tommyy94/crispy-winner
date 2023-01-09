/* System includes */
#include <stdlib.h>
#include <stdbool.h>

/* User includes */
#include "throttle.h"
#include "err.h"

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
} ThrottleChannel_t;


typedef struct
{
    uint16_t x;
    uint16_t y;
} AxisStruct_t;


extern OS_QUEUE          throttleQ;


static void     Throttle(AxisStruct_t axis);
static uint32_t Scale(uint16_t axis);
static void     EnableThrottle(const ThrottleChannel_t ch);
static void     DisableThrottle(const ThrottleChannel_t ch);
static bool     CheckDeadZone(const uint16_t axis);
static uint32_t SetSteer(const uint32_t throttle,
                         const uint16_t steer);
static void     SetThrottle(const ThrottleChannel_t ch,
                             const uint32_t throttle);
static void     UpdateThrottle(const uint16_t joyPos,
                               const uint16_t rightThrottle,
                               const uint16_t leftThrottle);

/**
 * @brief   Task responsible for controlling the DC motors.
 *
 * @param   pArg   Pointer to the task parameter.
 *
 * @return  None.
 */
void throttle_Task(void *pArg)
{
    uint32_t      ret;
    char          param[sizeof(AxisStruct_t)];
    AxisStruct_t  axis;
    (void)pArg;

    while (1)
    {
        ret = OS_QUEUE_GetPtrTimed(&throttleQ, (void **)&param, 10);
        if (ret > 0)
        {
            axis.x = (param[1] << 8) | param[0];
            axis.y = (param[3] << 8) | param[2];

            Throttle(axis);

            OS_QUEUE_Purge(&throttleQ);
        }
        else
        {
            for (ThrottleChannel_t ch = THROTTLE_CH_0; ch < THROTTLE_CH_CNT; ch++)
            {
                DisableThrottle(ch);
            }
            //err_report(THROTTLE_TIMEOUT);
        }
    }
}


/**
 * @brief   Writer analog axis values to digital PWM
 *          channels to control the DC motors.
 *
 * @param   axis   Axis values.
 *
 * @return  None.
 */
static void Throttle(AxisStruct_t axis)
{
    uint32_t leftThrottle;
    uint32_t rightThrottle;

    if (CheckDeadZone(axis.x) == false)
    {
        /* Need to scale duty cycle value (Datasheet Chapter 51.7.41) */
        leftThrottle  = (Scale(axis.x) * DUTY_SCALE) >> 14;
        rightThrottle = leftThrottle;

        /* Calculate steering if joystick moved */
        if (CheckDeadZone(axis.y) == false)
        {
            /* Positive Y axis means the joystick is turned left.
             * We have to reduce the left motor throttle to steer left.
             * and reduce the right side motor throttle to steer right.
             */
            if (axis.y > THROTTLE_RESOLUTION)
            {
                leftThrottle = SetSteer(leftThrottle, axis.y);
            }
            else
            {
                rightThrottle = SetSteer(rightThrottle, axis.y);
            }
        }

        UpdateThrottle(axis.x, rightThrottle, leftThrottle);
    }
    else
    {
        for (ThrottleChannel_t ch = THROTTLE_CH_0; ch < THROTTLE_CH_CNT; ch++)
        {
            DisableThrottle(ch);
        }
    }
}


/**
 * @brief   Calculate steer modifier for a axis.
 *
 * @param   throttle  Throttle value to modify.
 *
 * @param   steer     Steer modifier.
 *
 * @param   Steer modified throttle value.
 */
static uint32_t SetSteer(const uint32_t throttle, const uint16_t steer)
{
    uint32_t steerPer = (Scale(steer) << 14) / UINT16_MAX;
    
    return (uint32_t)(throttle * steerPer) >> 14;
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
 * @param   axis  Axis to scale.
 *
 * @return  Scaled axis value.
 */
static uint32_t Scale(uint16_t axis)
{    
    if (axis < THROTTLE_RESOLUTION)
    {
        axis = ~axis;
    }

    return (uint32_t)((axis - THROTTLE_RESOLUTION) << 1);
}


/**
 * @brief   Set the channel throttle value.
 *
 * @param   ch         Target channel.
 *
 * @param   throttle  Throttle value to set.
 *
 * @return  None.
 */
static void SetThrottle(const ThrottleChannel_t ch, const uint32_t throttle)
{
    assert(ch < THROTTLE_CH_CNT);
    assert(throttle  <= DUTY_CYCLE_MAX);

    //ESC_Write(chMap.pPwm[ch], chMap.ch[ch], throttle);
}


/**
 * @brief   Enable channel throttle.
 *
 * @param   ch Channel to enable.
 *
 * @return None.
 */
static void EnableThrottle(const ThrottleChannel_t ch)
{
    assert(ch < THROTTLE_CH_CNT);
    //ESC_Enable(chMap.pPwm[ch], chMap.ch[ch]);
}


/**
 * @brief   Disable channel throttle.
 *
 * @param   ch Channel to disable.
 *
 * @return  None.
 */
static void DisableThrottle(const ThrottleChannel_t ch)
{
    assert(ch < THROTTLE_CH_CNT);
    //ESC_Disable(chMap.pPwm[ch], chMap.ch[ch]);
}


/**
 * @brief   Update motor throttle.
 *
 * @param   joyPos        Joystick position.
 *
 * @param   rightThrottle Right side channel throttle value.
 *
 * @param   leftThrottle  Left side channel throttle value.
 *
 * @return  None.
 */
static void UpdateThrottle(const uint16_t joyPos,
                           const uint16_t rightThrottle,
                           const uint16_t leftThrottle)
{
    /* Go forward if MSB set (joystick over center) else backward. */
    if (joyPos > THROTTLE_RESOLUTION)
    {
        DisableThrottle(THROTTLE_CH_2);
        DisableThrottle(THROTTLE_CH_3);
        SetThrottle(THROTTLE_CH_0, rightThrottle);
        SetThrottle(THROTTLE_CH_1, leftThrottle);
        EnableThrottle(THROTTLE_CH_0);
        EnableThrottle(THROTTLE_CH_1);
    }
    else
    {
        DisableThrottle(THROTTLE_CH_0);
        DisableThrottle(THROTTLE_CH_1);
        SetThrottle(THROTTLE_CH_2, rightThrottle);
        SetThrottle(THROTTLE_CH_3, leftThrottle);
        EnableThrottle(THROTTLE_CH_2);
        EnableThrottle(THROTTLE_CH_3);
    }
}


/**
 * @brief   Check if joystick in the dead zone aka
 *          it is not actively controlled.
 *
 * @param   axis    Axis to check.
 *
 * @return  deadZone Is axis in the dead zone?
 */
static bool CheckDeadZone(const uint16_t axis)
{
    bool deadZone = false;
    if ((axis < DEADZONE_MAX) && (axis > DEADZONE_MIN))
    {
        deadZone = true;
    }
    return deadZone;
}
