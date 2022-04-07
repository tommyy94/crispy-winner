#ifndef SPI_H_
#define SPI_H_

#include <stdbool.h>
#include <same70.h>


#define SPI_QUEUE_SIZE		(4)

void	 SPI0_Init(void);
bool     SPI_SelfTest(Spi *spi, uint8_t *msg, uint8_t *recv, uint32_t len);
uint16_t SPI0_vTransmitHalfword(uint16_t const halfword);
uint8_t  SPI0_vTransmitByte(uint8_t const byte);

void     SPI0_DMA_Init(void);
bool     SPI0_DMA_TransmitMessage(uint8_t *msg, uint8_t *recv, uint32_t len);


#endif /* SPI_H_ */
