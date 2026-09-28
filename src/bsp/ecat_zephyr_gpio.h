/*
 * Copyright (c) 2025 HPMicro
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Drop-in replacements for the hpm_gpio_drv.h inline helpers, backed by the
 * Zephyr GPIO driver and the gpio port nodes from the device tree.
 * Call sites keep the hpm_sdk signature (gpio base pointer is ignored).
 */

#ifndef ECAT_ZEPHYR_GPIO_H
#define ECAT_ZEPHYR_GPIO_H

#include <zephyr/drivers/gpio.h>
#include "hpm_soc.h"
#include "hpm_gpio_regs.h" /* GPIO_DO_GPIOA..E port indices */

static inline const struct device *ecat_gpio_dev(uint32_t port)
{
	switch (port) {
	case GPIO_DO_GPIOA:
		return DEVICE_DT_GET(DT_NODELABEL(gpioa));
	case GPIO_DO_GPIOB:
		return DEVICE_DT_GET(DT_NODELABEL(gpiob));
	case GPIO_DO_GPIOC:
		return DEVICE_DT_GET(DT_NODELABEL(gpioc));
	case GPIO_DO_GPIOD:
		return DEVICE_DT_GET(DT_NODELABEL(gpiod));
	case GPIO_DO_GPIOE:
		return DEVICE_DT_GET(DT_NODELABEL(gpioe));
	default:
		return NULL;
	}
}

static inline void gpio_write_pin(GPIO_Type *ptr, uint32_t port, uint8_t pin, uint8_t high)
{
	(void) ptr;
	gpio_pin_set_raw(ecat_gpio_dev(port), pin, high);
}

static inline uint8_t gpio_read_pin(GPIO_Type *ptr, uint32_t port, uint8_t pin)
{
	(void) ptr;
	return (uint8_t) gpio_pin_get_raw(ecat_gpio_dev(port), pin);
}

static inline void gpio_set_pin_output_with_initial(GPIO_Type *ptr, uint32_t port, uint8_t pin,
						    uint8_t initial)
{
	(void) ptr;
	gpio_pin_configure(ecat_gpio_dev(port), pin,
			   initial ? GPIO_OUTPUT_HIGH : GPIO_OUTPUT_LOW);
}

static inline void gpio_set_pin_output(GPIO_Type *ptr, uint32_t port, uint8_t pin)
{
	gpio_set_pin_output_with_initial(ptr, port, pin, 0);
}

static inline void gpio_set_pin_input(GPIO_Type *ptr, uint32_t port, uint8_t pin)
{
	(void) ptr;
	gpio_pin_configure(ecat_gpio_dev(port), pin, GPIO_INPUT);
}

#endif /* ECAT_ZEPHYR_GPIO_H */
