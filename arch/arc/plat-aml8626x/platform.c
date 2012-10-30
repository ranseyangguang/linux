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
#include <linux/err.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <asm-generic/sizes.h>

#include <asm/irq.h>
#include <plat/irq.h>
#include <plat/memmap.h>
#include <plat/am_regs.h>
#include <plat/usbclock.h>

/* ------------------------------------------------------------------------- */
#ifdef CONFIG_USB_DWCOTG
static struct resource dwc_otg_a_resources[] = {
	[0] = 	{
			.start	= IO_USB_A_BASE,
			.end	= IO_USB_A_BASE + SZ_128K - 1,
			.flags	= IORESOURCE_MEM,
		},
	[1] =	{
			.start	= AM_ISA_GEN_IRQ(30),
			.end	= AM_ISA_GEN_IRQ(30),
			.flags	= IORESOURCE_IRQ,
		},
};

static struct resource dwc_otg_b_resources[] = {
	[0] = 	{
			.start	= IO_USB_B_BASE,
			.end	= IO_USB_B_BASE + SZ_128K - 1,
			.flags	= IORESOURCE_MEM,
		},
	[1] =	{
			.start	= AM_ISA_GEN_IRQ(31),
			.end	= AM_ISA_GEN_IRQ(31),
			.flags	= IORESOURCE_IRQ,
		},
};

static u64 dwc_otg_dmamask = DMA_BIT_MASK(32);

static struct platform_device dwc_otg_a_dev = {
	.name		= "dwc_otg",
	.id		= 0,
	.resource	= dwc_otg_a_resources,
	.num_resources	= ARRAY_SIZE(dwc_otg_a_resources),
	.dev		= {	
				.dma_mask		= &dwc_otg_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
			},
};

static struct platform_device dwc_otg_b_dev = {
	.name		= "dwc_otg",
	.id		= 1,
	.resource	= dwc_otg_b_resources,
	.num_resources	= ARRAY_SIZE(dwc_otg_b_resources),
	.dev		= {	
				.dma_mask		= &dwc_otg_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
			},
};
#endif /* CONFIG_USB_DWCOTG */
/* ------------------------------------------------------------------------- */
#define OSD1_ADDR_START	0x90000000
#define OSD1_ADDR_END	0x90ffffff
#define OSD2_ADDR_START	0x91000000
#define OSD2_ADDR_END	0x91ffffff

static struct resource apollofb_device_resources[] = {
    [0] = {
        .start = AM_ISA_GEN_IRQ(INT_VIU_VSYNC),
        .end   = AM_ISA_GEN_IRQ(INT_VIU_VSYNC),
        .flags = IORESOURCE_IRQ,
    },
    [1] = {
        .start = OSD1_ADDR_START,
        .end   = OSD1_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
    [2] = { //for osd2
        .start = OSD2_ADDR_START,
        .end   = OSD2_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device apollofb_device = {
    .name       = "amlfb",
    .id         = 0,
    .num_resources = ARRAY_SIZE(apollofb_device_resources),
    .resource      = apollofb_device_resources,
};

static struct resource vout_device_resources[] = {
    [0] = {
        .start = 0,
        .end   = 0,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device vout_device = {
    .name       = "amlvout",
    .id         = 0,
    .num_resources = ARRAY_SIZE(vout_device_resources),
    .resource      = vout_device_resources,
};
static struct platform_device *dw_platform_devices[] __initdata = {
#ifdef CONFIG_USB_DWCOTG
	&dwc_otg_a_dev,
	&dwc_otg_b_dev,
#endif /* CONFIG_USB_DWCOTG */
	&vout_device,
	&apollofb_device,
};

int __init dw_platform_init(void)
{
#ifdef CONFIG_USB_DWCOTG
	set_usb_phy_clk(USB_PHY_CLOCK_SEL_XTAL_DIV2);
	set_usb_phy_id_mode(USB_PHY_PORT_A, USB_PHY_MODE_HW);
	set_usb_phy_id_mode(USB_PHY_PORT_B, USB_PHY_MODE_SW_HOST);
#endif
	return platform_add_devices(dw_platform_devices,
		ARRAY_SIZE(dw_platform_devices));
}

arch_initcall(dw_platform_init);
