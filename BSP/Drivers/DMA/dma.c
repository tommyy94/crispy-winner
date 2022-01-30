#include <same70.h>
#include <RTOS.h>

#include "dma.h"
#include "logWriter.h"
#include "system.h"


extern OS_EVENT dmaEvt;
extern OS_TASK journalTCB;


static void DMA_Init_memcpy(void);


void DMA_Init(void)
{
  Xdmac *dma = XDMAC;
  Pmc   *pmc = PMC;

  /* Enable XDMAC clock gating */
  //pmc->PMC_PCR |= PMC_PCR_CMD | PMC_PCR_PID(XDMAC_CLOCK_ID) | PMC_PCR_EN;
  pmc->PMC_PCR |= PMC_PCR_CMD | PMC_PCR_PID(ID_XDMAC) | PMC_PCR_EN;
  
  /* Disable all channels */
  dma->XDMAC_GD = 0xFFFFFFFF;

  DMA_Init_memcpy();
  
  OS_ARM_ISRSetPrio(XDMAC_IRQn + IRQn_OFFSET, XDMAC_IRQ_PRIO);
  OS_ARM_EnableISR(XDMAC_IRQn + IRQn_OFFSET);
}


static void DMA_Init_memcpy(void)
{
  Xdmac *dma = XDMAC;
  
  /* Memory-to-memory */
  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CC  = XDMAC_CC_TYPE_MEM_TRAN;
  
  /* Single byte transfer
   * single burst byte bursts
   * 32-bit data
   * Read and write data through the system bus interface 0
   * Hardware request
   */
  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CC |= XDMAC_CC_CSIZE_CHK_1 | XDMAC_CC_MBSIZE_SINGLE
                                         |  XDMAC_CC_DWIDTH_WORD
                                         |  XDMAC_CC_SIF_AHB_IF0 | XDMAC_CC_DIF_AHB_IF0
                                         |  XDMAC_CC_SWREQ_SWR_CONNECTED;

  /* The following registers need to be cleared */
  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CNDC    = 0;
  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CBC     = 0;
  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CDS_MSP = 0;
  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CSUS    = 0;
  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CDUS    = 0;
}


void DMA_memcpy(uint32_t *dest, uint32_t *src, uint32_t n)
{
  Xdmac *dma = XDMAC;

  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CSA  = (uint32_t)src;
  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CDA  = (uint32_t)dest;
  dma->XDMAC_CHID[DMA_MEMCPY_CH].XDMAC_CUBC = n;

  /* Enable interrupts if needed... */

  /* Enable channel 0 */
  __DMB();
  dma->XDMAC_GE = XDMAC_GE_EN0;

  /* Poll for transfer completion */
  while ((dma->XDMAC_GS & XDMAC_GS_ST0) != 0);
}


void XDMAC_Handler(void)
{
  uint32_t globalIrqMask;
  uint32_t spiStatus;

  OS_INT_Enter();

  const uint32_t dmaErrMask = XDMAC_CIS_ROIS | XDMAC_CIS_WBEIS | XDMAC_CIS_RBEIS;
  Xdmac *dma = XDMAC;

  globalIrqMask = dma->XDMAC_GIS;

  /* Pending IRQ cleared by reading XDMAC_CISx */
  spiStatus  = (dma->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CIS & dmaErrMask);
  spiStatus |= (dma->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CIS & dmaErrMask);
  if (spiStatus != 0)
  {
    Journal_vWriteError(DMA_ERROR);
    __BKPT();
  }

  /* Signal task */
  OS_EVENT_SetMask(&dmaEvt, globalIrqMask);

  OS_INT_Leave();
}
