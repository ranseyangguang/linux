/* Abilis Systems early console driver
 *
 * Copyright (C) Abilis Systems 2012
 *
 * Authors: Pierrick Hascoet <pierrick.hascoet@abilis.com>
 *          Christian Ruppert <christian.ruppert@abilis.com>
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

#include <linux/module.h>
#include <linux/serial.h>
#include <linux/console.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <plat/memmap.h>
#include <asm/io.h>
#include <asm/clk.h>

#include "tb100_early_console.h"

#ifdef CONFIG_TB100_TUBE_CONSOLE
#define TUBE_WRITE_CMD(x)   writel((x), (u32 *)(MISC_REGS_A7 + 0x0))
#define TUBE_WRITE_DATA(x)  writel((x), (u32 *)(MISC_REGS_A7 + 0x4))

#define TUBE_CMD_VT_FINISH  (0xffffffff)

#define MSG_TYPE_INFO       (0x01)

#define tb100_early_serial_init()

static void tube_vconsole_write(struct console *con, const char *s,
				unsigned int n)
{
	unsigned int i;

	TUBE_WRITE_CMD(MSG_TYPE_INFO);
	for (i = 0; i < n; i++, s++) {
		if (*s == '\n')
			TUBE_WRITE_DATA('\r');
		TUBE_WRITE_DATA(*s);
	}
	TUBE_WRITE_DATA(0);
}
#else /* CONFIG_TB100_TUBE_CONSOLE */

#include <linux/serial_reg.h>

/* USR status register definition */
#define APB_UART_USR_RFF	0x10
#define APB_UART_USR_RFNE	0x08
#define APB_UART_USR_TFE	0x04
#define APB_UART_USR_TFNF	0x02
#define APB_UART_USR_BUSY	0x01

/* LSR status register definition */
#define APB_UART_LSR_DR     0x01

#define AB_UART_REGSH (2)
#define ab_uart_writereg(REG, VAL) writel((VAL), (u32 *)(APB_UART0_BASEADDR \
						+ ((REG) << AB_UART_REGSH)))
#define ab_uart_readreg(REG) readl((u32 *)(APB_UART0_BASEADDR \
						+ ((REG) << AB_UART_REGSH)))

#define ab_uart_divisor (arc_get_core_freq() / 16 / 3 \
				/ CONFIG_TB100_EARLY_CONSOLE_BAUD)

static void __init tb100_early_serial_init(void)
{
	ab_uart_writereg(UART_LCR, ab_uart_readreg(UART_LCR) | 0x80);
	ab_uart_writereg(UART_LCR, ab_uart_readreg(UART_LCR) | 0x03); /* 8n1 */

	ab_uart_writereg(UART_DLL, ab_uart_divisor & 0xFF);
	ab_uart_writereg(UART_DLM, (ab_uart_divisor & 0xFF00) >> 8);

	ab_uart_writereg(UART_LCR, ab_uart_readreg(UART_LCR) & ~0x80);
}

/* write a single byte to the serial port */
static void tb100_early_serial_putc(char c)
{
	if (c == '\n')
		tb100_early_serial_putc('\r');

	/* wait for tx fifo empty */
	while ((ab_uart_readreg(UART_LSR) & 0x60) == 0x20)
		cpu_relax();

	ab_uart_writereg(UART_TX, c);
}

static void tb100_early_serial_write(struct console *con, const char *s,
					unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++, s++)
		tb100_early_serial_putc(*s);
}
#endif /* CONFIG_TB100_TUBE_CONSOLE */

#ifdef CONFIG_TB100_TUBE_CONSOLE
static struct console tb100_vconsole = {
	.name = "tube",
	.write = tube_vconsole_write,
	.flags = CON_PRINTBUFFER,
	.index = 0
};
#else
static struct __initdata console tb100_vconsole = {
#define UN(A) #A
#define UARTNAME(ADDR) "ser" UN(ADDR)
	.name = UARTNAME(APB_UART0_BASEADDR),
#undef UARTNAME
	.write = tb100_early_serial_write,
	.flags = CON_PRINTBUFFER | CON_BOOT,
	.index = 0
};
#endif

void __init tb100_early_console_init(void)
{
	tb100_early_serial_init();
	register_console(&tb100_vconsole);
}
