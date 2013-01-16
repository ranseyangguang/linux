/* Abilis Systems TB100 platform initialisation
 *
 * Copyright (C) Abilis Systems 2012
 *
 * Author: Christian Ruppert <christian.ruppert@abilis.com>
 *         Pierrick Hascoet <pierrick.hascoet@abilis.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <linux/sizes.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>

#ifdef CONFIG_FIXED_PHY
#include <linux/phy_fixed.h>
#endif
#include <linux/phy.h>

#include <linux/serial_8250.h>
#include <linux/stmmac.h>
#include <asm/clk.h>
#include <plat/memmap.h>
#include <plat/irq.h>

#ifdef CONFIG_EARLY_PRINTK
#include "tb100_early_console.h"
#endif

static struct plat_serial8250_port uart0_pdat = {
	.mapbase  = APB_UART0_BASEADDR,			/* resource base */
	.irq      = UART0_IRQ,				/* interrupt number */
	.irqflags = IRQF_TRIGGER_LOW,			/* request_irq flags */
	.regshift = 2,					/* register shift */
	.iotype   = UPIO_MEM32,				/* UPIO_* */
	.flags    = UPF_BOOT_AUTOCONF | UPF_IOREMAP,	/* UPF_* flags */
};

static struct platform_device uart0_pdev = {
	.name	= "serial8250",
	.id	= PLAT8250_DEV_PLATFORM,
	.dev	= {
		.platform_data = &uart0_pdat,
	},
};

static struct resource i2c0_resources[] = {
	{
		.start	= I2C0_BASE,
		.end	= I2C0_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= I2C_IRQ,
		.flags	= IORESOURCE_IRQ | IRQF_SHARED | IRQF_TRIGGER_LOW,
	},
};
static struct platform_device i2c0_pdev = {
	.name = "i2c_designware",
	.id = 0,
	.dev = {
		.coherent_dma_mask = ~0,
	},
	.num_resources = ARRAY_SIZE(i2c0_resources),
	.resource = i2c0_resources,
};


static struct resource eth0_resources[] = {
	[0] = {
		.start	= GMAC0_BASE,
		.end	= GMAC0_BASE + 0x1058 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.name	= "macirq",
		.start	= GMAC0_INT,
		.flags	= IORESOURCE_IRQ | IRQF_TRIGGER_LOW,
	},
};

static int tb100_phy0_reset(void *mii)
{
	printk(KERN_DEBUG ">>> %s <<<\n", __func__);
	return 0;
}

#ifndef CONFIG_FIXED_PHY
static struct stmmac_mdio_bus_data mdio0_bus_data = {
	.bus_id		= 0,
	.phy_mask	= ~(1U << 1),
	.phy_reset	= &tb100_phy0_reset,
};
#endif

static struct plat_stmmacenet_data eth0_pdata = {
	.bus_id		= 0,
#ifndef CONFIG_FIXED_PHY
	.phy_addr	= -1,
#else
	.phy_addr	= 1,
#endif
	.has_gmac	= 1,
	.interface	= PHY_INTERFACE_MODE_RGMII,
#ifndef CONFIG_FIXED_PHY
	.mdio_bus_data	= &mdio0_bus_data,
#endif
	.enh_desc	= 0,
	.tx_coe		= 0, /* tx csum in HW = 1 */
	.bugged_jumbo	= 0,
	.pmt		= 0,
};

static u64 eth0_dma_mask = ~(u32) 0;
struct platform_device eth0_pdev = {
	.name = "stmmaceth",
	.id = 0,
	.num_resources = ARRAY_SIZE(eth0_resources),
	.resource = eth0_resources,
	.dev = {
		.platform_data = &eth0_pdata,
/*		.dma_mask = &eth0_dma_mask, */
/*		.coherent_dma_mask = ~0, */
	},
};


static struct platform_device *tb100_platform[] = {
	&uart0_pdev,
	&i2c0_pdev,
	&eth0_pdev,
};

#ifdef CONFIG_FIXED_PHY
static struct fixed_phy_status mac0_fixed_phy_status = {
	.link = 1,
	.speed = 1000,
	.duplex = 1,
};
#endif

int __init tb100_platform_init(void)
{
	uart0_pdat.uartclk = arc_get_core_freq() / 3;

#ifdef CONFIG_FIXED_PHY
	fixed_phy_add(PHY_POLL, 1, &mac0_fixed_phy_status);
#endif

	return platform_add_devices(tb100_platform,
					ARRAY_SIZE(tb100_platform));
}
arch_initcall(tb100_platform_init);

static void __init plat_tb100_early_init(void)
{
#ifdef CONFIG_EARLY_PRINTK
	tb100_early_console_init();
#endif
}
