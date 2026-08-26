/* ----------------------------------------------------------------------------
 *         Printarescu Labs
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 * Copyright (c) 2025, Printarescu Labs
 *
 * Based on at91sam9g20ek (AT91SAM9G20-EK) board support code.
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

#include "common.h"
#include "hardware.h"
#include "arch/at91_ccfg.h"
#include "arch/at91_matrix.h"
#include "arch/at91_rstc.h"
#include "arch/at91_pmc/pmc.h"
#include "arch/at91_smc.h"
#include "arch/at91_pio.h"
#include "arch/at91_sdramc.h"
#include "spi.h"
#include "gpio.h"
#include "pmc.h"
#include "usart.h"
#include "debug.h"
#include "sdramc.h"
#include "timer.h"
#include "watchdog.h"
#include "glasnost_m9g20.h"

static void initialize_dbgu(void)
{
	writel(((0x01 << 14) | (0x01 << 15)), AT91C_BASE_PIOB + PIO_ASR);
	writel(((0x01 << 14) | (0x01 << 15)), AT91C_BASE_PIOB + PIO_PDR);

	pmc_enable_periph_clock(AT91C_ID_PIOB, PMC_PERIPH_CLK_DIVIDER_NA);

	usart_init(BAUDRATE(MASTER_CLOCK, 115200));
}

#ifdef CONFIG_SDRAM
static void sdramc_init(void)
{
	struct sdramc_register sdramc_config;

	sdramc_config.cr = AT91C_SDRAMC_NC_9
		| AT91C_SDRAMC_NR_13 | AT91C_SDRAMC_CAS_3
		| AT91C_SDRAMC_NB_4_BANKS | AT91C_SDRAMC_DBW_32_BITS
		| AT91C_SDRAMC_TWR_3 | AT91C_SDRAMC_TRC_9
		| AT91C_SDRAMC_TRP_3 | AT91C_SDRAMC_TRCD_3
		| AT91C_SDRAMC_TRAS_6 | AT91C_SDRAMC_TXSR_10;

	sdramc_config.tr = (MASTER_CLOCK * 7) / 1000000;
	sdramc_config.mdr = AT91C_SDRAMC_MD_SDRAM;

	writel(0xFFFF0000, AT91C_BASE_PIOC + PIO_ASR);
	writel(0xFFFF0000, AT91C_BASE_PIOC + PIO_PDR);

	pmc_enable_periph_clock(AT91C_ID_PIOC, PMC_PERIPH_CLK_DIVIDER_NA);

	writel(readl(AT91C_BASE_CCFG + CCFG_EBICSA)
		| AT91C_EBI_CS1A_SDRAMC | AT91C_VDDIOM_SEL_33V
		| (0x01 << 17),
		AT91C_BASE_CCFG + CCFG_EBICSA);

	sdramc_initialize(&sdramc_config, AT91C_BASE_CS1);
}
#endif  /* #ifdef CONFIG_SDRAM */

#define LED_RED_MASK	(0x01 << 28)
#define LED_GREEN_MASK	(0x01 << 6)
#define LED_BLUE_MASK	(0x01 << 7)

static void leds_init(void)
{
	pmc_enable_periph_clock(AT91C_ID_PIOA, PMC_PERIPH_CLK_DIVIDER_NA);
	pmc_enable_periph_clock(AT91C_ID_PIOC, PMC_PERIPH_CLK_DIVIDER_NA);

	writel(LED_RED_MASK, AT91C_BASE_PIOA + PIO_SODR);
	writel(LED_RED_MASK, AT91C_BASE_PIOA + PIO_OER);
	writel(LED_RED_MASK, AT91C_BASE_PIOA + PIO_PER);

	writel(LED_GREEN_MASK | LED_BLUE_MASK, AT91C_BASE_PIOC + PIO_SODR);
	writel(LED_GREEN_MASK | LED_BLUE_MASK, AT91C_BASE_PIOC + PIO_OER);
	writel(LED_GREEN_MASK | LED_BLUE_MASK, AT91C_BASE_PIOC + PIO_PER);

	writel(LED_BLUE_MASK, AT91C_BASE_PIOC + PIO_CODR);
}

void board_boot_error_indication(void)
{
	writel(LED_RED_MASK, AT91C_BASE_PIOA + PIO_CODR);
	writel(LED_BLUE_MASK, AT91C_BASE_PIOC + PIO_CODR);
}

#ifdef CONFIG_HW_INIT
void hw_init(void)
{
	at91_disable_wdt();

	leds_init();

	pmc_init_pll(0);

	pmc_cfg_plla(PLLA_SETTINGS);

	pmc_mck_cfg_set(0, MCKR_SETTINGS, AT91C_PMC_PLLADIV2 | AT91C_PMC_MDIV);

	pmc_mck_cfg_set(0, MCKR_CSS_SETTINGS,
			AT91C_PMC_PLLADIV2 | AT91C_PMC_MDIV | AT91C_PMC_CSS);

	writel(AT91C_RSTC_KEY_UNLOCK | AT91C_RSTC_URSTEN, AT91C_BASE_RSTC + RSTC_RMR);

	writel((readl(AT91C_BASE_MATRIX + MATRIX_SCFG3) & ~0xFF) | 0x40,
			AT91C_BASE_MATRIX + MATRIX_SCFG3);

	timer_init();

	initialize_dbgu();

#ifdef CONFIG_SDRAM
	sdramc_init();
#endif
}
#endif /* #ifdef CONFIG_HW_INIT */

#ifdef CONFIG_DATAFLASH
void at91_spi0_hw_init(void)
{
	writel(((0x01 << 0) | (0x01 << 1) | (0x01 << 2)),
					AT91C_BASE_PIOA + PIO_ASR);
	writel(((0x01 << 0) | (0x01 << 1) | (0x01 << 2)),
					AT91C_BASE_PIOA + PIO_PDR);

#if (AT91C_SPI_PCS_DATAFLASH == AT91C_SPI_PCS0_DATAFLASH)
	writel((0x01 << 3), AT91C_BASE_PIOA + PIO_IDR);
	writel((0x01 << 3), AT91C_BASE_PIOA + PIO_PPUDR);
	writel((0x01 << 3), AT91C_BASE_PIOA + PIO_SODR);
	writel((0x01 << 3), AT91C_BASE_PIOA + PIO_OER);
	writel((0x01 << 3), AT91C_BASE_PIOA + PIO_PER);

	pmc_enable_periph_clock(AT91C_ID_PIOA, PMC_PERIPH_CLK_DIVIDER_NA);
#endif

#if (AT91C_SPI_PCS_DATAFLASH == AT91C_SPI_PCS1_DATAFLASH)
	writel((0x01 << 11), AT91C_BASE_PIOC + PIO_IDR);
	writel((0x01 << 11), AT91C_BASE_PIOC + PIO_PPUDR);
	writel((0x01 << 11), AT91C_BASE_PIOC + PIO_SODR);
	writel((0x01 << 11), AT91C_BASE_PIOC + PIO_OER);
	writel((0x01 << 11), AT91C_BASE_PIOC + PIO_PER);

	pmc_enable_periph_clock(AT91C_ID_PIOA, PMC_PERIPH_CLK_DIVIDER_NA);
	pmc_enable_periph_clock(AT91C_ID_PIOC, PMC_PERIPH_CLK_DIVIDER_NA);
#endif

	pmc_enable_periph_clock(AT91C_ID_SPI0, PMC_PERIPH_CLK_DIVIDER_NA);
}
#endif /* #ifdef CONFIG_DATAFLASH */

#ifdef CONFIG_SDCARD
void at91_mci0_hw_init(void)
{
	unsigned int mask = (0x01 << 6) | (0x01 << 7) | (0x01 << 8)
		| (0x01 << 9) | (0x01 << 10) | (0x01 << 11);

	writel(mask, AT91C_BASE_PIOA + PIO_ASR);
	writel(mask, AT91C_BASE_PIOA + PIO_PDR);
	writel(mask, AT91C_BASE_PIOA + PIO_PPUER);

	pmc_enable_periph_clock(AT91C_ID_PIOA, PMC_PERIPH_CLK_DIVIDER_NA);
	pmc_enable_periph_clock(AT91C_ID_MCI, PMC_PERIPH_CLK_DIVIDER_NA);
}
#endif /* #ifdef CONFIG_SDCARD */
