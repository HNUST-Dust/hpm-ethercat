/*
 * Copyright (c) 2025 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Zephyr port of hpm_sdk board.h ECAT section for hpm6e00evk
 */

#ifndef ECAT_BOARD_CFG_H
#define ECAT_BOARD_CFG_H

#include <zephyr/kernel.h>
#include "hpm_soc.h"
#include "hpm_clock_drv.h"

#define BOARD_FLASH_BASE_ADDRESS (0x80000000UL)
#define BOARD_FLASH_SIZE         (16 * 1024 * 1024)

/* XPI NOR configuration, required by eeprom_emulation component / hpm_nor_flash.c */
#define BOARD_APP_XPI_NOR_XPI_BASE     (HPM_XPI0)
#define BOARD_APP_XPI_NOR_CFG_OPT_HDR  (0xfcf90001U)
#define BOARD_APP_XPI_NOR_CFG_OPT_OPT0 (0x00000005U)
#define BOARD_APP_XPI_NOR_CFG_OPT_OPT1 (0x00001000U)

/* EtherCAT definitions */
/* ECAT PORT0 must support */
#define BOARD_ECAT_SUPPORT_PORT1 (1)
#define BOARD_ECAT_SUPPORT_PORT2 (0)

#define BOARD_ECAT_SUPPORT_RUN_ERROR_LED (1) /* board supports RUN/ERROR led */

/* invert esc port link signal, require low level for linkup */
#define BOARD_ECAT_PORT0_LINK_INVERT true  /* depend on hardware */
#define BOARD_ECAT_PORT1_LINK_INVERT true  /* depend on hardware */
#define BOARD_ECAT_PORT2_LINK_INVERT false /* depend on hardware */

#define BOARD_ECAT_PHY0_RESET_GPIO            HPM_GPIO0
#define BOARD_ECAT_PHY0_RESET_GPIO_PORT_INDEX GPIO_DO_GPIOA
#define BOARD_ECAT_PHY0_RESET_PIN_INDEX       (10)

#define BOARD_ECAT_PHY1_RESET_GPIO            HPM_GPIO0
#define BOARD_ECAT_PHY1_RESET_GPIO_PORT_INDEX GPIO_DO_GPIOA
#define BOARD_ECAT_PHY1_RESET_PIN_INDEX       (10)
#define BOARD_ECAT_PHY_RESET_LEVEL            (0)

/* input and output pin for ethercat io test (digital_io.c) */
#define BOARD_ECAT_IN1_GPIO            HPM_GPIO0
#define BOARD_ECAT_IN1_GPIO_PORT_INDEX GPIO_DO_GPIOC
#define BOARD_ECAT_IN1_GPIO_PIN_INDEX  (31U)

#define BOARD_ECAT_IN2_GPIO            HPM_GPIO0
#define BOARD_ECAT_IN2_GPIO_PORT_INDEX GPIO_DO_GPIOD
#define BOARD_ECAT_IN2_GPIO_PIN_INDEX  (9U)

#define BOARD_ECAT_OUT1_GPIO            HPM_GPIO0
#define BOARD_ECAT_OUT1_GPIO_PORT_INDEX GPIO_DO_GPIOD
#define BOARD_ECAT_OUT1_GPIO_PIN_INDEX  (8U)

#define BOARD_ECAT_OUT2_GPIO            HPM_GPIO0 /* reuse RGB red led */
#define BOARD_ECAT_OUT2_GPIO_PORT_INDEX GPIO_DO_GPIOE
#define BOARD_ECAT_OUT2_GPIO_PIN_INDEX  (14U)

#define BOARD_ECAT_OUT_ON_LEVEL (1) /* ECAT control LED on level */

/* Must match IOC_PXnn_FUNC_CTL_ESC0_CTR_y in init_esc_pins (ecat_zephyr_port.c) */
#define BOARD_ECAT_NMII_LINK0_CTRL_INDEX 3
#define BOARD_ECAT_NMII_LINK1_CTRL_INDEX 0
#define BOARD_ECAT_LED_RUN_CTRL_INDEX    1
#define BOARD_ECAT_LED_ERROR_CTRL_INDEX  6

/* ECAT PHY address definition */
#define BOARD_ECAT_PHY_ADDR_OFFSET (0U)
#define BOARD_ECAT_PORT0_PHY_ADDR  (0U) /* actual PHY address = BOARD_ECAT_PHY_ADDR_OFFSET + BOARD_ECAT_PORT0_PHY_ADDR */
#define BOARD_ECAT_PORT1_PHY_ADDR  (1U) /* actual PHY address = BOARD_ECAT_PHY_ADDR_OFFSET + BOARD_ECAT_PORT1_PHY_ADDR */

/* the address of ecat flash emulate eeprom component in flash */
#define BOARD_ECAT_FLASH_EMULATE_EEPROM_ADDR (0x200000) /* offset 2M */

/* busy-wait delay, usable in ISR context */
static inline void board_delay_ms(uint32_t ms)
{
	k_busy_wait(ms * 1000);
}

#endif /* ECAT_BOARD_CFG_H */
