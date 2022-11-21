#include "srf05.h"
#include "io.h"
#include "RTOS.h"
#include "logWriter.h"


#define SRF05_PORT          (PIOD)
#define SRF05_ECHO_PIN      (18u)
#define SRF05_TRIGGER_PIN   (19u)
#define SRF05_TIMEOUT_MS    (30u)

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


static void SRF05_Handler(void);
static void SRF05_PulseOutput(void);


/**
 * @brief   Initialize IO for SRF05.
 *
 * @param   None.
 *
 * @retval  None.
 */
void SRF05_Init(void)
{
    IO_ConfigureOutput(SRF05_PORT, 1 << SRF05_TRIGGER_PIN, 1 << SRF05_TRIGGER_PIN);

    (void)IO_ConfigureIRQ(SRF05_PORT, IO_SENSE_RISE_FALL, 1 << SRF05_TRIGGER_PIN);
    IO_InstallIrqHandler(SRF05_ECHO_PIN, SRF05_Handler);
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
    bool      status  = false;

    SRF05_PulseOutput();

    if (OS_TASKEVENT_GetSingleTimed(mask, SRF05_TIMEOUT_MS) == 0)
    {
        /* Avoid divide by zero */
        if (time > 0)
        {
            *pDistanceCm = SRF05_FORMULA(time);
        }
        status = true;
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
    IO_SetOutput(SRF05_PORT, SRF05_TRIGGER_PIN);

    /* 10us delay here */

    IO_ClearOutput(SRF05_PORT, SRF05_TRIGGER_PIN);
}


/**
 * @brief   SRF05 output end handler.
 *
 * @param   None.
 *
 * @retval  None.
 */
static void SRF05_Handler(void)
{
    IO_IrqSource_t  src;
    uint32_t        time;
    bool            status;

    src = IO_GetIrqSource(SRF05_PORT, 1 << SRF05_TRIGGER_PIN);
    if (src == IO_IRQ_SOURCE_RISE_HIGH)
    {
        /* Start measuring */

        /* Start timer */
        OS_TASKEVENT_Set(&distanceTCB, SRF05_EVT_MEAS_FINISHED);
    }
    else /* if (src == IO_IRQ_SOURCE_FALL_LOW) */
    {
        /* Stop measuring */


        /* Report the measurement */
        time = 0;
        status = OS_MAILBOX_Put(&distanceMbox, &time);
        assert(status == true);
    }
}
