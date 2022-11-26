#include <same70.h>
#include <RTOS.h>

#include "dma.h"
#include "logWriter.h"
#include "pmc_driver.h"
#include "system.h"


extern OS_EVENT dmaEvt;
extern OS_TASK journalTCB;

extern void XDMAC_IRQHandler(void);

static void DMA_Init_memcpy(void);


void DMA_Init(void)
{
  /* Enable XDMAC clock gating */
  PMC_PeripheralClockEnable(ID_XDMAC);
  
  /* Disable all channels */
  XDMAC->XDMAC_GD = 0xFFFFFFFF;

  DMA_Init_memcpy();

  NVIC_ClearPendingIRQ(XDMAC_IRQn);
  NVIC_SetPriority(XDMAC_IRQn, XDMAC_IRQ_PRIO);
  NVIC_EnableIRQ(XDMAC_IRQn);
}


static void DMA_Init_memcpy(void)
{  
  XdmacChid *pDmaCh = &XDMAC->XDMAC_CHID[DMA_MEMCPY_CH];
  
  /* Memory-to-memory
   * Single byte transfer
   * single burst byte bursts
   * 32-bit data
   * Read and write data through the system bus interface 0
   * Hardware request
   * Increment source and destination addresses
   */
  pDmaCh->XDMAC_CC |= XDMAC_CC_CSIZE_CHK_1
                   |  XDMAC_CC_MBSIZE_SINGLE
                   |  XDMAC_CC_DWIDTH_WORD
                   |  XDMAC_CC_SIF_AHB_IF0
                   |  XDMAC_CC_DIF_AHB_IF0
                   |  XDMAC_CC_SWREQ_SWR_CONNECTED
                   |  XDMAC_CC_SAM_INCREMENTED_AM
                   |  XDMAC_CC_DAM_INCREMENTED_AM;

  /* The following registers need to be cleared */
  pDmaCh->XDMAC_CNDC    = 0;
  pDmaCh->XDMAC_CBC     = 0;
  pDmaCh->XDMAC_CDS_MSP = 0;
  pDmaCh->XDMAC_CSUS    = 0;
  pDmaCh->XDMAC_CDUS    = 0;
}


void DMA_memcpy(uint32_t *dest, uint32_t *src, uint32_t n)
{
  XdmacChid *pDmaCh = &XDMAC->XDMAC_CHID[DMA_MEMCPY_CH];

  pDmaCh->XDMAC_CSA  = XDMAC_CSA_SA((uint32_t)src);
  pDmaCh->XDMAC_CDA  = XDMAC_CDA_DA((uint32_t)dest);
  pDmaCh->XDMAC_CUBC = XDMAC_CUBC_UBLEN(n);

  /* Enable interrupts if needed... */

  /* Enable channel 0 */
  __DMB();
  XDMAC->XDMAC_GE = XDMAC_GE_EN0;

  /* Poll for transfer completion */
  while ((XDMAC->XDMAC_GS & XDMAC_GS_ST0) != 0);
}

uint32_t DMA_ComputeBurstSize(const uint32_t len)
{
    uint32_t bsize;

    /* Determine burst size */
    bsize = XDMAC_CC_MBSIZE_SINGLE;
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
        /* Burst size should be single if we ever get here */
    }

    bsize = XDMAC_CC_MBSIZE_SINGLE;
    return bsize;
}


void XDMAC_IRQHandler(void)
{
  uint32_t        globalIrqMask;
  uint32_t        spiStatus;
  const uint32_t  dmaErrMask = XDMAC_CIS_ROIS
                             | XDMAC_CIS_WBEIS
                             | XDMAC_CIS_RBEIS;

  OS_INT_Enter();

  globalIrqMask = XDMAC->XDMAC_GIS;

  /* Pending IRQ cleared by reading XDMAC_CISx */
  spiStatus  = (XDMAC->XDMAC_CHID[DMA_SPI0_TX_CH].XDMAC_CIS & dmaErrMask);
  spiStatus |= (XDMAC->XDMAC_CHID[DMA_SPI0_RX_CH].XDMAC_CIS & dmaErrMask);
  if (spiStatus != 0)
  {
    err_report(DMA_ERROR);
    __BKPT();
  }

  /* Signal task */
  OS_EVENT_SetMask(&dmaEvt, globalIrqMask);

  OS_INT_Leave();
}
