#ifndef DMA_H_
#define DMA_H_

#include <same70.h>


/*********************
* DMA CHANNEL MAP
*
* CH0  = memcpy()
* CH1  = SPI0_TX
* CH2  = SPI0_RX
*
**********************/


#define DMA_MEMCPY_CH   (0u)
#define DMA_SPI0_TX_CH  (1u)
#define DMA_SPI0_RX_CH  (2u)

enum
{
  DMA_EVENT_MEMCPY  = (XDMAC_GIS_IS0),
  DMA_EVENT_SPI0_TX = (XDMAC_GIS_IS1),
  DMA_EVENT_SPI0_RX = (XDMAC_GIS_IS2)
};


typedef enum
{
    DMA_HW_IF_HSMCI_TX_RX = 0,
    DMA_HW_IF_SPI0_TX,
    DMA_HW_IF_SPI0_RX,
    DMA_HW_IF_SPI1_TX,
    DMA_HW_IF_SPI1_RX,
    DMA_HW_IF_QSPI_TX,
    DMA_HW_IF_QSPI_RX,
    DMA_HW_IF_USART0_TX,
    DMA_HW_IF_USART0_RX,
    DMA_HW_IF_USART1_TX,
    DMA_HW_IF_USART1_RX,
    DMA_HW_IF_USART2_TX,
    DMA_HW_IF_USART2_RX,
    DMA_HW_IF_PWM0_TX,
    DMA_HW_IF_TWIHS0_TX,
    DMA_HW_IF_TWIHS0_RX,
    DMA_HW_IF_TWIHS1_TX,
    DMA_HW_IF_TWIHS1_RX,
    DMA_HW_IF_TWIHS2_TX,
    DMA_HW_IF_TWIHS2_RX,
    DMA_HW_IF_UART0_TX,
    DMA_HW_IF_UART0_RX,
    DMA_HW_IF_UART1_TX,
    DMA_HW_IF_UART1_RX,
    DMA_HW_IF_UART2_TX,
    DMA_HW_IF_UART2_RX,
    DMA_HW_IF_UART3_TX,
    DMA_HW_IF_UART3_RX,
    DMA_HW_IF_UART4_TX,
    DMA_HW_IF_UART4_RX,
    DMA_HW_IF_DACC_TX,
    DMA_HW_IF_SSC_TX,
    DMA_HW_IF_SSC_RX,
    DMA_HW_IF_PIOA_RX,
    DMA_HW_IF_AFEC0_RX,
    DMA_HW_IF_AFEC1_RX,
    DMA_HW_IF_AES_TX,
    DMA_HW_IF_AES_RX,
    DMA_HW_IF_PWM1_TX,
    DMA_HW_IF_TC0_RX,
    DMA_HW_IF_TC3_RX,
    DMA_HW_IF_TC6_RX,
    DMA_HW_IF_TC9_RX,
    DMA_HW_IF_I2SC0_L_TX,
    DMA_HW_IF_I2SC0_L_RX,
    DMA_HW_IF_I2SC1_L_TX,
    DMA_HW_IF_I2SC1_L_RX,
    DMA_HW_IF_I2SC0_R_TX,
    DMA_HW_IF_I2SC0_R_RX,
    DMA_HW_IF_I2SC1_R_TX,
    DMA_HW_IF_I2SC1_R_RX,
    DMA_HW_IF_CNT
} DMA_HW_Interface_t;



void      DMA_Init(void);
void      DMA_memcpy(uint32_t *dest, uint32_t *src, uint32_t n);
uint32_t  DMA_ComputeBurstSize(const uint32_t len);

#endif /* DMA_H_ */
