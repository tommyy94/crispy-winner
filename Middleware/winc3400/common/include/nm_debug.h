/**
 *
 * \file
 *
 * \brief This module contains debug APIs declarations.
 *
 * Copyright (c) 2017-2019 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#ifndef _NM_DEBUG_H_
#define _NM_DEBUG_H_

#include "bsp/include/nm_bsp.h"
#include "bsp/include/nm_bsp_internal.h"

#define M2M_LOG_NONE									0
#define M2M_LOG_ERROR									1
#define M2M_LOG_INFO									2
#define M2M_LOG_REQ										3
#define M2M_LOG_DBG										4

#if (defined __APP_APS3_CORTUS__)
#define M2M_LOG_LEVEL									M2M_LOG_ERROR
#else
#define M2M_LOG_LEVEL									M2M_LOG_REQ
#endif

/**/
#if !((defined __MSP430FR5739)||(defined __MCF964548__))

#define M2M_ERR(...)
#define M2M_INFO(...)
#define M2M_REQ(...)
#define M2M_DBG(...)

#if (CONF_WINC_DEBUG == 1)
#define M2M_PRINT(...)							do{CONF_WINC_PRINTF(__VA_ARGS__);}while(0)
#if (M2M_LOG_LEVEL >= M2M_LOG_ERROR)
#undef M2M_ERR
#define M2M_ERR(...)							do{CONF_WINC_PRINTF("WINC Error > [%s] [%d]",__FUNCTION__,__LINE__); CONF_WINC_PRINTF(__VA_ARGS__);}while(0)
#if (M2M_LOG_LEVEL >= M2M_LOG_INFO)
#undef M2M_INFO
#define M2M_INFO(...)							do{CONF_WINC_PRINTF("WINC Info > "); CONF_WINC_PRINTF(__VA_ARGS__);}while(0)
#if (M2M_LOG_LEVEL >= M2M_LOG_REQ)
#undef M2M_REQ
#define M2M_REQ(...)							do{CONF_WINC_PRINTF("WINC Request > "); CONF_WINC_PRINTF(__VA_ARGS__);}while(0)
#if (M2M_LOG_LEVEL >= M2M_LOG_DBG)
#undef M2M_DBG
#define M2M_DBG(...)							do{CONF_WINC_PRINTF("WINC Debug > [%s] [%d]",__FUNCTION__,__LINE__); CONF_WINC_PRINTF(__VA_ARGS__);}while(0)
#endif
#endif
#endif
#endif
#else
#define M2M_ERR(...)
#define M2M_DBG(...)
#define M2M_REQ(...)
#define M2M_INFO(...)
#define M2M_PRINT(...)
#endif
#else
#if (!defined  __MCF964548__)||(!defined __SAMD21J18A__)
static void M2M_ERR(const char *_format, ...) //__attribute__ ((__format__ (M2M_ERR, 1, 2)))
{
}
static void M2M_DBG(const char *_format, ...) //__attribute__ ((__format__ (M2M_DBG, 1, 2)))
{
}
static void M2M_REQ(const char *_format, ...) //__attribute__ ((__format__ (M2M_DBG, 1, 2)))
{
}
static void M2M_INFO(const char *_format, ...) // __attribute__ ((__format__ (M2M_INFO, 1, 2)))
{

}
static void M2M_PRINT(const char *_format, ...) // __attribute__ ((__format__ (M2M_INFO, 1, 2)))
{

}
static void CONF_WINC_PRINTF(const char *_format, ...)
{
}
#endif
#endif

#endif /* _NM_DEBUG_H_ */
