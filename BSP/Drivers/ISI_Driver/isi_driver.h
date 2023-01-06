/** @file */

#ifndef ISI_DRIVER_H
#define ISI_DRIVER_H

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include <stdint.h>

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/
/* Return codes */
#define ISI_OK                        (0)
#define ISI_INVALID_RESOLUTION        (-1)
#define ISI_INVALID_PIXEL_FORMAT      (-2)
#define ISI_INVALID_PARAMETER         (-3)

/* Supported pixel formats */
#define PF_YUV                        (0)
#define PF_RGB565                     (1)
#define PF_RGB888                     (2)
#define PF_GRAYSCALE                  (3)
#define PF_UNSUPPORTED                (4)

/* Maximum resolution */
#define RESOLUTION_HORIZONTAL_MAX     (2048)
#define RESOLUTION_VERTICAL_MAX       (2048)

/* DMA channels */
#define ISI_CHANNEL_PREVIEW           (0)
#define ISI_CHANNEL_CODEC             (1)
#define ISI_CHANNEL_COUNT             (2)

#define ISI_MODE_CONTINUOUS           (0)
#define ISI_MODE_SNAPSHOT             (1)


/*----------------------------------------------------------------------------
 *        Types
 *----------------------------------------------------------------------------*/
typedef struct
{
    uint32_t h; /**< Horizontal */
    uint32_t v; /**< Vertical   */
} Resolution_t; 

typedef void (*ISI_CB_t)(uint32_t ch);


/*----------------------------------------------------------------------------
 *         Exported functions
 *----------------------------------------------------------------------------*/
int32_t ISI_Init(Resolution_t Resolution, uint32_t PixelFormat);
void    ISI_Deinit(void);
void    ISI_IO_Init(void);
int32_t ISI_InstallCallback(ISI_CB_t cb);
void    ISI_Capture(uint32_t ch, uint8_t *buf);

#endif /* ISI_DRIVER_H */
