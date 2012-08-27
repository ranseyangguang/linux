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

#define OFFSET_DW_INT_CTRL	32

#define NR_IRQS			(OFFSET_DW_INT_CTRL + 64)

/* Internal interrupt controller */
#define RESET_IRQ		0
#define MEMERR_IRQ		1
#define INSTRERR_IRQ	2
#define TIMER0_IRQ		3
#define TIMER1_IRQ		4
#define USBH_IRQ		5
#define USBD_IRQ		6
#define DSI_IRQ			7
#define CSI2_IRQ		8
#define SDIO_IRQ		9
#define DMAC_IRQ		10
#define UART0_IRQ		11
#define INTC_IRQ		12
/* INT 13..15 reserved for internal purposes */
#define WDT_IRQ			16
#define GMAC_IRQ		17
/* INT 18..31 reserved for application use */

/* DesignWare interrupt controller */
#define UART1_IRQ		(OFFSET_DW_INT_CTRL + 0)
#define UART2_IRQ		(OFFSET_DW_INT_CTRL + 1)
#define UART3_IRQ		(OFFSET_DW_INT_CTRL + 2)

/* Defines for generic ARC code usage */
#define TIMER0_INT		TIMER0_IRQ

#endif
