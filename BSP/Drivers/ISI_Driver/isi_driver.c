/** @file */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stddef.h>
#include "same70.h"
#include "isi_driver.h"
#include "pmc_driver.h"
#include "io.h"
#include "system.h"
#include "err.h"


/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/
#define ISI_MAX_BUFFER_DESC   (1)


/*----------------------------------------------------------------------------
 *        Local types
 *----------------------------------------------------------------------------*/
typedef struct
{
    uint32_t current;
    uint32_t control;
    uint32_t next;
} ISI_FBD_t;


/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/

ISI_FBD_t FrameBufferDescriptor[ISI_MAX_BUFFER_DESC];

ISI_CB_t ISI_CaptureDoneCallback;


/*----------------------------------------------------------------------------
 *        Function prototypes
 *----------------------------------------------------------------------------*/
extern void     ISI_IRQHandler(void);

static int32_t  ISI_SetResolution(Resolution_t resolution, bool grayScale8Bit);
static int32_t  ISI_SetPixelFormat(uint32_t pixelFormat);
static int32_t  ISI_ConfigurePreviewPath(Resolution_t res);
static void     ISI_Enable(void);
static void     ISI_Disable(void);
static void     ISI_SoftReset(void);
__STATIC_INLINE void ISI_EnableDMAChannel(uint32_t ch);
__STATIC_INLINE void ISI_DisableDMAChannel(uint32_t ch);
__STATIC_INLINE void ISI_EnableChannelInterrupt(uint32_t ch);
__STATIC_INLINE void ISI_DisableChannelInterrupt(uint32_t ch);


/*----------------------------------------------------------------------------
 *        Local functions
 *----------------------------------------------------------------------------*/
/**
 * Set resolution.
 *
 * @param[in] resolution    Supported values:
 *                              0-2048 in both horizontal and vertical
 *
 * @param[in] grayScale8Bit Set true if 8-bit Grayscale mode
 *                          is enabled.
 *
 * @retval    ISI_OK/ISI_INVALID_RESOLUTION
 */
static int32_t ISI_SetResolution(Resolution_t resolution, bool grayScale8Bit)
{
    if ((resolution.h > RESOLUTION_HORIZONTAL_MAX)
     || (resolution.v > RESOLUTION_VERTICAL_MAX))
    {
        return ISI_INVALID_RESOLUTION;
    }

    /* If 8-bit Grayscale mode is enabled,
     * IM_HSIZE = (hSize/2) - 1
     * else IM_HSIZE = hSize - 1
     */
    if (grayScale8Bit == true)
    {
        resolution.h >>= 1;
    }

    ISI->ISI_CFG2 &= ~(ISI_CFG2_IM_HSIZE_Msk | ISI_CFG2_IM_VSIZE_Msk);
    ISI->ISI_CFG2 |= ISI_CFG2_IM_HSIZE(resolution.h - 1)
                  |  ISI_CFG2_IM_VSIZE(resolution.v - 1);

    return ISI_OK;
}


/**
 * Set ISI pixel format.
 *
 * @param[in] pixelFormat   Supported values:
 *                              PF_YUV
 *                              PF_RGB565
 *                              PF_RGB888
 *                              PF_GRAYSCALE
 *
 * @retval    ISI_OK/ISI_INVALID_PIXEL_FORMAT
 */
static int32_t ISI_SetPixelFormat(uint32_t pixelFormat)
{
    switch (pixelFormat)
    {
        case PF_RGB565:
            /* 16-bit RGB */
            ISI->ISI_CFG2 |= ISI_CFG2_RGB_MODE | ISI_CFG2_COL_SPACE;
            break;
        case PF_RGB888:
            /* 24-bit RGB */
            ISI->ISI_CFG2 &= ~ISI_CFG2_RGB_MODE;
            ISI->ISI_CFG2 |=  ISI_CFG2_COL_SPACE;
            break;
        case PF_YUV:
            ISI->ISI_CFG2 &= ~ISI_CFG2_COL_SPACE;
            break;
        case PF_GRAYSCALE:
            ISI->ISI_CFG2 &= ~(ISI_CFG2_RGB_MODE | ISI_CFG2_COL_SPACE);
            ISI->ISI_CFG2 |= ISI_CFG2_GRAYSCALE;
            break;
        default:
            return ISI_INVALID_PIXEL_FORMAT;
            break;
    }

    return ISI_OK;
}


static int32_t ISI_ConfigurePreviewPath(Resolution_t res)
{
    uint32_t decFactor;

    ISI->ISI_PSIZE = ISI_PSIZE_PREV_HSIZE(res.h >> 1)
                   | ISI_PSIZE_PREV_VSIZE(res.v);

    decFactor = 1600 * res.h / (res.h >> 1) / 100;
    ISI->ISI_PDECF = ISI_PDECF_DEC_FACTOR(decFactor);

    return ISI_OK;
}


/**
 * Enable Image Sensor Interface.
 */
static void ISI_Enable(void)
{
    ISI->ISI_CR = ISI_CR_ISI_EN;
    while (!(ISI->ISI_SR & ISI_SR_ENABLE));
}


/**
 * Disable Image Sensor Interface.
 */
static void ISI_Disable(void)
{
    ISI->ISI_CR = ISI_CR_ISI_DIS;
    while (!(ISI->ISI_SR & ISI_SR_DIS_DONE));
}


/**
 * Request software reset of the ISI module.
 */
static void ISI_SoftReset(void)
{
    ISI->ISI_CR = ISI_CR_ISI_SRST;
}


/**
 * Enable ISI DMA channel.
 *
 * @param[in] ch    ISI_CHANNEL_PREVIEW or ISI_CHANNEL_CODEC
 */
__STATIC_INLINE void ISI_EnableDMAChannel(uint32_t ch)
{
    assert(ch <= ISI_CHANNEL_CODEC);

    ISI->ISI_DMA_CHER = 1 << ch;
}


/**
 * Disable ISI DMA channel.
 *
 * @param[in] ch    ISI_CHANNEL_PREVIEW or ISI_CHANNEL_CODEC
 */
__STATIC_INLINE void ISI_DisableDMAChannel(uint32_t ch)
{
    assert(ch <= ISI_CHANNEL_CODEC);

    ISI->ISI_DMA_CHDR = 1 << ch;
}


/**
 * Enable ISI channel interrupt.
 *
 * @param[in] ch    ISI_CHANNEL_PREVIEW or ISI_CHANNEL_CODEC
 */
__STATIC_INLINE void ISI_EnableChannelInterrupt(uint32_t ch)
{    
    assert((ch == ISI_CHANNEL_PREVIEW) || (ch == ISI_CHANNEL_CODEC));

    if (ch == ISI_CHANNEL_PREVIEW)
    {
        ISI->ISI_IER = ISI_IER_P_OVR | ISI_IER_PXFR_DONE;
    }
    else
    {
        ISI->ISI_IER = ISI_IER_C_OVR | ISI_IER_CXFR_DONE;
    }
}


/**
 * Disable ISI channel interrupt.
 *
 * @param[in] ch    ISI_CHANNEL_PREVIEW or ISI_CHANNEL_CODEC
 */
__STATIC_INLINE void ISI_DisableChannelInterrupt(uint32_t ch)
{    
    assert((ch == ISI_CHANNEL_PREVIEW) || (ch == ISI_CHANNEL_CODEC));

    if (ch == ISI_CHANNEL_PREVIEW)
    {
        ISI->ISI_IDR = ISI_IDR_P_OVR | ISI_IDR_PXFR_DONE;
    }
    else
    {
        ISI->ISI_IDR = ISI_IDR_C_OVR | ISI_IDR_CXFR_DONE;
    }
}


/*----------------------------------------------------------------------------
 *        Global functions
 *----------------------------------------------------------------------------*/

/**
 * Install ISI IRQ callback.
 *
 * @param[in] cb    Callback to install.
 *
 * @retval    ISI_OK/ISI_INVALID_PARAMETER
 */
int32_t ISI_InstallCallback(ISI_CB_t cb)
{
    if (cb != NULL)
    {
        ISI_CaptureDoneCallback = cb;
        return ISI_OK;
    }

    return ISI_INVALID_PARAMETER;
}


/**
 * ISI interrupt handler.
 */
void ISI_IRQHandler(void)
{
    uint32_t status;
    uint32_t mask;

    status = ISI->ISI_SR;
    mask = ISI->ISI_IMR;

    /* Preview path */
    if ((status & ISI_SR_PXFR_DONE) && (mask & ISI_IMR_PXFR_DONE))
    {
        if (ISI_CaptureDoneCallback != NULL)
        {
            ISI_DisableChannelInterrupt(ISI_CHANNEL_PREVIEW);
            ISI_DisableDMAChannel(ISI_CHANNEL_PREVIEW);
            ISI_CaptureDoneCallback(ISI_CHANNEL_PREVIEW);
        }
    }

    /* Codec path */
    if ((status & ISI_SR_CXFR_DONE) && (mask & ISI_IMR_CXFR_DONE))
    {
        if (ISI_CaptureDoneCallback != NULL)
        {
            ISI_DisableChannelInterrupt(ISI_CHANNEL_CODEC);
            ISI_DisableDMAChannel(ISI_CHANNEL_CODEC);
            ISI_CaptureDoneCallback(ISI_CHANNEL_CODEC);
        }
    }
}


void ISI_Capture(uint32_t ch, uint8_t *buf)
{
    assert((ch == ISI_CHANNEL_PREVIEW) || (ch == ISI_CHANNEL_CODEC));

    ISI_EnableDMAChannel(ch);

    FrameBufferDescriptor[ch].current = (uint32_t)buf;
    FrameBufferDescriptor[ch].control = ISI_DMA_P_CTRL_P_DONE;
    FrameBufferDescriptor[ch].next    = 0x0;

    /* Configure buffer descriptors */
    ISI->ISI_DMA_P_DSCR = ISI_DMA_P_DSCR_P_DSCR(FrameBufferDescriptor->current);
    ISI->ISI_DMA_P_CTRL = FrameBufferDescriptor[ch].control;
    ISI->ISI_DMA_P_ADDR = ISI_DMA_C_ADDR_C_ADDR((uint32_t)buf);

    ISI_EnableChannelInterrupt(ch);
}


/**
 * Initialize Image Sensor Interface.
 *
 * @param[in] resolution    Supported values:
 *                              0-2048 in both horizontal and vertical
 *
 * @param[in] pixelFormat   Supported values:
 *                              PF_YUV
 *                              PF_RGB565
 *                              PF_RGB888
 *                              PF_GRAYSCALE
 */
int32_t ISI_Init(Resolution_t resolution, uint32_t pixelFormat)
{
    int32_t ret;

    PMC_PeripheralClockEnable(ID_ISI);

    ISI_IO_Init();

    ret = ISI_SetResolution(resolution, false);
    if (ret != ISI_OK)
    {
        return ret;
    }

    ret = ISI_SetPixelFormat(pixelFormat);
    if (ret != ISI_OK)
    {
        return ret;
    }

    ret = ISI_ConfigurePreviewPath(resolution);
    if (ret != ISI_OK)
    {
        return ret;
    }

    NVIC_ClearPendingIRQ(ISI_IRQn);
    NVIC_SetPriority(ISI_IRQn, ISI_IRQ_PRIO);
    NVIC_EnableIRQ(ISI_IRQn);

    /* Keep error interrupts always enabled */
    ISI->ISI_IER = ISI_IER_FR_OVR | ISI_IER_CRC_ERR;

    ISI_Enable();

    return ISI_OK;
}


/**
 * Deinitialize Image Sensor Interface.
 */
void ISI_Deinit(void)
{
    ISI_Disable();
    ISI_SoftReset();
}


/**
 * Initialize Image Sensor Interface
 * IO pins.
 */
void ISI_IO_Init(void)
{
    uint32_t mask;

    /* Initialize data pins */
    mask = PIO_PA5B_ISI_D4 | PIO_PA9B_ISI_D3;
    IO_SetPeripheralFunction(PIOA, mask, IO_PERIPH_B);
    mask = PIO_PD28D_ISI_D9 | PIO_PD27D_ISI_D8
         | PIO_PD12D_ISI_D6 | PIO_PD11D_ISI_D5
         | PIO_PD21D_ISI_D1 | PIO_PD22D_ISI_D0;
    IO_SetPeripheralFunction(PIOD, mask, IO_PERIPH_D);
    IO_SetPeripheralFunction(PIOB, PIO_PB3D_ISI_D2, IO_PERIPH_D);

    /* Sync and clock signals */
    mask = PIO_PD25D_ISI_VSYNC | PIO_PD24D_ISI_HSYNC;
    IO_SetPeripheralFunction(PIOD, mask, IO_PERIPH_D);

    /* Clock signal */
    IO_SetPeripheralFunction(PIOA, PIO_PA24D_ISI_PCK, IO_PERIPH_D);
}
