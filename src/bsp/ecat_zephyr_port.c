/*
 * Copyright (c) 2025 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Zephyr adaptation layer for the hpm_sdk EtherCAT port:
 *  - interrupt registration: PLIC sources are level-2 irqs behind the CPU
 *    external line, so raw hpm_soc IRQn values must be multi-level encoded
 *    (IRQ_CONNECT on the raw value would collide with DT-registered irqs)
 *  - ESC board-level init (clock/pins/PHY reset), GPIO via Zephyr driver
 */

#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <zephyr/irq_multilevel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include "hpm_soc.h"
#include "hpm_iomux.h"
#include "hpm_clock_drv.h"

#include "ecat_board_cfg.h"
#include "ecat_zephyr_gpio.h"
#include "ecat_config.h"
#include "ecat_zephyr_port.h"

/* ISRs implemented in hpm_ecat_hw.c */
void ecat_pdi_isr(void);
void ecat_sync0_isr(void);
void ecat_sync1_isr(void);
void ecat_reset_isr(void);
void ecat_time_ms_isr(void);

static void ecat_pdi_isr_z(const void *arg)
{
	ARG_UNUSED(arg);
	ecat_pdi_isr();
}

static void ecat_sync0_isr_z(const void *arg)
{
	ARG_UNUSED(arg);
	ecat_sync0_isr();
}

static void ecat_sync1_isr_z(const void *arg)
{
	ARG_UNUSED(arg);
	ecat_sync1_isr();
}

static void ecat_reset_isr_z(const void *arg)
{
	ARG_UNUSED(arg);
	ecat_reset_isr();
}

static void ecat_time_ms_isr_z(const void *arg)
{
	ARG_UNUSED(arg);
	ecat_time_ms_isr();
}

/* PLIC sources sit at interrupt level 2, parented to the CPU external line */
#define CPU_EXT_IRQ_LINE DT_IRQ_BY_IDX(DT_NODELABEL(plic), 0, irq)
#define ECAT_PLIC_IRQ(n) (IRQ_TO_L2(n) | CPU_EXT_IRQ_LINE)

/* PLIC priority order preserved from hpm_sdk: PDI > SYNC0/SYNC1 > RESET/TIMER */
void ecat_irq_init(void)
{
	IRQ_CONNECT(ECAT_PLIC_IRQ(IRQn_ESC), 1, ecat_pdi_isr_z, NULL, 0);
	IRQ_CONNECT(ECAT_PLIC_IRQ(IRQn_ESC_SYNC0), 2, ecat_sync0_isr_z, NULL, 0);
	IRQ_CONNECT(ECAT_PLIC_IRQ(IRQn_ESC_SYNC1), 2, ecat_sync1_isr_z, NULL, 0);
	IRQ_CONNECT(ECAT_PLIC_IRQ(IRQn_ESC_RESET), 3, ecat_reset_isr_z, NULL, 0);
	IRQ_CONNECT(ECAT_PLIC_IRQ(ECAT_TIMER_GPTRM_IRQ), 3, ecat_time_ms_isr_z, NULL, 0);
}

/* required by hpm_ecat_hw.c, originally intc_m_enable_irq_with_priority() */
void ENABLE_ESC_INT(void)
{
	irq_enable(ECAT_PLIC_IRQ(IRQn_ESC_SYNC0));
	irq_enable(ECAT_PLIC_IRQ(IRQn_ESC_SYNC1));
	irq_enable(ECAT_PLIC_IRQ(IRQn_ESC));
	irq_enable(ECAT_PLIC_IRQ(IRQn_ESC_RESET));
	irq_enable(ECAT_PLIC_IRQ(ECAT_TIMER_GPTRM_IRQ));
}

void DISABLE_ESC_INT(void)
{
	irq_disable(ECAT_PLIC_IRQ(IRQn_ESC_SYNC0));
	irq_disable(ECAT_PLIC_IRQ(IRQn_ESC_SYNC1));
	irq_disable(ECAT_PLIC_IRQ(IRQn_ESC));
	irq_disable(ECAT_PLIC_IRQ(IRQn_ESC_RESET));
	irq_disable(ECAT_PLIC_IRQ(ECAT_TIMER_GPTRM_IRQ));
}

/* copied from hpm_sdk hpm6e00evk pinmux.c init_esc_pins() */
static void init_esc_pins(void)
{
	/* ESC */
	HPM_IOC->PAD[IOC_PAD_PA09].FUNC_CTL = IOC_PA09_FUNC_CTL_ESC0_REFCK;

	HPM_IOC->PAD[IOC_PAD_PA30].FUNC_CTL = IOC_PA30_FUNC_CTL_ESC0_MDIO;

	HPM_IOC->PAD[IOC_PAD_PA31].FUNC_CTL = IOC_PA31_FUNC_CTL_ESC0_MDC;

	/*
	 * ESC needs to configure these pins for specific functions, The corresponding macro should be defined in ecat_board_cfg.h.
	 * Such as PA15(IOC_PA15_FUNC_CTL_ESC0_CTR_3) pin as NMII_LINK0 signal, BOARD_ECAT_NMII_LINK0_CTRL_INDEX should be defined to 3.
	 * GPIO to reset PHY
	 */
	HPM_IOC->PAD[IOC_PAD_PA10].FUNC_CTL = IOC_PA10_FUNC_CTL_GPIO_A_10;

	/* NMII_LINK0(PORTA_LINK) function */
	HPM_IOC->PAD[IOC_PAD_PA15].FUNC_CTL = IOC_PA15_FUNC_CTL_ESC0_CTR_3;

	/* NMII_LINK1(PORTB_LINK) function */
	HPM_IOC->PAD[IOC_PAD_PA11].FUNC_CTL = IOC_PA11_FUNC_CTL_ESC0_CTR_0;

	/* LED_ERROR function */
	HPM_IOC->PAD[IOC_PAD_PE02].FUNC_CTL = IOC_PE02_FUNC_CTL_ESC0_CTR_6;

	/* LED_RUN function */
	HPM_IOC->PAD[IOC_PAD_PE03].FUNC_CTL = IOC_PE03_FUNC_CTL_ESC0_CTR_1;

	/* ESC PORTA */
	HPM_IOC->PAD[IOC_PAD_PA24].FUNC_CTL = IOC_PA24_FUNC_CTL_ESC0_P0_TXCK;

	HPM_IOC->PAD[IOC_PAD_PA29].FUNC_CTL = IOC_PA29_FUNC_CTL_ESC0_P0_TXEN;

	HPM_IOC->PAD[IOC_PAD_PA25].FUNC_CTL = IOC_PA25_FUNC_CTL_ESC0_P0_TXD_0;
	HPM_IOC->PAD[IOC_PAD_PA26].FUNC_CTL = IOC_PA26_FUNC_CTL_ESC0_P0_TXD_1;
	HPM_IOC->PAD[IOC_PAD_PA27].FUNC_CTL = IOC_PA27_FUNC_CTL_ESC0_P0_TXD_2;
	HPM_IOC->PAD[IOC_PAD_PA28].FUNC_CTL = IOC_PA28_FUNC_CTL_ESC0_P0_TXD_3;

	HPM_IOC->PAD[IOC_PAD_PA21].FUNC_CTL = IOC_PA21_FUNC_CTL_ESC0_P0_RXCK;

	HPM_IOC->PAD[IOC_PAD_PA16].FUNC_CTL = IOC_PA16_FUNC_CTL_ESC0_P0_RXDV;
	HPM_IOC->PAD[IOC_PAD_PA23].FUNC_CTL = IOC_PA23_FUNC_CTL_ESC0_P0_RXER;
	HPM_IOC->PAD[IOC_PAD_PA17].FUNC_CTL = IOC_PA17_FUNC_CTL_ESC0_P0_RXD_0;
	HPM_IOC->PAD[IOC_PAD_PA18].FUNC_CTL = IOC_PA18_FUNC_CTL_ESC0_P0_RXD_1;
	HPM_IOC->PAD[IOC_PAD_PA19].FUNC_CTL = IOC_PA19_FUNC_CTL_ESC0_P0_RXD_2;
	HPM_IOC->PAD[IOC_PAD_PA20].FUNC_CTL = IOC_PA20_FUNC_CTL_ESC0_P0_RXD_3;

	/* ESC PORTB */
	HPM_IOC->PAD[IOC_PAD_PB18].FUNC_CTL = IOC_PB18_FUNC_CTL_ESC0_P1_TXCK;

	HPM_IOC->PAD[IOC_PAD_PB23].FUNC_CTL = IOC_PB23_FUNC_CTL_ESC0_P1_TXEN;

	HPM_IOC->PAD[IOC_PAD_PB19].FUNC_CTL = IOC_PB19_FUNC_CTL_ESC0_P1_TXD_0;
	HPM_IOC->PAD[IOC_PAD_PB20].FUNC_CTL = IOC_PB20_FUNC_CTL_ESC0_P1_TXD_1;
	HPM_IOC->PAD[IOC_PAD_PB21].FUNC_CTL = IOC_PB21_FUNC_CTL_ESC0_P1_TXD_2;
	HPM_IOC->PAD[IOC_PAD_PB22].FUNC_CTL = IOC_PB22_FUNC_CTL_ESC0_P1_TXD_3;

	HPM_IOC->PAD[IOC_PAD_PB17].FUNC_CTL = IOC_PB17_FUNC_CTL_ESC0_P1_RXCK;

	HPM_IOC->PAD[IOC_PAD_PB12].FUNC_CTL = IOC_PB12_FUNC_CTL_ESC0_P1_RXDV;

	HPM_IOC->PAD[IOC_PAD_PA22].FUNC_CTL = IOC_PA22_FUNC_CTL_ESC0_P1_RXER;
	HPM_IOC->PAD[IOC_PAD_PB13].FUNC_CTL = IOC_PB13_FUNC_CTL_ESC0_P1_RXD_0;
	HPM_IOC->PAD[IOC_PAD_PB14].FUNC_CTL = IOC_PB14_FUNC_CTL_ESC0_P1_RXD_1;
	HPM_IOC->PAD[IOC_PAD_PB15].FUNC_CTL = IOC_PB15_FUNC_CTL_ESC0_P1_RXD_2;
	HPM_IOC->PAD[IOC_PAD_PB16].FUNC_CTL = IOC_PB16_FUNC_CTL_ESC0_P1_RXD_3;

	/* ESC SYNC0 signal */
	HPM_IOC->PAD[IOC_PAD_PE06].FUNC_CTL = IOC_PE06_FUNC_CTL_ESC0_EVTO_0;
}

/* copied from hpm_sdk hpm6e00evk pinmux.c init_esc_in_out_pin() */
static void init_esc_in_out_pin(void)
{
	HPM_IOC->PAD[IOC_PAD_PC31].FUNC_CTL = IOC_PC31_FUNC_CTL_GPIO_C_31;

	HPM_IOC->PAD[IOC_PAD_PD08].FUNC_CTL = IOC_PD08_FUNC_CTL_GPIO_D_08;

	HPM_IOC->PAD[IOC_PAD_PD09].FUNC_CTL = IOC_PD09_FUNC_CTL_GPIO_D_09;

	HPM_IOC->PAD[IOC_PAD_PE14].FUNC_CTL = IOC_PE14_FUNC_CTL_GPIO_E_14;
}

/* replaces hpm_sdk board_init_ethercat() */
void ecat_board_init(void)
{
	clock_add_to_group(clock_esc0, 0);

	init_esc_pins();
	/* keep ECAT PHY reset */
	gpio_set_pin_output_with_initial(BOARD_ECAT_PHY0_RESET_GPIO, BOARD_ECAT_PHY0_RESET_GPIO_PORT_INDEX,
					 BOARD_ECAT_PHY0_RESET_PIN_INDEX, BOARD_ECAT_PHY_RESET_LEVEL);
	gpio_set_pin_output_with_initial(BOARD_ECAT_PHY1_RESET_GPIO, BOARD_ECAT_PHY1_RESET_GPIO_PORT_INDEX,
					 BOARD_ECAT_PHY1_RESET_PIN_INDEX, BOARD_ECAT_PHY_RESET_LEVEL);
}

/* replaces hpm_sdk board_init_switch_led(): DIO pins used by digital_io.c */
void ecat_dio_init(void)
{
	init_esc_in_out_pin();

	gpio_set_pin_input(BOARD_ECAT_IN1_GPIO, BOARD_ECAT_IN1_GPIO_PORT_INDEX,
			   BOARD_ECAT_IN1_GPIO_PIN_INDEX);
	gpio_set_pin_input(BOARD_ECAT_IN2_GPIO, BOARD_ECAT_IN2_GPIO_PORT_INDEX,
			   BOARD_ECAT_IN2_GPIO_PIN_INDEX);

	gpio_set_pin_output_with_initial(BOARD_ECAT_OUT1_GPIO, BOARD_ECAT_OUT1_GPIO_PORT_INDEX,
					 BOARD_ECAT_OUT1_GPIO_PIN_INDEX, 0);
	gpio_set_pin_output_with_initial(BOARD_ECAT_OUT2_GPIO, BOARD_ECAT_OUT2_GPIO_PORT_INDEX,
					 BOARD_ECAT_OUT2_GPIO_PIN_INDEX, 0);
}
