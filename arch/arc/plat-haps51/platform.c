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

#ifdef CONFIG_STMMAC_ETH
#include <linux/stmmac.h>
#include <linux/phy.h>

static int stphy_reset(void *bus)
{
	return 1;
}

static struct stmmac_mdio_bus_data mdio_bus_data = {
	.bus_id = 1,
	.phy_reset = &stphy_reset,
	.phy_mask = 1,
};

static struct resource stmmac_eth_resources[] = {
	{
		.name = "macirq",
		.start = GMAC_IRQ,
		.end = GMAC_IRQ,
		.flags = IORESOURCE_IRQ,
	}, {
		.name = "stmmac-regs",
		.start = GMAC_BASE,
		.end = GMAC_BASE + GMAC_REG_SZ,
		.flags = IORESOURCE_MEM,
	 },
};

static struct plat_stmmacenet_data mac_private_data = {
	.bus_id = 1,
	.phy_addr = 1,
	.interface = PHY_INTERFACE_MODE_MII,
	.mdio_bus_data = &mdio_bus_data,
	.pbl = 32,
	.clk_csr = 0,
	.has_gmac = 1,
	.enh_desc = 0,
};

static struct platform_device stmmac_eth_device = {
	.name = "stmmaceth",
	.resource = stmmac_eth_resources,
	.num_resources = ARRAY_SIZE(stmmac_eth_resources),
	.dev = {
		.platform_data = &mac_private_data,
		.p = 0,
	},
};
#endif /* CONFIG_STMMAC_ETH */

/* ------------------------------------------------------------------------- */

static struct platform_device *dw_platform_devices[] __initdata = {
	&dw_uart_device,
#ifdef CONFIG_STMMAC_ETH
	&stmmac_eth_device,
#endif
};

int __init dw_platform_init(void)
{
	return platform_add_devices(dw_platform_devices,
		ARRAY_SIZE(dw_platform_devices));
}

arch_initcall(dw_platform_init);
