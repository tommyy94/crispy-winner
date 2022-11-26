/**
 *
 * \file
 *
 * \brief This module contains NMC1000 bus wrapper APIs implementation.
 *
 * Copyright (c) 2016-2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>     /* Included for uint_t */

#include "bsp/include/nm_bsp.h"
#include "common/include/nm_common.h"
#include "bus_wrapper/include/nm_bus_wrapper.h"
#include "conf_winc.h"
#include "spi_driver.h"
#include "err.h"

#define NM_BUS_MAX_TRX_SZ	256


tstrNmBusCapabilities egstrNmBusCapabilities =
{
	NM_BUS_MAX_TRX_SZ
};

#ifdef CONF_WINC_USE_SPI

/*
*	@fn		spi_rw
*	@brief	transmit and/or receive data buffer via spi
*	@return	status
*/
static sint8 spi_rw(uint8* pu8Mosi, uint8* pu8Miso, uint16 u16Sz)
{
    bool      ret;
    sint8     status = M2M_ERR_BUS_FAIL;

    ret = SPI0_DMA_TransmitMessage(pu8Mosi, pu8Miso, u16Sz);
    if (ret == true)
    {
        //Journal_vWriteError(DMA_ERROR);
        status = M2M_SUCCESS;
    }
    return status;
}
#endif //CONF_WINC_USE_SPI

/*
*	@fn		nm_bus_init
*	@brief	Initialize the bus wrapper
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_bus_init(uint8 *pvinit, uint32 req_serial_number)
{
    static bool bus_init_done = false;

    (void)pvinit;
    (void)req_serial_number;

    if (bus_init_done == false)
    {
        SPI0_Init();
        bus_init_done = true;
    }
    return M2M_SUCCESS;
}


/*
*	@fn		nm_bus_ioctl
*	@brief	send/receive from the bus
*	@param[IN]	u8Cmd
*					IOCTL command for the operation
*	@param[IN]	pvParameter
*					Arbitrary parameter depenging on IOCTL
*	@return	M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*	@note	For SPI only, it's important to be able to send/receive at the same time
*/
sint8 nm_bus_ioctl(uint8 u8Cmd, void* pvParameter)
{
    tstrNmSpiRw *pstrParam;
    sint8 s8Ret = 0;
    switch(u8Cmd)
    {
        case NM_BUS_IOCTL_RW:
            pstrParam = (tstrNmSpiRw *)pvParameter;
            s8Ret = spi_rw(pstrParam->pu8InBuf, pstrParam->pu8OutBuf, pstrParam->u16Sz);
            break;
        default:
            s8Ret = -1;
            M2M_ERR("Invalid IOCTL cmd\n");
            break;
    }

    return s8Ret;
}

/*
*	@fn		nm_bus_deinit
*	@brief	De-initialize the bus wrapper
*/
sint8 nm_bus_deinit(void)
{
	return M2M_SUCCESS;
}

/*
*	@fn			nm_bus_reinit
*	@brief		re-initialize the bus wrapper
*	@param [in]	void *config
*					re-init configuration data
*	@return		M2M_SUCCESS in case of success and M2M_ERR_BUS_FAIL in case of failure
*/
sint8 nm_bus_reinit(void* config)
{
        (void)config;
	return M2M_SUCCESS;
}

