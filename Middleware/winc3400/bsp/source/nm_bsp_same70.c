/**
 *
 * \file
 *
 * \brief This module contains SAMD21 BSP APIs implementation.
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

#include "bsp/include/nm_bsp.h"
#include "common/include/nm_common.h"
#include "conf_winc.h"
#include "same70.h"
#include "RTOS.h"
#include "io.h"

#define WINC3400_IRQ_PORT   (PIOD)
#define WINC3400_PORT       (PIOD)
#define WINC3400_CE_PIN     (19u)  /* PORTD */
#define WINC3400_RST_PIN    (18u)  /* PORTD */
#define WINC3400_IRQ_PIN    (26u)  /* PORTD */


static IRQn_Type nmIrqn;


/*
 *	@fn		init_chip_pins
 *	@brief	Initialize reset and chip enable.
 */
static void init_chip_pins(void)
{
    const uint32_t outMask = IO_MASK(WINC3400_CE_PIN) | IO_MASK(WINC3400_RST_PIN);
    IO_ConfigureOutput(WINC3400_PORT, outMask, outMask);
    IO_ClearOutput(WINC3400_PORT, outMask);

    IO_ConfigurePull(WINC3400_PORT, IO_MASK(WINC3400_RST_PIN), IO_PULLUP);
}

/*
 *	@fn		nm_bsp_init
 *	@brief	Initialize BSP
 *	@return	0 in case of success and -1 in case of failure
 */
sint8 nm_bsp_init(void)
{
    /* Initialize chip IOs. */
    init_chip_pins();

    /* Perform chip reset. */
    nm_bsp_reset();

    return M2M_SUCCESS;
}

/**
 *	@fn		nm_bsp_deinit
 *	@brief	De-Initialize BSP
 *	@return	0 in case of success and -1 in case of failure
 */
sint8 nm_bsp_deinit(void)
{
    IO_ClearOutput(WINC3400_PORT, IO_MASK(WINC3400_CE_PIN) | IO_MASK(WINC3400_RST_PIN));
    nm_bsp_sleep(10);
    return M2M_SUCCESS;
}

/**
 *	@fn		nm_bsp_reset
 *	@brief	Reset NMC1500 SoC by setting CHIP_EN and RESET_N signals low,
 *           CHIP_EN high then RESET_N high
 */
void nm_bsp_reset(void)
{
    IO_ClearOutput(WINC3400_PORT, IO_MASK(WINC3400_CE_PIN) | IO_MASK(WINC3400_RST_PIN));
    nm_bsp_sleep(100);
    IO_SetOutput(WINC3400_PORT, IO_MASK(WINC3400_CE_PIN));
    nm_bsp_sleep(100);
    IO_SetOutput(WINC3400_PORT, IO_MASK(WINC3400_RST_PIN));
    nm_bsp_sleep(100);
}

/*
 *	@fn		nm_bsp_sleep
 *	@brief	Sleep in units of mSec
 *	@param[IN]	u32TimeMsec
 *				Time in milliseconds
 */
void nm_bsp_sleep(uint32 u32TimeMsec)
{
    if (u32TimeMsec < 10)
    {
        u32TimeMsec = 10;
    }
    OS_TASK_Delay(u32TimeMsec);
}

/*
 *	@fn		nm_bsp_register_isr
 *	@brief	Register interrupt service routine
 *	@param[IN]	pfIsr
 *				Pointer to ISR handler
 */
void nm_bsp_register_isr(tpfNmBspIsr pfIsr)
{
    IO_InstallIrqHandler(WINC3400_IRQ_PORT, WINC3400_IRQ_PIN, pfIsr);
    IO_ConfigurePull(WINC3400_IRQ_PORT, IO_MASK(WINC3400_IRQ_PIN), IO_PULLUP);
    nmIrqn = IO_ConfigureIRQ(WINC3400_IRQ_PORT, IO_SENSE_FALL, IO_MASK(WINC3400_IRQ_PIN));
}


/*
 *	@fn		nm_bsp_interrupt_ctrl
 *	@brief	Enable/Disable interrupts
 *	@param[IN]	u8Enable
 *				'0' disable interrupts. '1' enable interrupts
 */
void nm_bsp_interrupt_ctrl(uint8 u8Enable)
{
    if (u8Enable == 0)
    {
        NVIC_DisableIRQ(nmIrqn);
    }
    else
    {
        NVIC_EnableIRQ(nmIrqn);
    }
}
