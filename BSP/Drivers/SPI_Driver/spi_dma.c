#include <same70.h>

#include <RTOS.h>

#include "spi_driver.h"
#include "dma.h"
#include "system.h"
#include "logWriter.h"


#define SPI_DMA_TIMEOUT   (1000u) /* ms */

extern OS_EVENT dmaEvt;


static void SPI_DMA_InitTransaction(Spi *pSpi, uint8_t *msg, uint8_t *recv, uint32_t len);


void SPI0_DMA_Init(void)
{
  XdmacChid     *pTxCh     = &XDMAC->XDMAC_CHID[DMA_SPI0_TX_CH];
  XdmacChid     *pRxCh     = &XDMAC->XDMAC_CHID[DMA_SPI0_RX_CH];
  const uint32_t irqMask   = XDMAC_CIE_ROIE
                           | XDMAC_CIE_WBIE
                           | XDMAC_CIE_RBIE;

  /* Configure SPI0 TX & RX:
   * - Single halfword transfer
   * - 8-bit data
   * - Hardware request
   * - Memory-to-peripheral
   */
  const uint32_t chCnfMask = XDMAC_CC_CSIZE_CHK_1
                           | XDMAC_CC_DWIDTH_BYTE
                           | XDMAC_CC_TYPE_PER_TRAN
                           | XDMAC_CC_SWREQ_HWR_CONNECTED;

  pTxCh->XDMAC_CC          |= chCnfMask
                           |  XDMAC_CC_DSYNC_MEM2PER
                           |  XDMAC_CC_SIF_AHB_IF0
                           |  XDMAC_CC_DIF_AHB_IF1
                           |  XDMAC_CC_PERID(DMA_HW_IF_SPI0_TX)
                           |  XDMAC_CC_DAM_FIXED_AM;

  pRxCh->XDMAC_CC          |= chCnfMask
                           |  XDMAC_CC_DSYNC_PER2MEM
                           |  XDMAC_CC_SIF_AHB_IF1
                           |  XDMAC_CC_DIF_AHB_IF0
                           |  XDMAC_CC_PERID(DMA_HW_IF_SPI0_RX)
                           |  XDMAC_CC_SAM_FIXED_AM;

  pTxCh->XDMAC_CIE         = irqMask;
  pRxCh->XDMAC_CIE         = irqMask;

  /* The following registers need to be cleared */
  pTxCh->XDMAC_CNDC    = 0;
  pTxCh->XDMAC_CBC     = 0;
  pTxCh->XDMAC_CDS_MSP = 0;
  pTxCh->XDMAC_CSUS    = 0;
  pTxCh->XDMAC_CDUS    = 0;
  pRxCh->XDMAC_CNDC    = 0;
  pRxCh->XDMAC_CBC     = 0;
  pRxCh->XDMAC_CDS_MSP = 0;
  pRxCh->XDMAC_CSUS    = 0;
  pRxCh->XDMAC_CDUS    = 0;

  while (((pTxCh->XDMAC_CC & XDMAC_CC_INITD_IN_PROGRESS) != 0)
      && ((pRxCh->XDMAC_CC & XDMAC_CC_INITD_IN_PROGRESS) != 0))
  {
    ; /* Poll until init done */
  }
}


static void SPI_DMA_InitTransaction(Spi       *pSpi,
                                    uint8_t   *msg,
                                    uint8_t   *recv,
                                    uint32_t   len)
{
  uint32_t        bsize;
  XdmacChid      *pTxCh = &XDMAC->XDMAC_CHID[DMA_SPI0_TX_CH];
  XdmacChid      *pRxCh = &XDMAC->XDMAC_CHID[DMA_SPI0_RX_CH];
  static uint8_t  dummy = 0x00;

  /* Reality check */
  assert((pSpi == SPI0) || (pSpi == SPI1));
  assert(len > 0);
  assert(((msg == NULL) && (recv == NULL)) == false);

  /* Clear pending interrupt requests */
  (void)pTxCh->XDMAC_CIS;
  (void)pRxCh->XDMAC_CIS;

  /* Flush channels */
  XDMAC->XDMAC_GSWF = XDMAC_GSWF_SWF2 | XDMAC_GSWF_SWF1;
  while ((pTxCh->XDMAC_CIS & pRxCh->XDMAC_CIS & XDMAC_CIS_FIS) != 0)
  {
      ; /* Wait until FIFO written to memory */
  }

  /* Set transmitter */
  pTxCh->XDMAC_CDA  =  XDMAC_CDA_DA((uint32_t)&pSpi->SPI_TDR);
  pTxCh->XDMAC_CC  &= ~XDMAC_CC_SAM_Msk;
  if (msg != NULL)
  {
      pTxCh->XDMAC_CC  |= XDMAC_CC_SAM_INCREMENTED_AM;
      pTxCh->XDMAC_CSA  = XDMAC_CSA_SA((uint32_t)msg);
  }
  else
  {
      /* Send dummy bytes if NULL pointer passed */
      pTxCh->XDMAC_CC  |= XDMAC_CC_SAM_FIXED_AM;
      pTxCh->XDMAC_CSA  = XDMAC_CSA_SA((uint32_t)&dummy);
  }

  /* Setup receiver */
  pRxCh->XDMAC_CSA  =  XDMAC_CSA_SA((uint32_t)&pSpi->SPI_RDR);
  pRxCh->XDMAC_CC  &= ~XDMAC_CC_DAM_Msk;
  if (recv != NULL)
  {
      pRxCh->XDMAC_CC  |= XDMAC_CC_DAM_INCREMENTED_AM;
      pRxCh->XDMAC_CDA  = XDMAC_CDA_DA((uint32_t)recv);
  }
  else
  {
      /* Discard received bytes if NULL pointer passed */
      pRxCh->XDMAC_CC  |= XDMAC_CC_DAM_FIXED_AM;
      pRxCh->XDMAC_CDA  = XDMAC_CDA_DA((uint32_t)&dummy);
  }

  /* Set transfer length */
  pTxCh->XDMAC_CUBC = XDMAC_CUBC_UBLEN(len);
  pRxCh->XDMAC_CUBC = XDMAC_CUBC_UBLEN(len);

  /* Burst size 32, 16 or 8 bits */
  bsize = DMA_ComputeBurstSize(len);
  pTxCh->XDMAC_CC &= ~XDMAC_CC_MBSIZE_Msk;
  pRxCh->XDMAC_CC &= ~XDMAC_CC_MBSIZE_Msk;
  pTxCh->XDMAC_CC |=  bsize;
  pRxCh->XDMAC_CC |=  bsize;
}


bool SPI0_DMA_TransmitMessage(uint8_t *msg, uint8_t *recv, uint32_t len)
{
  OS_TASKEVENT  evtMask;
  bool          ret       = true;
  XdmacChid    *pTxCh     = &XDMAC->XDMAC_CHID[DMA_SPI0_TX_CH];
  XdmacChid    *pRxCh     = &XDMAC->XDMAC_CHID[DMA_SPI0_RX_CH];
  Spi          *spi       = SPI0;

  const uint32_t timeoutMs = SPI_DMA_TIMEOUT;

  /* Clean DCache before DMA tansfer (AT17417) */
  if (msg != NULL)
  {
      SCB_CleanDCache_by_Addr((uint32_t *)msg,  len);
  }
  if (recv != NULL)
  {
      SCB_CleanDCache_by_Addr((uint32_t *)recv, len);
  }

  SPI_DMA_InitTransaction(spi, msg, recv, len);

  pTxCh->XDMAC_CNDC    = 0;
  pTxCh->XDMAC_CBC     = 0;
  pTxCh->XDMAC_CDS_MSP = 0;
  pTxCh->XDMAC_CSUS    = 0;
  pTxCh->XDMAC_CDUS    = 0;

  pRxCh->XDMAC_CNDC    = 0;
  pRxCh->XDMAC_CBC     = 0;
  pRxCh->XDMAC_CDS_MSP = 0;
  pRxCh->XDMAC_CSUS    = 0;
  pRxCh->XDMAC_CDUS    = 0;

  __DMB();

  /* Enable DMA IRQ */
  XDMAC->XDMAC_GIE = XDMAC_GIE_IE2 | XDMAC_GIE_IE1;
  pTxCh->XDMAC_CIE = XDMAC_CIE_BIE;
  pRxCh->XDMAC_CIE = XDMAC_CIE_BIE;

  /* Enable SPI & DMA */
  spi->SPI_CR   |= SPI_CR_SPIEN;
  __DMB();
  XDMAC->XDMAC_GE  = XDMAC_GE_EN2 | XDMAC_GE_EN1;

  /* Wait for signal from DMA handler */
  evtMask = DMA_EVENT_SPI0_TX | DMA_EVENT_SPI0_RX;
  evtMask = OS_EVENT_GetMaskTimed(&dmaEvt, evtMask, timeoutMs);

  /* Disable SPI & DMA */
  XDMAC->XDMAC_GD  = XDMAC_GD_DI2 | XDMAC_GD_DI1;
  __DMB();
  spi->SPI_CR |= SPI_CR_SPIDIS;
    
  /* Disable DMA IRQ */
  pTxCh->XDMAC_CID = XDMAC_CID_BID;
  pRxCh->XDMAC_CID = XDMAC_CID_BID;
  XDMAC->XDMAC_GID = XDMAC_GID_ID2 | XDMAC_GID_ID1;
  
  /* Invalidate DCache after DMA tansfer (AT17417) */
  if (msg != NULL)
  {
      SCB_InvalidateDCache_by_Addr((uint32_t *)msg,  len);
  }
  if (recv != NULL)
  {
      SCB_InvalidateDCache_by_Addr((uint32_t *)recv, len);
  }

  /* Check for errors */
  if (((evtMask & DMA_EVENT_SPI0_TX) == 0)
   || ((evtMask & DMA_EVENT_SPI0_RX) == 0))
  {
    ret = false;
    Journal_vWriteError(DMA_ERROR);
  }
  return ret;
}
