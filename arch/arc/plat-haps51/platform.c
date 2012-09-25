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
#include <plat/irq.h>
#include <plat/memmap.h>

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_SERIAL_8250
#include <linux/serial.h>
#include <linux/serial_8250.h>

#define ARC_UART1_CLK 12500000 /* 1/4th of core clock */

#define PORT(_base, _irq)                \
{                                        \
	.iobase   = _base,                   \
	.membase  = (void __iomem *) _base,  \
	.mapbase  = (resource_size_t) _base, \
	.irq      = _irq,                    \
	.irqflags = 0    ,                   \
	.uartclk  = ARC_UART1_CLK,           \
	.regshift = 2,                       \
	.iotype   = UPIO_MEM32,              \
	.flags    = UPF_SKIP_TEST,           \
	.type     = PORT_16550A              \
}

static struct plat_serial8250_port dw_uart_data[] = {
	PORT(DW_UART_BASE0, UART0_IRQ),
#ifdef CONFIG_DW_INTC
	PORT(DW_UART_BASE1, UART1_IRQ),
	PORT(DW_UART_BASE2, UART2_IRQ),
	PORT(DW_UART_BASE3, UART3_IRQ),
#endif
	{},
};

static struct platform_device dw_uart_device = {
	.name = "serial8250",
	.id   = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = dw_uart_data,
	},
};
#endif /* CONFIG_SERIAL_8250 */

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_DW_GPIO
#include <plat/dw_gpio.h>

static struct resource dw_gpio_res[] = {
	[0] = {
		.name  = "dw_gpio_reg",
		.start = GPIO_BASE0,
		.end   = GPIO_BASE0 + GPIO_REG_SZ,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "dw_gpio_irq",
		.start = DW_GPIO_IRQ,
		.end   = DW_GPIO_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct dw_gpio_pdata dw_gpio_data = {
	.gpio_base          = 0,
	.ngpio              = 0x20,
	.port_mask          = 0xf,
	.gpio_int_en        = 0xff,
	.gpio_int_mask      = 0x00,
	.gpio_int_type      = 0xff,
	.gpio_int_pol       = 0x00,
	.gpio_int_both_edge = 0xff,
	.gpio_int_deb       = 0x00,
};

static struct platform_device dw_apb_gpio_device = {
	.name          = "dw_gpio",
	.resource      = &dw_gpio_res[0],
	.num_resources = 2,
	.dev = {
		.platform_data = &dw_gpio_data,
	},
};

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <plat/gpio.h>

static struct gpio_keys_button dw_gpio_keys_table[] = {
	{
		.gpio		= GPIO_BUTTON_1,
		.code		= 232, /* = CENTER */
		.desc		= "GPIO key 1",
		.active_low	= 1,
		.wakeup		= 1,
		.type		= EV_KEY,
	},
	{
		.gpio		= GPIO_BUTTON_2,
		.code		= KEY_UP,
		.desc		= "GPIO key 2",
		.active_low	= 1,
		.wakeup		= 1,
		.type		= EV_KEY,
	},
	{
		.gpio		= GPIO_BUTTON_3,
		.code		= 229, /* = MENU */
		.desc		= "GPIO key 3",
		.active_low	= 1,
		.wakeup		= 1,
		.type		= EV_KEY,
	},
	{
		.gpio		= GPIO_BUTTON_4,
		.code		= 102, /* = HOME */
		.desc		= "GPIO key 4",
		.active_low	= 1,
		.wakeup		= 1,
		.type		= EV_KEY,
	},
	{
		.gpio		= GPIO_BUTTON_5,
		.code		= KEY_LEFT,
		.desc		= "GPIO key 5",
		.active_low	= 1,
		.wakeup		= 1,
		.type		= EV_KEY,
	},
	{
		.gpio		= GPIO_BUTTON_6,
		.code		= KEY_DOWN,
		.desc		= "GPIO key 6",
		.active_low	= 1,
		.wakeup		= 1,
		.type		= EV_KEY,
	},
	{
		.gpio		= GPIO_BUTTON_7,
		.code		= KEY_RIGHT,
		.desc		= "GPIO key 7",
		.active_low	= 1,
		.wakeup		= 1,
		.type		= EV_KEY,
	},
	{
		.gpio		= GPIO_BUTTON_8,
		.code		= 158, /* = BACK */
		.desc		= "GPIO key 8",
		.active_low	= 1,
		.wakeup		= 1,
		.type		= EV_KEY,
	}
};

static struct gpio_keys_platform_data dw_gpio_keys_data = {
	.buttons  = dw_gpio_keys_table,
	.nbuttons = ARRAY_SIZE(dw_gpio_keys_table),
};

static struct platform_device dw_device_gpiokeys = {
	.name = "gpio-keys",
	.id   = -1,
	.dev = {
		.platform_data = &dw_gpio_keys_data,
	},
};
#endif /* CONFIG_KEYBOARD_GPIO || CONFIG_KEYBOARD_GPIO_MODULE */

#ifdef CONFIG_LEDS_GPIO
#include <linux/leds.h>

static struct gpio_led default_leds[] = {
	{
		.name = "led1",
		.gpio = GPIO_LED_1,
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name = "led2",
		.gpio = GPIO_LED_2,
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name = "led3",
		.gpio = GPIO_LED_3,
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name = "led4",
		.gpio = GPIO_LED_4,
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name = "led5",
		.gpio = GPIO_LED_5,
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},
};

static struct gpio_led_platform_data arc_led_data = {
	.num_leds = ARRAY_SIZE(default_leds),
	.leds     = default_leds,
};

static struct platform_device device_gpioleds = {
	.name = "leds-gpio",
	.id   = -1,
	.dev = {
		.platform_data = &arc_led_data,
	}
};
#endif /* CONFIG_LEDS_GPIO */
#endif /* CONFIG_DW_GPIO */

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_STMMAC_ETH
#include <linux/stmmac.h>
#include <linux/phy.h>

static int stphy_reset(void *bus)
{
	return 1;
}

static struct stmmac_mdio_bus_data mdio_bus_data = {
	.bus_id    = 1,
	.phy_reset = &stphy_reset,
	.phy_mask  = 1,
};

static struct resource stmmac_eth_resources[] = {
	[0] = {
		.name  = "stmmac-regs",
		.start = GMAC_BASE,
		.end   = GMAC_BASE + GMAC_REG_SZ,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "macirq",
		.start = GMAC_IRQ,
		.end   = GMAC_IRQ,
		.flags = IORESOURCE_IRQ,
	 },
};

static struct plat_stmmacenet_data mac_private_data = {
	.bus_id        = 1,
	.phy_addr      = 1,
	.interface     = PHY_INTERFACE_MODE_MII,
	.mdio_bus_data = &mdio_bus_data,
	.pbl           = 32,
	.clk_csr       = 0,
	.has_gmac      = 1,
	.enh_desc      = 0,
};

static struct platform_device stmmac_eth_device = {
	.name          = "stmmaceth",
	.resource      = stmmac_eth_resources,
	.num_resources = ARRAY_SIZE(stmmac_eth_resources),
	.dev = {
		.platform_data = &mac_private_data,
		.p             = 0,
	},
};
#endif /* CONFIG_STMMAC_ETH */

/* ------------------------------------------------------------------------- */

static struct platform_device *dw_platform_devices[] __initdata = {
#ifdef CONFIG_SERIAL_8250
	&dw_uart_device,
#endif
#ifdef CONFIG_DW_GPIO
	&dw_apb_gpio_device,
#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
	&dw_device_gpiokeys,
#endif
#ifdef CONFIG_LEDS_GPIO
	&device_gpioleds,
#endif
#endif
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
