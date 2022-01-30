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


void DMA_Init(void);
void DMA_memcpy(uint32_t *dest, uint32_t *src, uint32_t n);

#endif /* DMA_H_ */
