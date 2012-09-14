/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __PLAT_MEMMAP_H
#define __PLAT_MEMMAP_H

#define DCCM_COMPILE_BASE	CONFIG_LINUX_LINK_BASE
#define DCCM_COMPILE_SZ		(64 * 1024)
#define ICCM_COMPILE_SZ		(64 * 1024)

#define DW_UART_BASE0		0xC0000000
#define DW_UART_BASE1		0xC0002000
#define DW_UART_BASE2		0xC0004000
#define DW_UART_BASE3		0xC0006000
#define UART_REG_SZ		0x00002000

#define DW_INTC_BASE		0xC2000000
#define DW_INTC_REG_SZ		0x01000000

#define GMAC_BASE		0xE2010000
#define GMAC_REG_SZ		0x00004000

#endif
