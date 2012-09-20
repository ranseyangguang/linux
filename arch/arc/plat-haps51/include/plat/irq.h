/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __PLAT_IRQ_H
#define __PLAT_IRQ_H

/*
 * This set-up consists of the ARC internal interrupt controller (32
 * interrupt lines) and a DesignWare external interrupt controller providing
 * 64 additional interrupts. The DesignWare interrupt controller is connected
 * to line 12 of the internal interrupt controller.
 * Interrupts on the internal interrupt controller count from 0 up to and
 * including 31; interrupts on the DesignWare interrupt controller count from
 * 32 up to 96. For example, the SPI0 interrupt that is hooked to interrupt
 * line 3 of the DesignWare interrupt controller will have interrupt number 35.
 */

/* Internal interrupt controller */
#define USBH_IRQ		5
#define USBD_IRQ		6
#define DSI_IRQ			7
#define CSI2_IRQ		8
#define SDIO_IRQ		9
#define DMAC_IRQ		10
#define UART0_IRQ		11
#define DW_INTC_IRQ		12	/* Casceded 2nd level Intr Controller */
#define WDT_IRQ			16
#define GMAC_IRQ		17

/* DesignWare interrupt controller Handled Interrupts */
#ifdef CONFIG_DW_INTC

#define DW_INTC_IRQS_START	32
#define DW_INTC_NBR_OF_IRQS	32	/* Making this 64 needs lot more work */

#define UART1_IRQ		(DW_INTC_IRQS_START + 0)
#define UART2_IRQ		(DW_INTC_IRQS_START + 1)
#define UART3_IRQ		(DW_INTC_IRQS_START + 2)

#define NR_IRQS			(DW_INTC_IRQS_START + DW_INTC_NBR_OF_IRQS)

/* Is this IRQ fed by the cacsceded controller */
#define IS_EXTERNAL_IRQ(g_irq)	(g_irq >= DW_INTC_IRQS_START)

/* Global irqnum namespace [32..NR_IRQs] to controller private [0..31] */
#define TO_INTC_IRQ(g_irq)	(g_irq - DW_INTC_IRQS_START)

#define TO_SYS_IRQ(intc_irq)	(intc_irq + DW_INTC_IRQS_START)

#else	/* ! CONFIG_DW_INTC */

#define IS_EXTERNAL_IRQ(g_irq)	(0)
#define NR_IRQS			32

#endif	/* CONFIG_DW_INTC */

#endif
