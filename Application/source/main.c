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


static OS_STACKPTR int stackErr[512]        __attribute__((aligned(8)));
       OS_TASK         errTCB;
static OS_STACKPTR int stackRtc[512]        __attribute__((aligned(8)));
       OS_TASK         rtcTCB;
static OS_STACKPTR int stackWireless[2048]  __attribute__((aligned(8)));
static OS_TASK         wirelessTCB;

/* TODO: Temporarily increased sensor task stack size !!! */
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
static OS_STACKPTR int stackDistance[128]   __attribute__((aligned(8)));
       OS_TASK         distanceTCB;


OS_QUEUE          throttleQ;
OS_QUEUE          gyroQ;
OS_QUEUE          tsQ;
OS_EVENT          dmaEvt;
OS_EVENT          svEvt;
OS_EVENT          wlessEvt;
OS_MUTEX          wlessMutex;

#define Q_MSG_SIZE  (1u)
#define Q_MSG_CNT   (32u)
#define Q_SIZE      (Q_MSG_CNT * (Q_MSG_SIZE + OS_Q_SIZEOF_HEADER) + MESSAGE_ALIGNMENT)
char              _tsMemBuffer[Q_SIZE];
char              _gyroMemBuffer[Q_SIZE];
char              _throttleMemBuffer[Q_SIZE];


extern void Wireless_Task(void *arg);
extern void Sensor_Task(void *arg);
extern void Control_Task(void *arg);
extern void Video_Task(void *arg);
extern void err_task(void *arg);
extern void RTC_vTask(void *arg);
extern void throttle_vTask(void *arg);
extern void gyro_vTask(void *arg);
extern void Distance_Task(void *arg);
static void StartupTask(void *arg);

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
    OS_TASK_CREATEEX(&startupTCB, "Startup", TASK_STARTUP_PRIORITY, StartupTask, stackStartup, NULL);
    OS_TASK_CREATEEX(&wirelessTCB, "Wireless", TASK_WIRELESS_PRIORITY, Wireless_Task, stackWireless, NULL);
    OS_TASK_CREATEEX(&sensorTCB, "Sensor", TASK_SENSOR_PRIORITY, Sensor_Task, stackSensor, NULL);
    //OS_TASK_CREATEEX(&controlTCB, "Control", TASK_CONTROL_PRIORITY, Control_Task, stackControl, NULL);
    OS_TASK_CREATEEX(&videoTCB, "Video", TASK_VIDEO_PRIORITY, Video_Task, stackVideo, NULL);
    //OS_TASK_CREATEEX(&rtcTCB, "RTC", TASK_RTC_PRIORITY, RTC_vTask, stackRtc, NULL);
    OS_TASK_CREATEEX(&errTCB, "Error",  TASK_ERROR_PRIORITY, err_task, stackErr, NULL);
    //OS_TASK_CREATEEX(&gyroTCB, "Gyro", TASK_GYRO_PRIORITY, gyro_vTask, stackGyro, NULL);
    //OS_TASK_CREATEEX(&throttleTCB, "Throttle", TASK_THROTTLE_PRIORITY, throttle_vTask, stackThrottle, NULL);
    OS_TASK_CREATEEX(&distanceTCB, "Distance", TASK_DISTANCE_PRIORITY, Distance_Task, stackDistance, NULL);
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
    OS_EVENT_CreateEx(&dmaEvt, OS_EVENT_MASK_MODE_AND_LOGIC);
    OS_EVENT_CreateEx(&svEvt, OS_EVENT_MASK_MODE_OR_LOGIC);
    OS_EVENT_CreateEx(&wlessEvt, OS_EVENT_MASK_MODE_OR_LOGIC);

    OS_QUEUE_Create(&throttleQ, &_throttleMemBuffer, sizeof(_throttleMemBuffer));
    
    OS_QUEUE_Create(&gyroQ, &_gyroMemBuffer, sizeof(_gyroMemBuffer));
    OS_QUEUE_Create(&tsQ, &_tsMemBuffer, sizeof(_tsMemBuffer));

    OS_MUTEX_Create(&wlessMutex);
}


static void StartupTask(void *arg)
{
    (void)arg;
    OS_InitServices();

    BSP_Init();

    OS_TASK_Terminate(NULL);
}
