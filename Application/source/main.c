#include "same70.h"
#include <string.h>
#include "socket/include/socket.h"

/* Application includes */
#include "system.h"
#include "err.h"
#include "throttle.h"
#include "gyro.h"
#include "wireless.h"
#include "distance.h"

/* RTOS includes */
#include "RTOS.h"
#include "BSP.h"

/* Driver includes */
#include "twi.h"

#define MESSAGE_ALIGNMENT                       (4u)

/* High number = high priority */
#define TASK_STARTUP_PRIORITY                   (70u)
#define TASK_WIRELESS_PRIORITY                  (69u)
#define TASK_SENSOR_PRIORITY                    (62u)
#define TASK_VIDEO_PRIORITY                     (63u)
#define TASK_CONTROL_PRIORITY                   (61u)
#define TASK_ERROR_PRIORITY                     (57u)
#define TASK_RTC_PRIORITY                       (58u)
#define TASK_THROTTLE_PRIORITY                  (59u)
#define TASK_GYRO_PRIORITY                      (56u)
#define TASK_DISTANCE_PRIORITY                  (60u)
#define TASK_IMAGE_PRIORITY                     (68u)


static OS_STACKPTR int stackErr[128]        __attribute__((aligned(8)));
       OS_TASK         errTCB;
static OS_STACKPTR int stackRtc[128]        __attribute__((aligned(8)));
       OS_TASK         rtcTCB;
static OS_STACKPTR int stackWireless[2048]  __attribute__((aligned(8)));
static OS_TASK         wirelessTCB;
static OS_STACKPTR int stackSensor[1024]     __attribute__((aligned(8)));
static OS_TASK         sensorTCB;
static OS_STACKPTR int stackControl[512]    __attribute__((aligned(8)));
static OS_TASK         controlTCB;
static OS_STACKPTR int stackVideo[1024]      __attribute__((aligned(8)));
static OS_TASK         videoTCB;
static OS_STACKPTR int stackThrottle[512]   __attribute__((aligned(8)));
static OS_TASK         throttleTCB;
static OS_STACKPTR int stackGyro[512]       __attribute__((aligned(8)));
static OS_TASK         gyroTCB;
static OS_STACKPTR int stackStartup[512]    __attribute__((aligned(8)));
static OS_TASK         startupTCB;
static OS_STACKPTR int stackDistance[256]   __attribute__((aligned(8)));
       OS_TASK         distanceTCB;
static OS_STACKPTR int stackImage[2048]     __attribute__((aligned(8)));
       OS_TASK         imageTCB;


OS_QUEUE          throttleQ;
OS_QUEUE          gyroQ;
OS_QUEUE          tsQ;
OS_EVENT          dmaEvt;
OS_EVENT          svEvt;
OS_EVENT          wlessEvt;
OS_MAILBOX        twiMbox;
OS_SEMAPHORE      twiSema;
OS_MUTEX          wlessMutex;
OS_MUTEX          twiMutex;
#ifdef EVABOARD_WORKAROUND
OS_MUTEX          evabrdWaMutex;
#endif /* EVABOARD_WORKAROUND */

#define Q_MSG_SIZE  (1u)
#define Q_MSG_CNT   (32u)
#define Q_SIZE      (Q_MSG_CNT * (Q_MSG_SIZE + OS_Q_SIZEOF_HEADER) + MESSAGE_ALIGNMENT)
char              _twiMemBuffer[sizeof(TWI_Msg *)];
char              _tsMemBuffer[Q_SIZE];
char              _gyroMemBuffer[Q_SIZE];
char              _throttleMemBuffer[Q_SIZE];


extern void Wireless_Task(void *arg);
extern void Sensor_Task(void *arg);
extern void Control_Task(void *arg);
extern void Video_Task(void *arg);
extern void err_Task(void *arg);
extern void RTC_Task(void *arg);
extern void throttle_Task(void *arg);
extern void gyro_Task(void *arg);
extern void Distance_Task(void *arg);
extern void Image_Task(void *arg);
static void Startup_Task(void *arg);

static void OS_InitTasks(void);
static void OS_InitServices(void);


/**
 * @brief   Initialize RTOS and start the scheduler.
 *
 * @param   None
 *
 * @retval  None
 */
int main(void)
{
    OS_Init();
    OS_InitHW();
    OS_InitTasks();
    OS_Start();
}


/**
 * @brief   Startup task.
 *
 * @param   None
 *
 * @retval  None
 */
static void OS_InitTasks(void)
{
    OS_TASK_CREATEEX(&startupTCB,   "Startup",  TASK_STARTUP_PRIORITY,  Startup_Task,   stackStartup,   NULL);
    OS_TASK_CREATEEX(&wirelessTCB,  "Wireless", TASK_WIRELESS_PRIORITY, Wireless_Task,  stackWireless,  NULL);
    OS_TASK_CREATEEX(&sensorTCB,    "Sensor",   TASK_SENSOR_PRIORITY,   Sensor_Task,    stackSensor,    NULL);
    //OS_TASK_CREATEEX(&controlTCB,   "Control",  TASK_CONTROL_PRIORITY,  Control_Task,   stackControl,   NULL);
    OS_TASK_CREATEEX(&videoTCB,     "Video",    TASK_VIDEO_PRIORITY,    Video_Task,     stackVideo,     NULL);
    OS_TASK_CREATEEX(&rtcTCB,       "RTC",      TASK_RTC_PRIORITY,      RTC_Task,       stackRtc,       NULL);
    OS_TASK_CREATEEX(&errTCB,       "Error",    TASK_ERROR_PRIORITY,    err_Task,       stackErr,       NULL);
    OS_TASK_CREATEEX(&gyroTCB,      "Gyro",     TASK_GYRO_PRIORITY,     gyro_Task,      stackGyro,      NULL);
    OS_TASK_CREATEEX(&throttleTCB,  "Throttle", TASK_THROTTLE_PRIORITY, throttle_Task,  stackThrottle,  NULL);
    OS_TASK_CREATEEX(&distanceTCB,  "Distance", TASK_DISTANCE_PRIORITY, Distance_Task,  stackDistance,  NULL);
    OS_TASK_CREATEEX(&imageTCB,     "Image",    TASK_IMAGE_PRIORITY,    Image_Task,     stackImage,     NULL);
}


/**
 * @brief   Create RTOS events, queues, mutexes, semaphores, etc.
 *
 * @param   None
 *
 * @retval  None
 */
static void OS_InitServices(void)
{
    /* Events */
    OS_EVENT_CreateEx(&dmaEvt, OS_EVENT_MASK_MODE_AND_LOGIC);
    OS_EVENT_CreateEx(&svEvt, OS_EVENT_MASK_MODE_OR_LOGIC);
    OS_EVENT_CreateEx(&wlessEvt, OS_EVENT_MASK_MODE_OR_LOGIC);

    /* Queues */
    OS_QUEUE_Create(&throttleQ, &_throttleMemBuffer, sizeof(_throttleMemBuffer));
    OS_QUEUE_Create(&gyroQ, &_gyroMemBuffer, sizeof(_gyroMemBuffer));
    OS_QUEUE_Create(&tsQ, &_tsMemBuffer, sizeof(_tsMemBuffer));

    /* Mailboxes */
    OS_MAILBOX_Create(&twiMbox, sizeof(TWI_Msg *), 1, &_twiMemBuffer);

    /* Semaphores */
    OS_SEMAPHORE_Create(&twiSema, 0);

    /* Mutexes */
    OS_MUTEX_Create(&wlessMutex);
    OS_MUTEX_Create(&twiMutex);

    /* SPI0 and ISI share pins in SAME70 Xplained evaluation kit:
     * - PD20: SPI0_MISO only
     * - PD21: SPI0_MOSI / ISI_D1
     * - PD22: SPI0_SCK / ISI_D0
     * - PD25: SPI0_SS / ISI_VSYNC
     *
     * Therefore the pins have to be reconfigured
     * before operating with SPI0/ISI peripheral.
     */
#ifdef EVABOARD_WORKAROUND
    OS_MUTEX_Create(&evabrdWaMutex);
#endif /* EVABOARD_WORKAROUND */
}


static void Startup_Task(void *arg)
{
    (void)arg;
    OS_InitServices();

    BSP_Init();

    OS_TASK_Terminate(NULL);
}
