/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2019  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : RTOSInit_SAME70.c for Microchip SAMV70
Purpose : Initializes and handles the hardware for embOS as far
          as required by embOS
          Feel free to modify this file acc. to your target system.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"
#include "SEGGER_SYSVIEW.h"
#include "same70.h"     /* Device specific header file, contains CMSIS */
#include "SEGGER_RTT.h"

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

/*********************************************************************
*
*       Clock frequency settings (configuration)
*/
#ifndef   OS_FSYS                   /* CPU Main clock frequency     */
  #define OS_FSYS SystemCoreClock
#endif

#ifndef   OS_PCLK_TIMER             /* Peripheral clock for timer   */
  #define OS_PCLK_TIMER (OS_FSYS)   /* May vary from CPU clock      */
#endif                              /* depending on CPU             */

#ifndef   OS_PCLK_UART              /* Peripheral clock for UART    */
  #define OS_PCLK_UART  (OS_FSYS)   /* May vary from CPU clock      */
#endif                              /* depending on CPU             */

#ifndef   OS_TICK_FREQ
  #define OS_TICK_FREQ  (1000u)
#endif

#define OS_TIMER_RELOAD (OS_PCLK_TIMER / OS_TICK_FREQ)

/*********************************************************************
*
*       Configuration of communication to embOSView
*/
#ifndef   OS_VIEW_IFSELECT
  #define OS_VIEW_IFSELECT  OS_VIEW_IF_JLINK
#endif

/****** End of configurable options *********************************/

/*********************************************************************
*
*       Vector table
*/
#if (defined __SES_ARM)           /* SEGGER Embedded Studio */
  extern int _vectors;
  #define __Vectors    _vectors
#elif (defined __CROSSWORKS_ARM)  /* Rowley CrossStudio */
  extern int _vectors;
  #define __Vectors    _vectors
#elif (defined __ICCARM__)        /* IAR */
  #define __Vectors    __vector_table
#elif (defined __GNUC__)          /* GCC */
  extern unsigned char __Vectors;
#elif (defined __CC_ARM)          /* KEIL */
  extern unsigned char __Vectors;
#elif defined (__CLANG__) || (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)) /* ARM CLANG */
  extern unsigned char __Vectors;
#endif

/*********************************************************************
*
*       Local defines (sfrs and addresses used in RTOSInit.c)
*
**********************************************************************
*/
#define NVIC_VTOR         (*(volatile OS_U32*) (0xE000ED08uL))

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#if (OS_VIEW_IFSELECT == OS_VIEW_IF_JLINK)
  #include "JLINKMEM.h"
  const OS_U32 OS_JLINKMEM_BufferSize = 32u;  /* Size of the communication buffer for JLINKMEM */
#else
  const OS_U32 OS_JLINKMEM_BufferSize = 0;    /* Communication not used */
#endif

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _OS_GetHWTimerCycles()
*
*  Function description
*    Returns the current hardware timer count value
*
*  Return value
*    Current timer count value
*/
static unsigned int _OS_GetHWTimerCycles(void)
{
  return SysTick->VAL;
}

/*********************************************************************
*
*       _OS_GetHWTimer_IntPending()
*
*  Function description
*    Returns if the hardware timer interrupt pending flag is set
*
*  Return value
*    == 0; Interrupt pending flag not set
*    != 0: Interrupt pending flag set
*/
static unsigned int _OS_GetHWTimer_IntPending(void)
{
  return SCB->ICSR & SCB_ICSR_PENDSTSET_Msk;
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       SysTick_Handler()
*
*  Function description
*    This is the code that gets called when the processor receives a
*    _SysTick exception. SysTick is used as OS timer tick.
*/
#ifdef __cplusplus
extern "C" {
#endif
void SysTick_Handler(void);      /* Avoid warning, Systick_Handler is not prototyped in any CMSIS header */
#ifdef __cplusplus
}
#endif
void SysTick_Handler(void)
{
  OS_INT_EnterNestable();
  OS_TICK_Handle();
  #if (OS_VIEW_IFSELECT == OS_VIEW_IF_JLINK)
    JLINKMEM_Process();
  #endif
  OS_INT_LeaveNestable();
}

/*********************************************************************
*
*       OS_InitHW()
*
*  Function description
*    Initialize the hardware (timer) required for embOS to run.
*    May be modified, if an other timer should be used
*/
void OS_InitHW(void)
{
  /* Structure with information about timer frequency, tick frequency, etc.
   * for micro second precise system time.
   * SysTimerConfig.TimerFreq will be set later, thus it is initialized with zero.
   */
  OS_SYSTIMER_CONFIG SysTimerConfig = {0, OS_TICK_FREQ, 0, _OS_GetHWTimerCycles, _OS_GetHWTimer_IntPending};

  OS_INT_IncDI();

  /* We assume, the PLL and core clock was already set by the SystemInit() function
   * which was called from the startup code
   * Therefore, we don't have to initialize any hardware here,
   * we just ensure that the system clock variable is updated and then
   * set the periodic system timer tick for embOS.
   */
  SystemCoreClockUpdate();                /* Update the system clock variable (might not have been set before)  */
  if (SysTick_Config(OS_TIMER_RELOAD))
  {                                       /* Setup SysTick Timer for 1 msec interrupts                          */
    while (1);                            /* Handle Error                                                       */
  }
  //OS_TICK_Config(SystemCoreClock, 300000000);

  /* Initialize NVIC vector base address. Might be necessary for RAM targets or application not running from 0 */
  NVIC_VTOR = (OS_U32)&__Vectors;

  /* Set the interrupt priority for the system timer to 2nd lowest level to ensure the timer can preempt PendSV handler */
  NVIC_SetPriority(SysTick_IRQn, (1u << __NVIC_PRIO_BITS) - 2u);

  /* Setup values for usec precise system time functions */
  SysTimerConfig.TimerFreq = SystemCoreClock;
  OS_TIME_ConfigSysTimer(&SysTimerConfig);

  /* Enable Cortex M7 cache */
  SCB_EnableICache();
  SCB_EnableDCache();

  /* Configure and initialize SEGGER SystemView */
#if OS_PROFILE
  SEGGER_SYSVIEW_Conf();
#endif

  /* Configure RTT */
  SEGGER_RTT_Init();

  /* Initialize the optional communication for embOSView */
#if (OS_VIEW_IFSELECT != OS_VIEW_DISABLED)
  OS_COM_Init();
#endif
  OS_INT_DecRI();
}

/*********************************************************************
*
*       OS_Idle()
*
*  Function description
*    This is basically the "core" of the idle loop.
*    This core loop can be changed, but:
*    The idle loop does not have a stack of its own, therefore no
*    functionality should be implemented that relies on the stack
*    to be preserved. However, a simple program loop can be programmed
*    (like toggling an output or incrementing a counter)
*/
void OS_Idle(void)
{                        /* Idle loop: No task is ready to execute  */
  while (1) {            /* Nothing to do ... wait for interrupt    */
    #if ((OS_VIEW_IFSELECT != OS_VIEW_IF_JLINK) && (OS_DEBUG == 0))
      __WFI();           /* Switch CPU into sleep mode              */
    #endif
  }
}

/*********************************************************************
*
*       OS_GetTime_Cycles()
*
*  Function description
*    This routine is required for task-info via embOSView or high
*    resolution time measurement functions.
*    It returns the system time in timer clock cycles.
*/
OS_U32 OS_GetTime_Cycles(void)
{
  OS_U32 Time;
  OS_U32 Cnt;

  Time = OS_TIME_GetTicks32();
  Cnt  = OS_TIMER_RELOAD - SysTick->VAL;

  /* Check if timer interrupt pending ... */
  if (SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)
  {
    Cnt = OS_TIMER_RELOAD - SysTick->VAL;  /* Interrupt pending, re-read timer and adjust result */
    Time++;
  }
  return (OS_TIMER_RELOAD * Time) + Cnt;
}

/*********************************************************************
*
*       OS_ConvertCycles2us()
*
*  Function description
*    Convert Cycles into micro seconds.
*
*    If your clock frequency is not a multiple of 1 MHz,
*    you may have to modify this routine in order to get proper
*    diagnostics.
*
*    This routine is required for profiling or high resolution time
*    measurement only. It does not affect operation of the OS.
*/
OS_U32 OS_ConvertCycles2us(OS_U32 Cycles)
{
  return (Cycles / (OS_TIMER_RELOAD / 1000u));
}

/*********************************************************************
*
*       Optional communication with embOSView
*
**********************************************************************
*/
#if (OS_VIEW_IFSELECT == OS_VIEW_IF_JLINK)

/*********************************************************************
*
*       OS_COM_Send1()
*
*  Function description
*    Sends one character via J-Link
*/
void OS_COM_Send1(OS_U8 c)
{
  JLINKMEM_SendChar(c);
}

/*********************************************************************
*
*       OS_COM_Init()
*
*  Function description
*    Initializes J-Link communication for embOSView
*/
void OS_COM_Init(void)
{
  JLINKMEM_SetpfOnRx(OS_COM_OnRx);
  JLINKMEM_SetpfOnTx(OS_COM_OnTx);
  JLINKMEM_SetpfGetNextChar(OS_COM_GetNextChar);
}

#elif (OS_VIEW_IFSELECT == OS_VIEW_DISABLED)

/*********************************************************************
*
*       OS_COM_Send1()
*
*  Function description
*    Dummy routine.
*/
void OS_COM_Send1(OS_U8 c)
{
  OS_USEPARA(c);           /* Avoid compiler warning              */
  OS_COM_ClearTxActive();  /* Let embOS know that Tx is not busy  */
}

#else
  #error "Selected embOSView interface is currently not supported."
#endif

/****** End Of File *************************************************/

