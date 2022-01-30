#include <same70.h>

#include <RTOS.h>

#include "spi_driver.h"
#include "dma.h"
#include "system.h"
#include "logWriter.h"


#define SPI_DMA_TIMEOUT   (100u) /* ms */

extern OS_EVENT dmaEvt;


void SPI0_DMA_Init(void)
{
  /* Assume DMA clock gating enabled */

  Xdmac *dma = XDMAC;
  
  /* Configure SPI0 TX:
   * - Single halfword transfer
   * - 16-bit data
   * - Read and write data through the system bus interface 0
   * - Hardware request
   * - Memory-to-peripheral
   */
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CC
    |= XDMAC_CC_CSIZE_CHK_1
    |  XDMAC_CC_DWIDTH_BYTE
    |  XDMAC_CC_DSYNC_MEM2PER
    |  XDMAC_CC_TYPE_PER_TRAN
    |  XDMAC_CC_SIF_AHB_IF0
    |  XDMAC_CC_DIF_AHB_IF1
    |  XDMAC_CC_SWREQ_HWR_CONNECTED
    |  XDMAC_CC_PERID(ID_SPI0)
    |  XDMAC_CC_SAM_INCREMENTED_AM
    |  XDMAC_CC_DAM_FIXED_AM;

  /* Configure SPI0 RX:
   * - Single halfword transfer
   * - 16-bit data
   * - Read and write data through the system bus interface 0
   * - Hardware request
   * - Peripheral-to-memory
   */
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CC
    |= XDMAC_CC_CSIZE_CHK_1
    |  XDMAC_CC_DWIDTH_BYTE
    |  XDMAC_CC_DSYNC_PER2MEM
    |  XDMAC_CC_TYPE_PER_TRAN
    |  XDMAC_CC_SIF_AHB_IF1
    |  XDMAC_CC_DIF_AHB_IF0
    |  XDMAC_CC_SWREQ_HWR_CONNECTED
    |  XDMAC_CC_PERID(ID_SPI0)
    |  XDMAC_CC_SAM_FIXED_AM
    |  XDMAC_CC_DAM_INCREMENTED_AM;

  /* The following registers need to be cleared */
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CNDC    = 0;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CNDC    = 0;
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CBC     = 0;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CBC     = 0;
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CDS_MSP = 0;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CDS_MSP = 0;
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CSUS    = 0;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CSUS    = 0;
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CDUS    = 0;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CDUS    = 0;

  while (((dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CC & XDMAC_CC_INITD_IN_PROGRESS) != 0)
      && ((dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CC & XDMAC_CC_INITD_IN_PROGRESS) != 0))
  {
    ; /* Poll until init done */
  }
}


static void SPI0_DMA_InitTransaction(uint8_t *msg, uint8_t *recv, uint32_t len)
{
  uint32_t bsize = 0;
  Xdmac *dma = XDMAC;
  Spi *spi = SPI0;

  assert(len > 0);

  /* Clear pending interrupt requests */
  (void)dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CIS;
  (void)dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CIS;

  /* Flush channels */
  dma->XDMAC_GSWF = XDMAC_GSWF_SWF2 | XDMAC_GSWF_SWF1;
  while ((dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CIS &
          dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CIS &
          XDMAC_CIS_FIS) != 0)
  {
      ; /* Wwait until FIFO written to memory */
  }

  /* Set addresses and transfer length */
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CSA  = XDMAC_CSA_SA((uint32_t)msg);
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CSA  = XDMAC_CSA_SA((uint32_t)&spi->SPI_RDR);
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CDA  = XDMAC_CDA_DA((uint32_t)&spi->SPI_TDR);
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CDA  = XDMAC_CDA_DA((uint32_t)recv);
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CUBC = XDMAC_CUBC_UBLEN(len);
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CUBC = XDMAC_CUBC_UBLEN(len);

  if ((len % 16) == 0)
  {
    if ((len / 16) > 0)
    {
      bsize = XDMAC_CC_MBSIZE_SIXTEEN;
    }
  }
  else if ((len % 8) == 0)
  {
    if ((len / 8) > 0)
    {
      bsize = XDMAC_CC_MBSIZE_EIGHT;
    }
  }
  else if ((len % 4) == 0)
  {
    if ((len / 4) > 0)
    {
      bsize = XDMAC_CC_MBSIZE_FOUR;
    }
  }
  else
  {
    bsize = XDMAC_CC_MBSIZE_SINGLE;
  }

  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CC &= ~XDMAC_CC_MBSIZE_Msk;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CC &= ~XDMAC_CC_MBSIZE_Msk;

  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CC |= bsize;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CC |= bsize;
}


void SPI0_DMA_TransmitMessage(uint8_t *msg, uint8_t *recv, uint32_t len)
{
  OS_TASKEVENT   evtMask;
  Xdmac         *dma       = XDMAC;
  Spi           *spi       = SPI0;
  //const uint32_t timeoutMs = OS_TIME_Convertms2Ticks(SPI_DMA_TIMEOUT);
  const uint32_t timeoutMs = SPI_DMA_TIMEOUT;

  /* Clean DCache before DMA tansfer (AT17417) */
  SCB_CleanDCache_by_Addr((uint32_t *)msg,  len);
  SCB_CleanDCache_by_Addr((uint32_t *)recv, len);

  SPI0_DMA_InitTransaction(msg, recv, len);

  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CNDC    = 0;
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CBC     = 0;
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CDS_MSP = 0;
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CSUS    = 0;
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CDUS    = 0;

  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CNDC    = 0;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CBC     = 0;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CDS_MSP = 0;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CSUS    = 0;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CDUS    = 0;

  __DMB();

  /* Enable DMA IRQ */
  dma->XDMAC_GIE = XDMAC_GIE_IE2 | XDMAC_GIE_IE1;
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CIE = XDMAC_CIE_BIE;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CIE = XDMAC_CIE_BIE;

  /* Enable SPI & DMA */
  spi->SPI_CR   |= SPI_CR_SPIEN;
  __DMB();
  dma->XDMAC_GE  = XDMAC_GE_EN2 | XDMAC_GE_EN1;

  /* Wait for signal from DMA handler */
  evtMask = DMA_EVENT_SPI0_TX | DMA_EVENT_SPI0_RX;
  evtMask = OS_EVENT_GetMaskTimed(&dmaEvt, evtMask, timeoutMs);

  /* Disable SPI & DMA */
  dma->XDMAC_GD  = XDMAC_GD_DI2 | XDMAC_GD_DI1;
  __DMB();
  spi->SPI_CR   |= SPI_CR_SPIDIS;
    
  /* Disable DMA IRQ */
  dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CID = XDMAC_CID_BID;
  dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CID = XDMAC_CID_BID;
  dma->XDMAC_GID = XDMAC_GID_ID2 | XDMAC_GID_ID1;
  
  /* Invalidate DCache after DMA tansfer (AT17417) */
  SCB_InvalidateDCache_by_Addr((uint32_t *)msg,  len);
  SCB_InvalidateDCache_by_Addr((uint32_t *)recv, len);

  /* Check for errors */
  if (((evtMask & DMA_EVENT_SPI0_TX) == 0)
   || ((evtMask & DMA_EVENT_SPI0_RX) == 0))
  {
    Journal_vWriteError(DMA_ERROR);
  }
}
