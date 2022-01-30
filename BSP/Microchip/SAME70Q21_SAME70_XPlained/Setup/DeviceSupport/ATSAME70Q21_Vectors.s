/*****************************************************************************
 *                   SEGGER Microcontroller GmbH & Co. KG                    *
 *            Solutions for real time microcontroller applications           *
 *****************************************************************************
 *                                                                           *
 *               (c) 2017 SEGGER Microcontroller GmbH & Co. KG               *
 *                                                                           *
 *           Internet: www.segger.com   Support: support@segger.com          *
 *                                                                           *
 *****************************************************************************/

/*****************************************************************************
 *                         Preprocessor Definitions                          *
 *                         ------------------------                          *
 * VECTORS_IN_RAM                                                            *
 *                                                                           *
 *   If defined, an area of RAM will large enough to store the vector table  *
 *   will be reserved.                                                       *
 *                                                                           *
 *****************************************************************************/

  .syntax unified
  .code 16

  .section .init, "ax"
  .align 0

/*****************************************************************************
 * Default Exception Handlers                                                *
 *****************************************************************************/

  .thumb_func
  .weak NMI_Handler
NMI_Handler:
  b .

  .thumb_func
  .weak HardFault_Handler
HardFault_Handler:
  b .
  
  .thumb_func
  .weak MemManage_Handler
MemManage_Handler:
  b .
  
  .thumb_func
  .weak BusFault_Handler
BusFault_Handler:
  b .
  
  .thumb_func
  .weak UsageFault_Handler
UsageFault_Handler:
  b .

  .thumb_func
  .weak SVC_Handler
SVC_Handler:
  b .

  .thumb_func
  .weak DebugMon_Handler
DebugMon_Handler:
  b .

  .thumb_func
  .weak PendSV_Handler
PendSV_Handler:
  b .

  .thumb_func
  .weak SysTick_Handler
SysTick_Handler:
  b .

  .thumb_func
Dummy_Handler:
  b .

#if defined(__OPTIMIZATION_SMALL)

  .weak SUPC_IRQHandler
  .thumb_set SUPC_IRQHandler,Dummy_Handler
  
  .weak RSTC_IRQHandler
  .thumb_set RSTC_IRQHandler,Dummy_Handler

  .weak RTC_IRQHandler
  .thumb_set RTC_IRQHandler,Dummy_Handler

  .weak RTT_IRQHandler
  .thumb_set RTT_IRQHandler,Dummy_Handler

  .weak WDT_IRQHandler
  .thumb_set WDT_IRQHandler,Dummy_Handler

  .weak PMC_IRQHandler
  .thumb_set PMC_IRQHandler,Dummy_Handler

  .weak EFC_IRQHandler
  .thumb_set EFC_IRQHandler,Dummy_Handler

  .weak UART0_IRQHandler
  .thumb_set UART0_IRQHandler,Dummy_Handler

  .weak UART1_IRQHandler
  .thumb_set UART1_IRQHandler,Dummy_Handler

  .weak PIOA_IRQHandler
  .thumb_set PIOA_IRQHandler,Dummy_Handler

  .weak PIOB_IRQHandler
  .thumb_set PIOB_IRQHandler,Dummy_Handler

  .weak PIOC_IRQHandler
  .thumb_set PIOC_IRQHandler,Dummy_Handler

  .weak USART0_IRQHandler
  .thumb_set USART0_IRQHandler,Dummy_Handler

  .weak USART1_IRQHandler
  .thumb_set USART1_IRQHandler,Dummy_Handler

  .weak USART2_IRQHandler
  .thumb_set USART2_IRQHandler,Dummy_Handler

  .weak PIOD_IRQHandler
  .thumb_set PIOD_IRQHandler,Dummy_Handler

  .weak PIOE_IRQHandler
  .thumb_set PIOE_IRQHandler,Dummy_Handler

  .weak HSMCI_IRQHandler
  .thumb_set HSMCI_IRQHandler,Dummy_Handler

  .weak TWIHS0_IRQHandler
  .thumb_set TWIHS0_IRQHandler,Dummy_Handler

  .weak TWIHS1_IRQHandler
  .thumb_set TWIHS1_IRQHandler,Dummy_Handler

  .weak SPI0_IRQHandler
  .thumb_set SPI0_IRQHandler,Dummy_Handler

  .weak SSC_IRQHandler
  .thumb_set SSC_IRQHandler,Dummy_Handler

  .weak TC0_IRQHandler
  .thumb_set TC0_IRQHandler,Dummy_Handler

  .weak TC1_IRQHandler
  .thumb_set TC1_IRQHandler,Dummy_Handler

  .weak TC2_IRQHandler
  .thumb_set TC2_IRQHandler,Dummy_Handler

  .weak TC3_IRQHandler
  .thumb_set TC3_IRQHandler,Dummy_Handler

  .weak TC4_IRQHandler
  .thumb_set TC4_IRQHandler,Dummy_Handler

  .weak TC5_IRQHandler
  .thumb_set TC5_IRQHandler,Dummy_Handler

  .weak AFEC0_IRQHandler
  .thumb_set AFEC0_IRQHandler,Dummy_Handler

  .weak DACC_IRQHandler
  .thumb_set DACC_IRQHandler,Dummy_Handler

  .weak PWM0_IRQHandler
  .thumb_set PWM0_IRQHandler,Dummy_Handler

  .weak ICM_IRQHandler
  .thumb_set ICM_IRQHandler,Dummy_Handler

  .weak ACC_IRQHandler
  .thumb_set ACC_IRQHandler,Dummy_Handler

  .weak USBHS_IRQHandler
  .thumb_set USBHS_IRQHandler,Dummy_Handler

  .weak MCAN0_IRQHandler
  .thumb_set MCAN0_IRQHandler,Dummy_Handler

  .weak MCAN1_IRQHandler
  .thumb_set MCAN1_IRQHandler,Dummy_Handler

  .weak GMAC_IRQHandler
  .thumb_set GMAC_IRQHandler,Dummy_Handler

  .weak AFEC1_IRQHandler
  .thumb_set AFEC1_IRQHandler,Dummy_Handler

  .weak TWIHS2_IRQHandler
  .thumb_set TWIHS2_IRQHandler,Dummy_Handler

  .weak SPI1_IRQHandler
  .thumb_set SPI1_IRQHandler,Dummy_Handler

  .weak QSPI_IRQHandler
  .thumb_set QSPI_IRQHandler,Dummy_Handler

  .weak UART2_IRQHandler
  .thumb_set UART2_IRQHandler,Dummy_Handler

  .weak UART3_IRQHandler
  .thumb_set UART3_IRQHandler,Dummy_Handler

  .weak UART4_IRQHandler
  .thumb_set UART4_IRQHandler,Dummy_Handler

  .weak TC6_IRQHandler
  .thumb_set TC6_IRQHandler,Dummy_Handler

  .weak TC7_IRQHandler
  .thumb_set TC7_IRQHandler,Dummy_Handler

  .weak TC8_IRQHandler
  .thumb_set TC8_IRQHandler,Dummy_Handler

  .weak TC9_IRQHandler
  .thumb_set TC9_IRQHandler,Dummy_Handler

  .weak TC10_IRQHandler
  .thumb_set TC10_IRQHandler,Dummy_Handler

  .weak TC11_IRQHandler
  .thumb_set TC11_IRQHandler,Dummy_Handler

  .weak AES_IRQHandler
  .thumb_set AES_IRQHandler,Dummy_Handler

  .weak TRNG_IRQHandler
  .thumb_set TRNG_IRQHandler,Dummy_Handler

  .weak XDMAC_IRQHandler
  .thumb_set XDMAC_IRQHandler,Dummy_Handler

  .weak ISI_IRQHandler
  .thumb_set ISI_IRQHandler,Dummy_Handler

  .weak PWM1_IRQHandler
  .thumb_set PWM1_IRQHandler,Dummy_Handler

#else

  .thumb_func
  .weak SUPC_IRQHandler
SUPC_IRQHandler:
  b .

  .thumb_func
  .weak RSTC_IRQHandler
RSTC_IRQHandler:
  b .

  .thumb_func
  .weak RTC_IRQHandler
RTC_IRQHandler:
  b .

  .thumb_func
  .weak RTT_IRQHandler
RTT_IRQHandler:
  b .

  .thumb_func
  .weak WDT_IRQHandler
WDT_IRQHandler:
  b .

  .thumb_func
  .weak PMC_IRQHandler
PMC_IRQHandler:
  b .

  .thumb_func
  .weak EFC_IRQHandler
EFC_IRQHandler:
  b .

  .thumb_func
  .weak UART0_IRQHandler
UART0_IRQHandler:
  b .

  .thumb_func
  .weak UART1_IRQHandler
UART1_IRQHandler:
  b .

  .thumb_func
  .weak PIOA_IRQHandler
PIOA_IRQHandler:
  b .

  .thumb_func
  .weak PIOB_IRQHandler
PIOB_IRQHandler:
  b .

  .thumb_func
  .weak PIOC_IRQHandler
PIOC_IRQHandler:
  b .

  .thumb_func
  .weak USART0_IRQHandler
USART0_IRQHandler:
  b .

  .thumb_func
  .weak USART1_IRQHandler
USART1_IRQHandler:
  b .

  .thumb_func
  .weak USART2_IRQHandler
USART2_IRQHandler:
  b .

  .thumb_func
  .weak PIOD_IRQHandler
PIOD_IRQHandler:
  b .

  .thumb_func
  .weak PIOE_IRQHandler
PIOE_IRQHandler:
  b .

  .thumb_func
  .weak HSMCI_IRQHandler
HSMCI_IRQHandler:
  b .

  .thumb_func
  .weak TWIHS0_IRQHandler
TWIHS0_IRQHandler:
  b .

  .thumb_func
  .weak TWIHS1_IRQHandler
TWIHS1_IRQHandler:
  b .

  .thumb_func
  .weak SPI0_IRQHandler
SPI0_IRQHandler:
  b .

  .thumb_func
  .weak SSC_IRQHandler
SSC_IRQHandler:
  b .

  .thumb_func
  .weak TC0_IRQHandler
TC0_IRQHandler:
  b .

  .thumb_func
  .weak TC1_IRQHandler
TC1_IRQHandler:
  b .

  .thumb_func
  .weak TC2_IRQHandler
TC2_IRQHandler:
  b .

  .thumb_func
  .weak TC3_IRQHandler
TC3_IRQHandler:
  b .

  .thumb_func
  .weak TC4_IRQHandler
TC4_IRQHandler:
  b .

  .thumb_func
  .weak TC5_IRQHandler
TC5_IRQHandler:
  b .

  .thumb_func
  .weak AFEC0_IRQHandler
AFEC0_IRQHandler:
  b .

  .thumb_func
  .weak DACC_IRQHandler
DACC_IRQHandler:
  b .

  .thumb_func
  .weak PWM0_IRQHandler
PWM0_IRQHandler:
  b .

  .thumb_func
  .weak ICM_IRQHandler
ICM_IRQHandler:
  b .

  .thumb_func
  .weak ACC_IRQHandler
ACC_IRQHandler:
  b .

  .thumb_func
  .weak USBHS_IRQHandler
USBHS_IRQHandler:
  b .

  .thumb_func
  .weak MCAN0_IRQHandler
MCAN0_IRQHandler:
  b .

  .thumb_func
  .weak MCAN1_IRQHandler
MCAN1_IRQHandler:
  b .

  .thumb_func
  .weak GMAC_IRQHandler
GMAC_IRQHandler:
  b .

  .thumb_func
  .weak AFEC1_IRQHandler
AFEC1_IRQHandler:
  b .

  .thumb_func
  .weak TWIHS2_IRQHandler
TWIHS2_IRQHandler:
  b .

  .thumb_func
  .weak SPI1_IRQHandler
SPI1_IRQHandler:
  b .

  .thumb_func
  .weak QSPI_IRQHandler
QSPI_IRQHandler:
  b .

  .thumb_func
  .weak UART2_IRQHandler
UART2_IRQHandler:
  b .

  .thumb_func
  .weak UART3_IRQHandler
UART3_IRQHandler:
  b .

  .thumb_func
  .weak UART4_IRQHandler
UART4_IRQHandler:
  b .

  .thumb_func
  .weak TC6_IRQHandler
TC6_IRQHandler:
  b .

  .thumb_func
  .weak TC7_IRQHandler
TC7_IRQHandler:
  b .

  .thumb_func
  .weak TC8_IRQHandler
TC8_IRQHandler:
  b .

  .thumb_func
  .weak TC9_IRQHandler
TC9_IRQHandler:
  b .

  .thumb_func
  .weak TC10_IRQHandler
TC10_IRQHandler:
  b .

  .thumb_func
  .weak TC11_IRQHandler
TC11_IRQHandler:
  b .

  .thumb_func
  .weak AES_IRQHandler
AES_IRQHandler:
  b .

  .thumb_func
  .weak TRNG_IRQHandler
TRNG_IRQHandler:
  b .

  .thumb_func
  .weak XDMAC_IRQHandler
XDMAC_IRQHandler:
  b .

  .thumb_func
  .weak ISI_IRQHandler
ISI_IRQHandler:
  b .

  .thumb_func
  .weak PWM1_IRQHandler
PWM1_IRQHandler:
  b .

#endif

/*****************************************************************************
 * Vector Table                                                              *
 *****************************************************************************/

  .section .vectors, "ax"
  .align 0
  .global _vectors
  .extern __stack_end__
  .extern Reset_Handler

_vectors:
  .word __stack_end__
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0 /* Reserved */
  .word 0 /* Reserved */
  .word 0 /* Reserved */
  .word 0 /* Reserved */
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0 /* Reserved */
  .word PendSV_Handler
  .word SysTick_Handler
  .word SUPC_IRQHandler
  .word RSTC_IRQHandler
  .word RTC_IRQHandler
  .word RTT_IRQHandler
  .word WDT_IRQHandler
  .word PMC_IRQHandler
  .word EFC_IRQHandler
  .word UART0_IRQHandler
  .word UART1_IRQHandler
  .word Dummy_Handler /* Reserved */
  .word PIOA_IRQHandler
  .word PIOB_IRQHandler
  .word PIOC_IRQHandler
  .word USART0_IRQHandler
  .word USART1_IRQHandler
  .word USART2_IRQHandler
  .word PIOD_IRQHandler
  .word PIOE_IRQHandler
  .word HSMCI_IRQHandler
  .word TWIHS0_IRQHandler
  .word TWIHS1_IRQHandler
  .word SPI0_IRQHandler
  .word SSC_IRQHandler
  .word TC0_IRQHandler
  .word TC1_IRQHandler
  .word TC2_IRQHandler
  .word TC3_IRQHandler
  .word TC4_IRQHandler
  .word TC5_IRQHandler
  .word AFEC0_IRQHandler
  .word DACC_IRQHandler
  .word PWM0_IRQHandler
  .word ICM_IRQHandler
  .word ACC_IRQHandler
  .word USBHS_IRQHandler
  .word MCAN0_IRQHandler
  .word Dummy_Handler /* Reserved */
  .word MCAN1_IRQHandler
  .word Dummy_Handler /* Reserved */
  .word GMAC_IRQHandler
  .word AFEC1_IRQHandler
  .word TWIHS2_IRQHandler
  .word SPI1_IRQHandler
  .word QSPI_IRQHandler
  .word UART2_IRQHandler
  .word UART3_IRQHandler
  .word UART4_IRQHandler
  .word TC6_IRQHandler
  .word TC7_IRQHandler
  .word TC8_IRQHandler
  .word TC9_IRQHandler
  .word TC10_IRQHandler
  .word TC11_IRQHandler
  .word Dummy_Handler /* Reserved */
  .word Dummy_Handler /* Reserved */
  .word Dummy_Handler /* Reserved */
  .word AES_IRQHandler
  .word TRNG_IRQHandler
  .word XDMAC_IRQHandler
  .word ISI_IRQHandler
  .word PWM1_IRQHandler
_vectors_end:

#ifdef VECTORS_IN_RAM
  .section .vectors_ram, "ax"
  .align 0
  .global _vectors_ram

_vectors_ram:
  .space _vectors_end - _vectors, 0
#endif
