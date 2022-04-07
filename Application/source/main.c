#include "same70.h"
#include <string.h>

/* Application includes */
#include "system.h"
#include "logWriter.h"
#include "throttle.h"
#include "gyro.h"
#include "wireless.h"

/* RTOS includes */
#include "RTOS.h"
#include "BSP.h"

/* Driver includes */
#include "twi.h"


#define TASK_STARTUP_PRIORITY                   (60u)
#define TASK_WIRELESS_PRIORITY                  (55u)
#define TASK_JOURNAL_PRIORITY                   (57u)
#define TASK_RTC_PRIORITY                       (58u)
#define TASK_THROTTLE_PRIORITY                  (59u)
#define TASK_GYRO_PRIORITY                      (56u)


static OS_STACKPTR int stackJournal[512]    __attribute__((aligned(8)));
       OS_TASK         journalTCB;
static OS_STACKPTR int stackRtc[512]        __attribute__((aligned(8)));
       OS_TASK         rtcTCB;
static OS_STACKPTR int stackWireless[2048]  __attribute__((aligned(8)));
static OS_TASK         wirelessTCB;
static OS_STACKPTR int stackThrottle[512]   __attribute__((aligned(8)));
static OS_TASK         throttleTCB;
static OS_STACKPTR int stackGyro[512]       __attribute__((aligned(8)));
static OS_TASK         gyroTCB;

OS_QUEUE          throttleQ;
OS_QUEUE          gyroQ;
OS_QUEUE          tsQ;
OS_MAILBOX        twiMbox;
OS_SEMAPHORE      twiSema;
OS_EVENT          dmaEvt;
OS_EVENT          svEvt;

char              _twiMemBuffer[sizeof(TWI_Msg *)];
char              _tsMemBuffer[32 + OS_Q_SIZEOF_HEADER];
char              _gyroMemBuffer[32 + OS_Q_SIZEOF_HEADER];
char              _throttleMemBuffer[32 + OS_Q_SIZEOF_HEADER];


extern void Wireless_Task(void *pvArg);
extern void Journal_vErrorTask(void *pvArg);
extern void RTC_vTask(void *pvArg);
extern void throttle_vTask(void *pvArg);
extern void gyro_vTask(void *pvArg);

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
    SystemCoreClockUpdate();
    
    OS_Init();
    OS_InitServices();
    OS_InitHW();
    OS_InitTasks();

    BSP_Init();

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
    OS_TASK_CREATEEX(&wirelessTCB, "Wireless", TASK_WIRELESS_PRIORITY, Wireless_Task, stackWireless, NULL);
    OS_TASK_CREATEEX(&rtcTCB, "RTC", TASK_RTC_PRIORITY, RTC_vTask, stackRtc, NULL);
    OS_TASK_CREATEEX(&journalTCB, "Journal",  TASK_JOURNAL_PRIORITY, Journal_vErrorTask, stackJournal, NULL);
    OS_TASK_CREATEEX(&gyroTCB, "Gyro", TASK_GYRO_PRIORITY, gyro_vTask, stackGyro, NULL);
    OS_TASK_CREATEEX(&throttleTCB, "Throttle", TASK_THROTTLE_PRIORITY, throttle_vTask, stackThrottle, NULL);
}


/**
 * @brief   Create RTOS events, queues, mutexes, sempahores, etc.
 *
 * @param   None
 *
 * @retval  None
 */
static void OS_InitServices(void)
{
    OS_EVENT_Create(&dmaEvt);
    OS_EVENT_Create(&svEvt);

    OS_QUEUE_Create(&throttleQ, &_throttleMemBuffer, sizeof(_throttleMemBuffer));
    OS_QUEUE_Create(&gyroQ, &_gyroMemBuffer, sizeof(_gyroMemBuffer));
    OS_QUEUE_Create(&tsQ, &_tsMemBuffer, sizeof(_tsMemBuffer));

    OS_MAILBOX_Create(&twiMbox, sizeof(TWI_Msg *), 1, &_twiMemBuffer);

    OS_SEMAPHORE_Create(&twiSema, 0);
}
