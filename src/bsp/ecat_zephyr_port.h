/*
 * Copyright (c) 2025 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Zephyr adaptation layer for the hpm_sdk EtherCAT port:
 * interrupt registration, ESC board-level init
 */

#ifndef ECAT_ZEPHYR_PORT_H
#define ECAT_ZEPHYR_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ESC function pins + clock + PHY reset hold, replaces hpm_sdk board_init_ethercat() */
void ecat_board_init(void);

/* DIO pins for digital_io.c, replaces hpm_sdk board_init_switch_led() */
void ecat_dio_init(void);

/* Zephyr IRQ_CONNECT registration for ESC PDI/SYNC0/SYNC1/RESET and GPTMR0,
 * must be called before ecat_hardware_init()
 */
void ecat_irq_init(void);

#ifdef __cplusplus
}
#endif

#endif /* ECAT_ZEPHYR_PORT_H */
