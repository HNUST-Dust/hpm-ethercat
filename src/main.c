/*
 * Copyright (c) 2025 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Zephyr port of hpm_sdk samples/ethercat/ecat_io/ecat.c
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>

#include "hpm_soc.h"
#include "ecat_def.h"
#include "ecatappl.h"
#include "ecatslv.h"
#include "applInterface.h"
#include "digital_io.h"
#include "hpm_ecat_hw.h"
#include "ecat_zephyr_port.h"


#define ESC_EEPROM_EMULATION 1

/* RGB LED blink thread, independent of the SSC main loop */
static const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_g = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led_b = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

static void led_blink_fn(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	const struct gpio_dt_spec leds[] = {led_r, led_g, led_b};

	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			return;
		}
		gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
	}

	uint32_t count = 0;

	while (1) {
		k_msleep(500);
		gpio_pin_set_dt(&leds[count % ARRAY_SIZE(leds)], 1);
		gpio_pin_set_dt(&leds[(count + ARRAY_SIZE(leds) - 1) % ARRAY_SIZE(leds)], 0);
		count++;
	}
}

/* priority 4 must be HIGHER than main (5): MainLoop is a busy loop, a lower
 * priority thread would starve; this one only wakes every 500ms
 */
K_THREAD_DEFINE(led_blink_tid, 512, led_blink_fn, NULL, NULL, NULL, 4, 0, 0);

/* called by SSC after the master wrote an object via SDO (mailbox path, main loop context) */
static void CoeWriteInd(UINT16 index, UINT8 subindex, BOOL completeAccess)
{
	(void) subindex;
	(void) completeAccess;

	if (index == 0x2000) {
		/* master data already stored in MyMbxData0x2000, consume it here */
		printk("Master wrote 0x2000 = 0x%08x\n", MyMbxData0x2000);
	}
}

int main(void)
{
	hpm_stat_t stat;

	printk("EtherCAT IO sample (Zephyr)\n");

	/* Zephyr port: board_init() equivalent, ESC clock/pins + PHY reset hold */
	ecat_board_init();
	/* Zephyr port: IRQ_CONNECT for ESC/GPTMR before hardware init enables them */
	ecat_irq_init();

	/* hardware init */
	stat = ecat_hardware_init(HPM_ESC);
	if (stat != status_success) {
		printk("Init ESC peripheral and related devices(EEPROM/PHY) failed!\n");
		return 0;
	}

	MainInit(); /* SSC Initilize the stack */
	pAPPL_CoeWriteInd = CoeWriteInd; /* get notified when master writes objects via SDO */

#if defined(ESC_EEPROM_EMULATION) && ESC_EEPROM_EMULATION
	pAPPL_EEPROM_Read = ecat_eeprom_emulation_read;
	pAPPL_EEPROM_Write = ecat_eeprom_emulation_write;
	pAPPL_EEPROM_Reload = ecat_eeprom_emulation_reload;
	pAPPL_EEPROM_Store = ecat_eeprom_emulation_store;
#endif

	/* Create basic mapping */
	APPL_GenerateMapping(&nPdInputSize, &nPdOutputSize);

	/* Set stack run flag */
	bRunApplication = TRUE;
	/* Execute the stack */
	while (bRunApplication == TRUE) {
		MainLoop();
	}

	/* hardware deinit */

	return 0;
}
