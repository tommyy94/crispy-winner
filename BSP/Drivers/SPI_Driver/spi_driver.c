#include <same70.h>
#include "RTOS.h"

#include "spi_driver.h"
#include "pmc_driver.h"
#include "system.h"
#include "logWriter.h"
#include "io.h"


typedef enum
{
    MODE_0,
    MODE_1,
    MODE_2,
    MODE_3,
    MODE_COUNT
} SPI_Mode;


typedef enum
{
    SLAVE_0,
    SLAVE_1,
    SLAVE_2,
    SLAVE_3,
    SLAVE_COUNT
} SPI_SlaveSelect;


extern void SPI0_IRQHandler(void);

__STATIC_INLINE void SPI0_IO_Init(void);
__STATIC_INLINE void SPI_Reset(Spi *spi);
__STATIC_INLINE void SPI_SetMode(Spi *spi, SPI_SlaveSelect slave, SPI_Mode mode);


void SPI0_Init(void)
{
    Spi *spi = SPI0;
    SPI_SlaveSelect slave = SLAVE_1;
    
    SPI0_IO_Init();
    SPI0_DMA_Init();
    
    /**
     * Enable SPI0 clock gating
     * - SPI0 clock = Peripheral clock / 2
     *              = 150 MHz / 2 = 75 MHz
     */
    PMC_PeripheralClockEnable(ID_SPI0);
    
    /* Wait data read before transfer
     * Master Mode
     * Chip Select 1
     */
    spi->SPI_MR = SPI_MR_MSTR | SPI_MR_PCS(~(1u << slave));
    
    /* SPI0.CS1 Baud rate = Main clock / 2
     * Serial Clock Bit Rate = f_perclock / SPCK Bit Rate
     *                       = 75 MHz / 16 = ~4.7 MHz (measured 4.5 MHz)
     * NOTE: SCBR should not be 0!
     * 8 bits per transfer
     */
    spi->SPI_CSR[slave] = SPI_CSR_SCBR(32) | SPI_CSR_BITS_8_BIT; /* 2.6 MHz */
    assert((spi->SPI_CSR[slave] & SPI_CSR_SCBR_Msk) != 0);
    
    /* CPOL = 0, CPHA = 1, SPCK inactive LOW */
    SPI_SetMode(spi, slave, MODE_1);
    
    /* Send data only when receive register empty */
    spi->SPI_MR |= SPI_MR_WDRBT;

    /* Enable error interrupts for:
     * - Overrun error
     * - Mode Fault error
     */
    spi->SPI_IER = SPI_IER_OVRES | SPI_IER_MODF;

    NVIC_ClearPendingIRQ(SPI0_IRQn);
    NVIC_SetPriority(SPI0_IRQn, SPI0_IRQ_PRIO);
    NVIC_EnableIRQ(SPI0_IRQn);
}


__STATIC_INLINE void SPI0_IO_Init(void)
{
    //const uint32_t selfTestPin = PIO_ABCDSR_P2;   /* CS0  */
    const uint32_t spiMask     = PIO_ABCDSR_P25   /* CS1  */
                               | PIO_ABCDSR_P22   /* SCK  */
                               | PIO_ABCDSR_P21   /* MOSI */
                               | PIO_ABCDSR_P20;  /* MISO */

    /* Enable internal pulldown for MISO */
    IO_ConfigurePull(PIOD, PIO_PUDR_P20, IO_PULLDOWN);

    IO_SetPeripheralFunction(PIOD, spiMask, IO_PERIPH_B);
    //IO_SetPeripheralFunction(PIOB, selfTestPin, IO_PERIPH_D);
}


/**
 * If multiple slaves are connected and require different configurations,
 * the master must reconfigure itself each time it needs to communicate
 * with a different slave
 */
static void SPI_SetMode(Spi *spi, SPI_SlaveSelect slave, SPI_Mode mode)
{
    assert((spi == SPI0) || (spi == SPI1));
    assert(slave < SLAVE_COUNT);
    assert(mode < MODE_COUNT);
    
    /* NOTE: The bit field positions are inverted in the register:
     * CPOL on the LSB side and NCPHA on the MSB side whereas in the datasheet
     * CPOL is represented on left and NCPHA on right in the mode table
     */
    const uint32_t modeTable[MODE_COUNT] =
    {
        0,                            /* Mode 0 */
        SPI_CSR_NCPHA,                /* Mode 1 */
        SPI_CSR_CPOL,                 /* Mode 2 */
        SPI_CSR_CPOL | SPI_CSR_NCPHA  /* Mode 3 */
    };
    
    spi->SPI_CSR[slave] &= ~(SPI_CSR_CPOL | SPI_CSR_NCPHA);
    __DMB(); /* Clear register first */
    spi->SPI_CSR[slave] |= modeTable[mode];
}


/* Loop Back Mode connected to SS0 (undocumented) */
bool SPI_SelfTest(Spi *spi, uint8_t *msg, uint8_t *recv, uint32_t len)
{
    bool ret = true;

    /* Enable Local Loopback */
    spi->SPI_MR |= SPI_MR_LLB;
       
    SPI0_DMA_TransmitMessage(msg, recv, len);
    
    for (uint32_t i = 0; i < len; i++)
    {
        if (msg[i] != recv[i])
        {
            ret = false;
            break;
        }
    }

    /* Disable Local Loopback */
    spi->SPI_MR &= ~SPI_MR_LLB;
    
    return ret;
}


uint16_t SPI0_vTransmitHalfword(uint16_t const halfword)
{
    Spi *spi = SPI0;

    spi->SPI_CR  |= SPI_CR_SPIEN;

    /* Clear RX register */
    (void)spi->SPI_RDR;

    /* Transmit character */
    spi->SPI_TDR  = SPI_TDR_TD(halfword);
    while ((spi->SPI_SR & SPI_SR_RDRF) == 0)
    {
        ; /* Wait until character received */
    }

    spi->SPI_CR  |= SPI_CR_SPIDIS;

    return spi->SPI_RDR & 0xFFFF;
}

uint8_t SPI0_vTransmitByte(uint8_t const byte)
{
    Spi *spi = SPI0;

    spi->SPI_CR  |= SPI_CR_SPIEN;

    /* Clear RX register */
    (void)spi->SPI_RDR;

    /* Transmit character */
    spi->SPI_TDR  = SPI_TDR_TD(byte);
    while ((spi->SPI_SR & SPI_SR_RDRF) == 0)
    {
        ; /* Wait until character received */
    }

    spi->SPI_CR  |= SPI_CR_SPIDIS;

    return spi->SPI_RDR & 0xFF;
}

__STATIC_INLINE void SPI_Reset(Spi *spi)
{
    spi->SPI_CR |= SPI_CR_SWRST;
}

void SPI0_IRQHandler(void)
{
    uint32_t status;

    OS_INT_Enter();

    status = SPI0->SPI_SR;
    assert((status & SPI_SR_MODF)  == 0);
    Journal_vWriteError(SPI_ERROR);

    OS_INT_Leave();
}
