/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <asm/irq.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>

#include <plat/irq.h>
#include <plat/memmap.h>

/* ------------------------------------------------------------------------- */

#define ARC_UART1_CLK	12500000  /* 1/4th of core clock */

#define PORT(_base, _irq)                   \
{                                           \
	.iobase     = _base,                    \
	.membase    = (void __iomem *) _base,   \
	.mapbase    = (resource_size_t) _base,  \
	.irq        = _irq,                     \
	.irqflags   = 0    ,                    \
	.uartclk    = ARC_UART1_CLK,            \
	.regshift   = 2,                        \
	.iotype     = UPIO_MEM32,               \
	.flags      = UPF_SKIP_TEST,            \
	.type       = PORT_16550A               \
}

static struct plat_serial8250_port dw_uart_data[] = {
	PORT(DW_UART_BASE0, UART0_IRQ),
	PORT(DW_UART_BASE1, UART1_IRQ),
	PORT(DW_UART_BASE2, UART2_IRQ),
	PORT(DW_UART_BASE3, UART3_IRQ),
	{},

};

static struct platform_device dw_uart_device = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = dw_uart_data,
	},

};

/* ------------------------------------------------------------------------- */

static struct platform_device *dw_platform_devices[] __initdata = {
	&dw_uart_device,

};

int __init dw_platform_init(void)
{
	return platform_add_devices(dw_platform_devices,
		ARRAY_SIZE(dw_platform_devices));
}

arch_initcall(dw_platform_init);
