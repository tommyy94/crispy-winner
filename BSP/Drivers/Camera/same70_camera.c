/**
  ******************************************************************************
  * @file    stm32h747i_discovery_camera.c
  * @author  MCD Application Team
  * @brief   This file includes the driver for Camera modules mounted on
  *          STM32H747I_DISCO board.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* File Info: ------------------------------------------------------------------
                                   User NOTES
1. How to use this driver:
--------------------------
   - This driver is used to drive the camera.
   - The   or OV9655 component driver MUST be included with this driver.

2. Driver description:
---------------------
     o Initialize the camera instance using the BSP_CAMERA_Init() function with the required
       Resolution and Pixel format where:
       - Instance: Is the physical camera interface and is always 0 on this board.
       - Resolution: The camera resolution
       - PixelFormat: The camera Pixel format

     o DeInitialize the camera instance using the BSP_CAMERA_Init() . This
       function will firstly stop the camera to insure the data transfer complete.
       Then de-initializes the dcmi peripheral.

     o Get the camera instance capabilities using the BSP_CAMERA_GetCapabilities().
       This function must be called after the BSP_CAMERA_Init() to get the right
       sensor capabilities

     o Start the camera using the CAMERA_Start() function by specifying the capture Mode:
       - CAMERA_MODE_CONTINUOUS: For continuous capture
       - CAMERA_MODE_SNAPSHOT  : For on shot capture

     o Suspend, resume or stop the camera capture using the following functions:
      - BSP_CAMERA_Suspend()
      - BSP_CAMERA_Resume()
      - BSP_CAMERA_Stop()

     o Call BSP_CAMERA_SetResolution()/BSP_CAMERA_GetResolution() to set/get the camera resolution
       Resolution: - CAMERA_R160x120
                   - CAMERA_R320x240
                   - CAMERA_R480x272
                   - CAMERA_R640x480
                   - CAMERA_R800x480

     o Call BSP_CAMERA_SetPixelFormat()/BSP_CAMERA_GetPixelFormat() to set/get the camera pixel format
       PixelFormat: - CAMERA_PF_RGB565
                    - CAMERA_PF_RGB888
                    - CAMERA_PF_YUV422

     o Call BSP_CAMERA_SetLightMode()/BSP_CAMERA_GetLightMode() to set/get the camera light mode
       LightMode: - CAMERA_LIGHT_AUTO
                  - CAMERA_LIGHT_SUNNY
                  - CAMERA_LIGHT_OFFICE
                  - CAMERA_LIGHT_HOME
                  - CAMERA_LIGHT_CLOUDY

     o Call BSP_CAMERA_SetColorEffect()/BSP_CAMERA_GetColorEffect() to set/get the camera color effects
       Effect: - CAMERA_COLOR_EFFECT_NONE
               - CAMERA_COLOR_EFFECT_BLUE
               - CAMERA_COLOR_EFFECT_RED
               - CAMERA_COLOR_EFFECT_GREEN
               - CAMERA_COLOR_EFFECT_BW
               - CAMERA_COLOR_EFFECT_SEPIA
               - CAMERA_COLOR_EFFECT_NEGATIVE

     o Call BSP_CAMERA_SetBrightness()/BSP_CAMERA_GetBrightness() to set/get the camera Brightness
       Brightness is value between -4(Level 4 negative) and 4(Level 4 positive).

     o Call BSP_CAMERA_SetSaturation()/BSP_CAMERA_GetSaturation() to set/get the camera Saturation
       Saturation is value between -4(Level 4 negative) and 4(Level 4 positive).

     o Call BSP_CAMERA_SetContrast()/BSP_CAMERA_GetContrast() to set/get the camera Contrast
       Contrast is value between -4(Level 4 negative) and 4(Level 4 positive).

     o Call BSP_CAMERA_SetHueDegree()/BSP_CAMERA_GetHueDegree() to set/get the camera Hue Degree
       HueDegree is value between -4(180 degree negative) and 4(150 degree positive).

     o Call BSP_CAMERA_SetMirrorFlip()/BSP_CAMERA_GetMirrorFlip() to set/get the camera mirror and flip
       MirrorFlip could be any combination of: - CAMERA_MIRRORFLIP_NONE
                                               - CAMERA_MIRRORFLIP_FLIP
                                               - CAMERA_MIRRORFLIP_MIRROR
       Note that This feature is only supported with   sensor.

     o Call BSP_CAMERA_SetZoom()/BSP_CAMERA_GetZoom() to set/get the camera zooming
       Zoom is supported only with   sensor could be any value of:
       - CAMERA_ZOOM_x8 for CAMERA_R160x120 resolution only
       - CAMERA_ZOOM_x4 For all resolutions except CAMERA_R640x480 and CAMERA_R800x480
       - CAMERA_ZOOM_x2 For all resolutions except CAMERA_R800x480
       - CAMERA_ZOOM_x1 For all resolutions

     o Call BSP_CAMERA_EnableNightMode() to enable night mode. This feature is only supported
       with   sensor

     o Call BSP_CAMERA_DisableNightMode() to disable night mode. This feature is only supported
       with   sensor

    o Call BSP_CAMERA_RegisterDefaultMspCallbacks() to register Msp default callbacks
    o Call BSP_CAMERA_RegisterMspCallbacks() to register application Msp callbacks.

    o Error, line event, vsync event and frame event are handled through dedicated weak
      callbacks that can be override at application level: BSP_CAMERA_LineEventCallback(),
      BSP_CAMERA_FrameEventCallback(), BSP_CAMERA_VsyncEventCallback(), BSP_CAMERA_ErrorCallback()

  Known Limitations:
  ------------------
   1- CAMERA_PF_RGB888 resolution is not supported with   sensor.
   2- The following feature are only supported through   sensor:
      o LightMode setting
      o Saturation setting
      o HueDegree setting
      o Mirror/Flip setting
      o Zoom setting
      o NightMode enable/disable
------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "same70_camera.h"
#include "ov5640_i2c_layer.h"
#include "same70.h"
#include "isi_driver.h"
#include "io.h"
#include "pmc_driver.h"
#include "delay.h"
#include "trace.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H747I_DISCO
  * @{
  */

/** @addtogroup STM32H747I_DISCO_CAMERA
  * @{
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Exported_Variables Exported Variables
  * @{
  */
void                *Camera_CompObj = NULL;
CAMERA_Ctx_t        Camera_Ctx[CAMERA_INSTANCES_NBR];
/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Private_Variables Private Variables
  * @{
  */
static CAMERA_Drv_t *Camera_Drv = NULL;
static CAMERA_Capabilities_t Camera_Cap;
static uint32_t CameraId;
/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Private_FunctionsPrototypes Private Functions Prototypes
  * @{
  */
static int32_t GetSize(uint32_t Resolution, uint32_t PixelFormat);
static int32_t BSP_CAMERA_ConfigureIO(void);


#if (USE_CAMERA_SENSOR_OV5640 == 1)
static int32_t OV5640_Probe(uint32_t Resolution, uint32_t PixelFormat);
#endif
/**
  * @}
  */
#define CAMERA_VGA_RES_X          640
#define CAMERA_VGA_RES_Y          480
#define CAMERA_480x272_RES_X      480
#define CAMERA_480x272_RES_Y      272


#define OV5640_RST_PORT   (PIOA)
#define OV5640_PCK_PORT   (PIOA)
#define OV5640_PWD_PORT   (PIOC)
#define OV5640_RST_PIN    (1u << 13)
#define OV5640_PWD_PIN    (1u << 19)
#define OV5640_XCLK_PIN   (1u << 6)


#define BSP_ERROR_NONE                         0
#define BSP_ERROR_NO_INIT                     -1
#define BSP_ERROR_WRONG_PARAM                 -2
#define BSP_ERROR_BUSY                        -3
#define BSP_ERROR_PERIPH_FAILURE              -4
#define BSP_ERROR_COMPONENT_FAILURE           -5
#define BSP_ERROR_UNKNOWN_FAILURE             -6
#define BSP_ERROR_UNKNOWN_COMPONENT           -7
#define BSP_ERROR_BUS_FAILURE                 -8
#define BSP_ERROR_CLOCK_FAILURE               -9
#define BSP_ERROR_MSP_FAILURE                 -10
#define BSP_ERROR_FEATURE_NOT_SUPPORTED       -11

/** @defgroup STM32H747I_DISCO_CAMERA_Exported_Functions Exported Functions
  * @{
  */


/**
 * Configure OV5640 I/O pins.
 *
 * @retval  OV5640_OK
 */
static int32_t BSP_CAMERA_ConfigureIO(void)
{
    IO_ConfigureOutput(OV5640_PWD_PORT, OV5640_PWD_PIN, OV5640_PWD_PIN);
    IO_ConfigureOutput(OV5640_RST_PORT, OV5640_RST_PIN, OV5640_RST_PIN);
    IO_SetPeripheralFunction(OV5640_PCK_PORT, OV5640_XCLK_PIN, IO_PERIPH_B);

    /* Configure the host controller to feed clock input to OV5640
     * MCK (System Clock) = 150 MHz
     * PCK0 = 150MHz / 6 = 25MHz
     */
    PMC_ProgrammableClockInit(0, 5, PCK_SOURCE_MCK_CLK);
    PMC_ProgrammableClockEnable(0);

    return OV5640_OK;
}


/**
  * @brief  Initializes the camera.
  * @param  Instance    Camera instance.
  * @param  Resolution  Camera sensor requested resolution (x, y) : standard resolution
  *         naming QQVGA, QVGA, VGA ...
  * @param  PixelFormat Capture pixel format
  * @retval BSP status
  */
int32_t BSP_CAMERA_Init(uint32_t Instance, uint32_t Resolution, uint32_t PixelFormat)
{

  Resolution_t  res;
  uint32_t      pf;
  uint32_t      pfTbl[] =
  {
      PF_RGB565, PF_RGB888, PF_YUV, PF_GRAYSCALE
  };
  Resolution_t  resTbl[] = 
  {
      { 160, 120 }, /* OV5640_R160x120 */
      { 320, 240 }, /* OV5640_R320x240 */
      { 480, 272 }, /* OV5640_R480x272 */
      { 640, 480 }, /* OV5640_R640x480 */
      { 800, 480 }  /* OV5640_R800x480 */
  };

  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Validity check */
    if (Resolution > OV5640_R640x480)
    {
      res.h = 0xFFFFFFFF;
      res.v = 0xFFFFFFFF;
    }
    else
    {
      res = resTbl[Resolution];
    }

    /* Validity check */
    if (PixelFormat >= PF_UNSUPPORTED)
    {
      pf = PF_UNSUPPORTED;
    }
    else
    {
      pf = pfTbl[PixelFormat];
    }

    ret = ISI_Init(res, pf);
    if (ret != ISI_OK)
    {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    /*
    if (ISI_InstallCallback(NULL) != ISI_OK)
    {
        return BSP_ERROR_WRONG_PARAM;
    }
    */

    /* ST eval board probably has this deasserted */
    IO_SetOutput(OV5640_RST_PORT, OV5640_RST_PIN);

    if(BSP_CAMERA_HwReset(CAMERA_INSTANCE) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_BUS_FAILURE;
    }
    else
    {
#if (USE_CAMERA_SENSOR_OV5640 == 1)
      //if(ret != BSP_ERROR_NONE) /* Why? */
      if(ret == BSP_ERROR_NONE)
      {
        ret = OV5640_Probe(Resolution, PixelFormat);
      }
#endif
      if(ret != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_UNKNOWN_COMPONENT;
      }
      else
      {
        if(BSP_CAMERA_HwReset(CAMERA_INSTANCE) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_BUS_FAILURE;
        }

        if(ret == BSP_ERROR_NONE)
        {
          Camera_Ctx[Instance].CameraId    = CameraId;
          Camera_Ctx[Instance].Resolution  = Resolution;
          Camera_Ctx[Instance].PixelFormat = PixelFormat;
        }
      }
    }
  }

  /* BSP status */
  return ret;
}
/**
  * @brief  DeInitializes the camera.
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_DeInit(uint32_t Instance)
{
    (void)Instance;
    return 0;
}


/**
  * @brief  Starts the camera capture in continuous mode.
  * @param  Instance Camera instance.
  * @param  pBff     pointer to the camera output buffer
  * @param  Mode CAMERA_MODE_CONTINUOUS or CAMERA_MODE_SNAPSHOT
  * @retval BSP status
  */
int32_t BSP_CAMERA_Start(uint32_t Instance, uint8_t *pBff, uint32_t Mode)
{
    (void)Mode;

    if(Instance >= CAMERA_INSTANCES_NBR)
    {
      return BSP_ERROR_WRONG_PARAM;
    }

    if (Mode == CAMERA_MODE_CONTINUOUS)
    {
        TRACE_INFO("CAMERA  > Continuous mode not supported!");
        return BSP_ERROR_FEATURE_NOT_SUPPORTED;
    }

    ISI_Capture(ISI_CHANNEL_PREVIEW, pBff);

    return BSP_ERROR_NONE;
}

/**
  * @brief  Stop the CAMERA capture
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_Stop(uint32_t Instance)
{
    (void)Instance;
    return 0;
}

/**
  * @brief Suspend the CAMERA capture
  * @param  Instance Camera instance.
  */
int32_t BSP_CAMERA_Suspend(uint32_t Instance)
{
    (void)Instance;
    return 0;
}

/**
  * @brief Resume the CAMERA capture
  * @param  Instance Camera instance.
  */
int32_t BSP_CAMERA_Resume(uint32_t Instance)
{
    (void)Instance;
    return 0;
}

/**
  * @brief  Get the Camera Capabilities.
  * @param  Instance  Camera instance.
  * @param  Capabilities  pointer to camera Capabilities
  * @note   This function should be called after the init. This to get Capabilities
  *         from the right camera sensor(OV9655)
  * @retval Component status
  */
int32_t BSP_CAMERA_GetCapabilities(uint32_t Instance, CAMERA_Capabilities_t *Capabilities)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Drv->GetCapabilities(Camera_CompObj, Capabilities) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera pixel format.
  * @param  Instance  Camera instance.
  * @param  PixelFormat pixel format to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetPixelFormat(uint32_t Instance, uint32_t PixelFormat)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Drv->SetPixelFormat(Camera_CompObj, PixelFormat) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].PixelFormat = PixelFormat;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera pixel format.
  * @param  Instance  Camera instance.
  * @param  PixelFormat pixel format to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetPixelFormat(uint32_t Instance, uint32_t *PixelFormat)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    *PixelFormat = Camera_Ctx[Instance].PixelFormat;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}


/**
  * @brief  Set the camera Resolution.
  * @param  Instance  Camera instance.
  * @param  Resolution Resolution to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetResolution(uint32_t Instance, uint32_t Resolution)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Resolution == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetResolution(Camera_CompObj, Resolution) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Resolution = Resolution;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Resolution.
  * @param  Instance  Camera instance.
  * @param  Resolution Resolution to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetResolution(uint32_t Instance, uint32_t *Resolution)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Resolution == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Resolution = Camera_Ctx[Instance].Resolution;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Light Mode.
  * @param  Instance  Camera instance.
  * @param  LightMode Light Mode to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetLightMode(uint32_t Instance, uint32_t LightMode)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.LightMode == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetLightMode(Camera_CompObj, LightMode) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].LightMode = LightMode;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Light Mode.
  * @param  Instance  Camera instance.
  * @param  LightMode Light Mode to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetLightMode(uint32_t Instance, uint32_t *LightMode)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.LightMode == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *LightMode = Camera_Ctx[Instance].LightMode;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Special Effect.
  * @param  Instance Camera instance.
  * @param  ColorEffect Effect to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetColorEffect(uint32_t Instance, uint32_t ColorEffect)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.ColorEffect == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetColorEffect(Camera_CompObj, ColorEffect) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].ColorEffect = ColorEffect;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Special Effect.
  * @param  Instance Camera instance.
  * @param  ColorEffect Effect to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetColorEffect(uint32_t Instance, uint32_t *ColorEffect)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.ColorEffect == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *ColorEffect = Camera_Ctx[Instance].ColorEffect;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Brightness Level.
  * @param  Instance   Camera instance.
  * @param  Brightness Brightness Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetBrightness(uint32_t Instance, int32_t Brightness)
{
  int32_t ret;

  if((Instance >= CAMERA_INSTANCES_NBR) || ((Brightness < CAMERA_BRIGHTNESS_MIN) && (Brightness > CAMERA_BRIGHTNESS_MAX)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Brightness == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetBrightness(Camera_CompObj, Brightness) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Brightness = Brightness;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Brightness Level.
  * @param  Instance Camera instance.
  * @param  Brightness  Brightness Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetBrightness(uint32_t Instance, int32_t *Brightness)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Brightness == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Brightness = Camera_Ctx[Instance].Brightness;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Saturation Level.
  * @param  Instance    Camera instance.
  * @param  Saturation  Saturation Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetSaturation(uint32_t Instance, int32_t Saturation)
{
  int32_t ret;

  if((Instance >= CAMERA_INSTANCES_NBR) || ((Saturation < CAMERA_SATURATION_MIN) && (Saturation > CAMERA_SATURATION_MAX)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Saturation == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetSaturation(Camera_CompObj, Saturation)  < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Saturation = Saturation;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Saturation Level.
  * @param  Instance    Camera instance.
  * @param  Saturation  Saturation Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetSaturation(uint32_t Instance, int32_t *Saturation)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Saturation == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Saturation = Camera_Ctx[Instance].Saturation;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Contrast Level.
  * @param  Instance Camera instance.
  * @param  Contrast Contrast Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetContrast(uint32_t Instance, int32_t Contrast)
{
  int32_t ret;

  if((Instance >= CAMERA_INSTANCES_NBR) || ((Contrast < CAMERA_CONTRAST_MIN) && (Contrast > CAMERA_CONTRAST_MAX)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Contrast == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetContrast(Camera_CompObj, Contrast)  < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Contrast = Contrast;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Contrast Level.
  * @param  Instance Camera instance.
  * @param  Contrast Contrast Level
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetContrast(uint32_t Instance, int32_t *Contrast)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Contrast == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Contrast = Camera_Ctx[Instance].Contrast;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Hue Degree.
  * @param  Instance   Camera instance.
  * @param  HueDegree  Hue Degree
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetHueDegree(uint32_t Instance, int32_t HueDegree)
{
  int32_t ret;

  if((Instance >= CAMERA_INSTANCES_NBR) || ((HueDegree < CAMERA_HUEDEGREE_MIN) && (HueDegree > CAMERA_HUEDEGREE_MAX)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.HueDegree == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->SetHueDegree(Camera_CompObj, HueDegree) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].HueDegree = HueDegree;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Hue Degree.
  * @param  Instance   Camera instance.
  * @param  HueDegree  Hue Degree
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetHueDegree(uint32_t Instance, int32_t *HueDegree)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.HueDegree == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *HueDegree = Camera_Ctx[Instance].HueDegree;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera Mirror/Flip.
  * @param  Instance  Camera instance.
  * @param  MirrorFlip CAMERA_MIRRORFLIP_NONE or any combination of
  *                    CAMERA_MIRRORFLIP_FLIP and CAMERA_MIRRORFLIP_MIRROR
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetMirrorFlip(uint32_t Instance, uint32_t MirrorFlip)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.MirrorFlip == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->MirrorFlipConfig(Camera_CompObj, MirrorFlip)  < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].MirrorFlip = MirrorFlip;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera Mirror/Flip.
  * @param  Instance   Camera instance.
  * @param  MirrorFlip Mirror/Flip config
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetMirrorFlip(uint32_t Instance, uint32_t *MirrorFlip)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.MirrorFlip == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *MirrorFlip = Camera_Ctx[Instance].MirrorFlip;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the camera zoom
  * @param  Instance Camera instance.
  * @param  Zoom     Zoom to be configured
  * @retval BSP status
  */
int32_t BSP_CAMERA_SetZoom(uint32_t Instance, uint32_t Zoom)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Zoom == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->ZoomConfig(Camera_CompObj, Zoom) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    Camera_Ctx[Instance].Zoom = Zoom;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the camera zoom
  * @param  Instance Camera instance.
  * @param  Zoom     Zoom to be returned
  * @retval BSP status
  */
int32_t BSP_CAMERA_GetZoom(uint32_t Instance, uint32_t *Zoom)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.Zoom == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else
  {
    *Zoom = Camera_Ctx[Instance].Zoom;
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Enable the camera night mode
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_EnableNightMode(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.NightMode == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->NightModeConfig(Camera_CompObj, CAMERA_NIGHT_MODE_SET) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Disable the camera night mode
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_DisableNightMode(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Camera_Cap.NightMode == 0U)
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if(Camera_Drv->NightModeConfig(Camera_CompObj, CAMERA_NIGHT_MODE_RESET) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  CAMERA hardware reset
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_HwReset(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* De-assert the camera POWER_DOWN pin (active high) */
    IO_SetOutput(OV5640_PWD_PORT, OV5640_PWD_PIN);
    /* POWER_DOWN de-asserted during 100 ms */
    delayMs(100);

    /* Assert the camera POWER_DOWN pin (active high) */
    IO_ClearOutput(OV5640_PWD_PORT, OV5640_PWD_PIN);
    delayMs(20);
  }

  return ret;
}

/**
  * @brief  CAMERA power down
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_PwrDown(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
      /* De-assert the camera POWER_DOWN pin (active high) */
    IO_SetOutput(OV5640_PWD_PORT, OV5640_PWD_PIN);
  }

  return ret;
}

/**
  * @brief  CAMERA power up
  * @param  Instance Camera instance.
  * @retval BSP status
  */
int32_t BSP_CAMERA_PwrUp(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= CAMERA_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Assert the camera POWER_DOWN pin (active high) */
    IO_ClearOutput(OV5640_PWD_PORT, OV5640_PWD_PIN);
  }

  return ret;
}
/**
  * @brief  This function handles DCMI interrupt request.
  * @param  Instance Camera instance
  * @retval None
  */
void BSP_CAMERA_IRQHandler(uint32_t Instance)
{
    (void)Instance;
}

/**
  * @brief  This function handles DCMI DMA interrupt request.
  * @param  Instance Camera instance
  * @retval None
  */
void BSP_CAMERA_DMA_IRQHandler(uint32_t Instance)
{
    (void)Instance;
}

/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Private_Functions Private Functions
  * @{
  */

/**
  * @brief  Get the capture size in pixels unit.
  * @param  Resolution  the current resolution.
  * @param  PixelFormat Camera pixel format
  * @retval capture size in 32-bit words.
  */
static int32_t GetSize(uint32_t Resolution, uint32_t PixelFormat)
{
  uint32_t size = 0;
  uint32_t pf_div;
  if(PixelFormat == CAMERA_PF_RGB888)
  {
    pf_div = 3; /* each pixel on 3 bytes so 3/4 words */
  }
  else
  {
    pf_div = 2; /* each pixel on 2 bytes so 1/2 words*/
  }
  /* Get capture size */
  switch (Resolution)
  {
  case CAMERA_R160x120:
    size =  ((uint32_t)(160*120)*pf_div)/4U;
    break;
  case CAMERA_R320x240:
    size =  ((uint32_t)(320*240)*pf_div)/4U;
    break;
  case CAMERA_R480x272:
    size =  ((uint32_t)(480*272)*pf_div)/4U;
    break;
  case CAMERA_R640x480:
    size =  ((uint32_t)(640*480)*pf_div)/4U;
    break;
  case CAMERA_R800x480:
    size =  ((uint32_t)(800*480)*pf_div)/4U;
    break;
  default:
    break;
  }

  return (int32_t)size;
}


/**
  * @brief  Line Event callback.
  * @param  Instance Camera instance.
  * @retval None
  */
 void BSP_CAMERA_LineEventCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  (void)Instance;

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_LineEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Frame Event callback.
  * @param  Instance Camera instance.
  * @retval None
  */
 void BSP_CAMERA_FrameEventCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  (void)Instance;

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_FrameEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Vsync Event callback.
  * @param  Instance Camera instance.
  * @retval None
  */
 void BSP_CAMERA_VsyncEventCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  (void)Instance;

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_FrameEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Error callback.
  * @param  Instance Camera instance.
  * @retval None
  */
 void BSP_CAMERA_ErrorCallback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  (void)Instance;

  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_DCMI_ErrorCallback could be implemented in the user file
   */
}

/**
  * @}
  */

/** @defgroup STM32H747I_DISCO_CAMERA_Private_Functions Private Functions
  * @{
  */

#if (USE_CAMERA_SENSOR_OV5640 == 1)

/**
  * @brief  Register Bus IOs if component ID is OK
  * @retval error status
  */
static int32_t OV5640_Probe(uint32_t Resolution, uint32_t PixelFormat)
{
    uint32_t          id;
    OV5640_Object_t   ov5640_obj;
    OV5640_IO_t       ov5640_io;

    ov5640_io.Address   = CAMERA_OV5640_ADDRESS; 
    ov5640_io.Init      = BSP_CAMERA_ConfigureIO;
    ov5640_io.DeInit    = OV5640_I2C_Stub;
    ov5640_io.GetTick   = OV5640_I2C_Stub;
    ov5640_io.ReadReg   = OV5640_ReadReg;
    ov5640_io.WriteReg  = OV5640_WriteReg;

    ov5640_obj.IsInitialized = 0U;

    TRACE_INFO("CAMERA  > Initializing camera...");

    if (OV5640_RegisterBusIO(&ov5640_obj, &ov5640_io) != OV5640_OK)
    {
        TRACE_INFO("CAMERA  > OV5640_RegisterBusIO() fail");
        return BSP_ERROR_NO_INIT;
    }

    if (OV5640_ReadID(&ov5640_obj, &id) != OV5640_OK)
    {
        TRACE_INFO("CAMERA  > OV5640_ReadID() fail");
        return BSP_ERROR_NO_INIT;
    }
    
    if (id != OV5640_ID)
    {
        TRACE_INFO("CAMERA  > Invalid camera ID");
        return BSP_ERROR_NO_INIT;
    }
    else
    {
        Camera_Drv = (CAMERA_Drv_t *)&OV5640_CAMERA_Driver;
        Camera_CompObj = &ov5640_obj;
        if(Camera_Drv->Init(Camera_CompObj, Resolution, PixelFormat) != OV5640_OK)
        {
            TRACE_INFO("CAMERA  > Camera_Drv->Init() fail");
            return BSP_ERROR_NO_INIT;
        }
        else if(Camera_Drv->GetCapabilities(Camera_CompObj, &Camera_Cap) != OV5640_OK)
        {
            TRACE_INFO("CAMERA  > Camera_Drv->GetCapabilities() fail");
            return BSP_ERROR_NO_INIT;
        }
        else
        {
            TRACE_INFO("CAMERA  > Camera init success");
        }
    }

    return BSP_ERROR_NONE;
}
#endif


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
