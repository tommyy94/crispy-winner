#include "srf05.h"
#include "io.h"
#include "RTOS.h"
#include "logWriter.h"
#include "tc_driver.h"
#include "distance.h"


#define SRF05_PORT                (PIOD)
#define SRF05_ECHO_PIN            (15u)
#define SRF05_TRIGGER_PIN         (16u)
#define SRF05_PULSE_US            (10u)
#define SRF05_TIMEOUT_MS          (30u)

#define SOUND_VELOCITY_MS   (343u)
#define SRF05_FORMULA(time) ((time) * SOUND_VELOCITY_MS / 100 / 2)

/*
 * d = t * 343 / 100 / 2
 *   = (t * (0.02 << 11) >> 11)
 *   = (t * 40.96 >> 11)
 *   = (t * 41 >> 11)
 */
//#define SRF05_FORMULA(time) (((time) * 41) >> 11)



extern OS_TASK    distanceTCB;


static void SRF05_PulseOutput(void);
static void SRF05_EchoHandler(void);
static void SRF05_PulseHandler(void);


/**
 * @brief   Initialize IO for SRF05.
 *
 * @param   None.
 *
 * @retval  None.
 */
void SRF05_Init(void)
{
    IO_ConfigureOutput(SRF05_PORT, IO_MASK(SRF05_TRIGGER_PIN), IO_MASK(SRF05_TRIGGER_PIN));
    IO_ClearOutput(SRF05_PORT, IO_MASK(SRF05_TRIGGER_PIN));

    (void)IO_ConfigureIRQ(SRF05_PORT, IO_SENSE_RISE_FALL, IO_MASK(SRF05_ECHO_PIN));
    IO_InstallIrqHandler(SRF05_ECHO_PIN, SRF05_EchoHandler);
    IO_ConfigurePull(SRF05_PORT, IO_MASK(SRF05_ECHO_PIN), IO_PULLDOWN);
}


/**
 * @brief   Measure distance.
 *
 * @param   pDistanceCm   Pointer where distance in CM is stored.
 *
 * @retval  status        false if timeout else true.
 */
bool SRF05_MeasureDistance(uint32_t *pDistanceCm)
{
    uint32_t  time;
    uint32_t  mask    = 0;
    bool      status  = false;

    SRF05_PulseOutput();

    if (OS_TASKEVENT_GetSingleTimed(mask, SRF05_TIMEOUT_MS) == 0)
    {
        time = TC_ReadCounter(TC0, TC_CHANNEL_0);

        /* Avoid divide by zero */
        if (time > 0)
        {
            *pDistanceCm = SRF05_FORMULA(time);
            status = true;
        }
    }

    return status;
}


/**
 * @brief   Stop SRF05 measurement.
 *
 * @param   None.
 *
 * @retval  None.
 */
void SRF05_StartMeasuring(void)
{
    TC_Start(TC0, TC_CHANNEL_0);
}


/**
 * @brief   Start SRF05 measurement.
 *
 * @param   None.
 *
 * @retval  None.
 */
void SRF05_StopMeasuring(void)
{
    TC_Stop(TC0, TC_CHANNEL_0);
}


/**
 * @brief   Pulse trigger pin HIGH for atleast 10us.
 *
 * @param   pDistanceCm   Pointer where distance in CM is stored.
 *
 * @retval  status        false if timeout else true.
 */
static void SRF05_PulseOutput(void)
{
    IO_SetOutput(SRF05_PORT, IO_MASK(SRF05_TRIGGER_PIN));

    TC_Delay(TC0, TC_CHANNEL_0, SRF05_PULSE_US);

    IO_ClearOutput(SRF05_PORT, IO_MASK(SRF05_TRIGGER_PIN));
}


/**
 * @brief   SRF05 output end handler.
 *
 * @param   None.
 *
 * @retval  None.
 */
static void SRF05_EchoHandler(void)
{
    IO_PinLevel_t   lvl;

    lvl = IO_GetPinLevel(SRF05_PORT, SRF05_ECHO_PIN);
    if (lvl == IO_PIN_HIGH)
    {
        SRF05_StartMeasuring();
    }
    else /* if (lvl == IO_PIN_LOW) */
    {
        SRF05_StopMeasuring();

        /* Signal task */        
        OS_TASKEVENT_Set(&distanceTCB, SRF05_EVT_MEAS_FINISHED);
    }
}


/**
 * @brief   SRF05 10us pulse end handler.
 *
 * @param   None.
 *
 * @retval  None.
 */
static void SRF05_PulseHandler(void)
{
    OS_TASKEVENT_Set(&distanceTCB, SRF05_EVT_PULSE);
}
