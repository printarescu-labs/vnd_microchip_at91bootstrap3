/* ----------------------------------------------------------------------------
 *         Printarescu Labs
 * ----------------------------------------------------------------------------
 * Copyright (c) 2006, Microchip Technology Inc. and its subsidiaries
 * Copyright (c) 2025, Printarescu Labs
 *
 * Based on at91sam9260ek (AT91SAM9260-EK) board support code.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Microchip's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY WHATCLOUD "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL WHATCLOUD BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __GLASNOST_M9260_H__
#define __GLASNOST_M9260_H__

/*
 * PMC Settings
 *
 * The main oscillator is enabled as soon as possible in the c_startup
 * and MCK is switched on the main oscillator.
 * PLL initialization is done later in the hw_init() function
 *
 * Same crystal/clock configuration as the at91sam9260ek reference design.
 */
#define MASTER_CLOCK		(198656000/2)

#define PLLA_SETTINGS		0x2060BF09

/* Switch MCK on PLLA output PCK = PLLA = 2 * MCK */
#define MCKR_SETTINGS		((0 << 2) | AT91C_PMC_MDIV_2)
#define MCKR_CSS_SETTINGS	(AT91C_PMC_CSS_PLLA_CLK | MCKR_SETTINGS)

/*
 * DataFlash Settings
 */
#define CONFIG_SYS_SPI_CLOCK	AT91C_SPI_CLK
#define CONFIG_SYS_SPI_MODE	SPI_MODE0

#if defined(CONFIG_SPI_BUS0)
#define CONFIG_SYS_BASE_SPI	AT91C_BASE_SPI0
#elif defined(CONFIG_SPI_BUS1)
#define CONFIG_SYS_BASE_SPI	AT91C_BASE_SPI1
#endif

#if (AT91C_SPI_PCS_DATAFLASH == AT91C_SPI_PCS0_DATAFLASH)
#define CONFIG_SYS_SPI_PCS	AT91C_PIN_PA(3)
#elif (AT91C_SPI_PCS_DATAFLASH == AT91C_SPI_PCS1_DATAFLASH)
#define CONFIG_SYS_SPI_PCS	AT91C_PIN_PC(11)
#endif

/*
 * Recovery Button
 *
 * Confirmed against the board schematic ("module", sheet 2/5): the
 * RECOVERY net lands on PA26. ALLOW_DATAFLASH_RECOVERY is still left
 * unselected in Config.in.board -- enabling the feature is a separate
 * decision from confirming the pin.
 */
#define CONFIG_SYS_RECOVERY_BUTTON_PIN	AT91C_PIN_PA(26)
#define RECOVERY_BUTTON_NAME	"RECOVERY"

#endif /* #ifndef __GLASNOST_M9260_H__ */
