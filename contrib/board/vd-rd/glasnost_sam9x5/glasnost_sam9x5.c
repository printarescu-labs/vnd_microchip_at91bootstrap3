/* ----------------------------------------------------------------------------
 *         Printarescu Labs
 * ----------------------------------------------------------------------------
 * Copyright (c) 2010, Atmel Corporation
 * Copyright (c) 2025, Printarescu Labs
 *
 * Based on at91sam9x5ek (AT91SAM9X5-EK) board support code.
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
#include "arch/at91_rstc.h"
#include "arch/at91_pmc/pmc.h"
#include "arch/at91_smc.h"
#include "arch/at91_pio.h"
#include "arch/at91_ddrsdrc.h"
#include "spi.h"
#include "gpio.h"
#include "pmc.h"
#include "usart.h"
#include "debug.h"
#include "ddramc.h"
#include "timer.h"
#include "watchdog.h"
#include "glasnost_sam9x5.h"

static void initialize_dbgu(void)
{
	const struct pio_desc dbgu_pins[] = {
		{"RXD", AT91C_PIN_PA(9), 0, PIO_DEFAULT, PIO_PERIPH_A},
		{"TXD", AT91C_PIN_PA(10), 0, PIO_DEFAULT, PIO_PERIPH_A},
		{(char *)0, 0, 0, PIO_DEFAULT, PIO_PERIPH_A},
	};

	pmc_enable_periph_clock(AT91C_ID_PIOA_B, PMC_PERIPH_CLK_DIVIDER_NA);
	pio_configure(dbgu_pins);

	usart_init(BAUDRATE(MASTER_CLOCK, BAUD_RATE));
}

#ifdef CONFIG_DDR2
static void ddramc_reg_config(struct ddramc_register *ddramc_config)
{
	ddramc_config->mdr = (AT91C_DDRC2_DBW_16_BITS
			| AT91C_DDRC2_MD_DDR2_SDRAM);

	ddramc_config->cr = (AT91C_DDRC2_NC_DDR10_SDR9
			| AT91C_DDRC2_NR_13
			| AT91C_DDRC2_CAS_3
			| AT91C_DDRC2_NB_BANKS_8
			| AT91C_DDRC2_DISABLE_RESET_DLL
			| AT91C_DDRC2_DECOD_INTERLEAVED);

	ddramc_config->rtr = 0x411;

	ddramc_config->t0pr = (AT91C_DDRC2_TRAS_(6)
			| AT91C_DDRC2_TRCD_(2)
			| AT91C_DDRC2_TWR_(2)
			| AT91C_DDRC2_TRC_(8)
			| AT91C_DDRC2_TRP_(2)
			| AT91C_DDRC2_TRRD_(2)
			| AT91C_DDRC2_TWTR_(2)
			| AT91C_DDRC2_TMRD_(2));

	ddramc_config->t1pr = (AT91C_DDRC2_TXP_(2)
			| AT91C_DDRC2_TXSRD_(200)
			| AT91C_DDRC2_TXSNR_(19)
			| AT91C_DDRC2_TRFC_(18));

	ddramc_config->t2pr = (AT91C_DDRC2_TFAW_(7)
			| AT91C_DDRC2_TRTP_(2)
			| AT91C_DDRC2_TRPA_(3)
			| AT91C_DDRC2_TXARDS_(7)
			| AT91C_DDRC2_TXARD_(2));
}

static void ddramc_init(void)
{
	unsigned long csa;
	struct ddramc_register ddramc_reg;

	ddramc_reg_config(&ddramc_reg);

	pmc_enable_system_clock(AT91C_PMC_DDR);

	csa = readl(AT91C_BASE_CCFG + CCFG_EBICSA);
	csa |= AT91C_EBI_CS1A_SDRAMC;
	csa &= ~AT91C_EBI_DBPUC;
	csa |= AT91C_EBI_DBPDC;
	csa |= AT91C_EBI_DRV_HD;

	writel(csa, AT91C_BASE_CCFG + CCFG_EBICSA);

	ddram_initialize(AT91C_BASE_DDRSDRC, AT91C_BASE_CS1, &ddramc_reg);
}
#endif /* #ifdef CONFIG_DDR2 */

#define LED_BLUE_MASK	(0x01 << 21)
#define LED_GREEN_MASK	(0x01 << 11)
#define LED_RED_MASK	(0x01 << 12)

static void leds_init(void)
{
	pmc_enable_periph_clock(AT91C_ID_PIOA_B, PMC_PERIPH_CLK_DIVIDER_NA);

	writel(LED_BLUE_MASK, AT91C_BASE_PIOA + PIO_SODR);
	writel(LED_BLUE_MASK, AT91C_BASE_PIOA + PIO_OER);
	writel(LED_BLUE_MASK, AT91C_BASE_PIOA + PIO_PER);

	writel(LED_GREEN_MASK | LED_RED_MASK, AT91C_BASE_PIOB + PIO_SODR);
	writel(LED_GREEN_MASK | LED_RED_MASK, AT91C_BASE_PIOB + PIO_OER);
	writel(LED_GREEN_MASK | LED_RED_MASK, AT91C_BASE_PIOB + PIO_PER);

	writel(LED_BLUE_MASK, AT91C_BASE_PIOA + PIO_CODR);
}

void board_boot_error_indication(void)
{
	writel(LED_RED_MASK, AT91C_BASE_PIOB + PIO_CODR);
	writel(LED_BLUE_MASK, AT91C_BASE_PIOA + PIO_CODR);
}

#ifdef CONFIG_HW_INIT
void hw_init(void)
{
	at91_disable_wdt();

	leds_init();

	pmc_init_pll(0);

	pmc_cfg_plla(PLLA_SETTINGS);

	pmc_mck_cfg_set(0, BOARD_PRESCALER_MAIN_CLOCK,
			AT91C_PMC_PLLADIV2 | AT91C_PMC_MDIV | AT91C_PMC_CSS);

	pmc_mck_cfg_set(0, BOARD_PRESCALER_PLLA,
			AT91C_PMC_PLLADIV2 | AT91C_PMC_MDIV | AT91C_PMC_CSS);

	writel(AT91C_RSTC_KEY_UNLOCK | AT91C_RSTC_URSTEN, AT91C_BASE_RSTC + RSTC_RMR);

	timer_init();

	initialize_dbgu();

#ifdef CONFIG_DDR2
	ddramc_init();
#endif
}
#endif /* #ifdef CONFIG_HW_INIT */

#ifdef CONFIG_DATAFLASH
void at91_spi0_hw_init(void)
{
	const struct pio_desc spi0_pins[] = {
		{"MISO", AT91C_PIN_PA(11), 0, PIO_DEFAULT, PIO_PERIPH_A},
		{"MOSI", AT91C_PIN_PA(12), 0, PIO_DEFAULT, PIO_PERIPH_A},
		{"SPCK", AT91C_PIN_PA(13), 0, PIO_DEFAULT, PIO_PERIPH_A},
		{(char *)0, 0, 0, PIO_DEFAULT, PIO_PERIPH_A},
	};

	pio_configure(spi0_pins);

#if (AT91C_SPI_PCS_DATAFLASH == AT91C_SPI_PCS0_DATAFLASH)
	writel((0x01 << 14), AT91C_BASE_PIOA + PIO_IDR);
	writel((0x01 << 14), AT91C_BASE_PIOA + PIO_PPUDR);
	writel((0x01 << 14), AT91C_BASE_PIOA + PIO_SODR);
	writel((0x01 << 14), AT91C_BASE_PIOA + PIO_OER);
	writel((0x01 << 14), AT91C_BASE_PIOA + PIO_PER);
#endif

#if (AT91C_SPI_PCS_DATAFLASH == AT91C_SPI_PCS1_DATAFLASH)
	writel((0x01 << 7), AT91C_BASE_PIOA + PIO_IDR);
	writel((0x01 << 7), AT91C_BASE_PIOA + PIO_PPUDR);
	writel((0x01 << 7), AT91C_BASE_PIOA + PIO_SODR);
	writel((0x01 << 7), AT91C_BASE_PIOA + PIO_OER);
	writel((0x01 << 7), AT91C_BASE_PIOA + PIO_PER);
#endif

	pmc_enable_periph_clock(AT91C_ID_PIOA_B, PMC_PERIPH_CLK_DIVIDER_NA);
	pmc_enable_periph_clock(AT91C_ID_SPI0, PMC_PERIPH_CLK_DIVIDER_NA);
}
#endif /* #ifdef CONFIG_DATAFLASH */

#ifdef CONFIG_SDCARD
void at91_mci0_hw_init(void)
{
	const struct pio_desc mci_pins[] = {
		{"MCCK", AT91C_PIN_PA(17), 0, PIO_PULLUP, PIO_PERIPH_A},
		{"MCCDA", AT91C_PIN_PA(16), 0, PIO_PULLUP, PIO_PERIPH_A},
		{"MCDA0", AT91C_PIN_PA(15), 0, PIO_PULLUP, PIO_PERIPH_A},
		{"MCDA1", AT91C_PIN_PA(18), 0, PIO_PULLUP, PIO_PERIPH_A},
		{"MCDA2", AT91C_PIN_PA(19), 0, PIO_PULLUP, PIO_PERIPH_A},
		{"MCDA3", AT91C_PIN_PA(20), 0, PIO_PULLUP, PIO_PERIPH_A},
		{(char *)0, 0, 0, PIO_DEFAULT, PIO_PERIPH_A},
	};

	pmc_enable_periph_clock(AT91C_ID_PIOA_B, PMC_PERIPH_CLK_DIVIDER_NA);
	pio_configure(mci_pins);

	pmc_enable_periph_clock(AT91C_ID_HSMCI0, PMC_PERIPH_CLK_DIVIDER_NA);
}
#endif /* #ifdef CONFIG_SDCARD */
