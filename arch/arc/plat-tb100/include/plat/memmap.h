/*
 * Abilis Systems: TB100 peripheral address map
 * Copyright (C) 2008-2012 Pierrick Hascoet <pierrick.hascoet@abilis.com>,
 *                         Christian Ruppert <christian.ruppert@abilis.com>
 */

#ifndef __PLAT_MEMMAP_H
#define __PLAT_MEMMAP_H

#define I2C4_BASE		(0xFF124000)
#define I2C3_BASE		(0xFF123000)
#define I2C2_BASE		(0xFF122000)
#define I2C1_BASE		(0xFF121000)
#define I2C0_BASE		(0xFF120000)
#define MISC_REGS_A7		(0xFF106000)     /* tube */
#define APB_UART0_BASEADDR	(0xFF100000)     /* uart0 */
#define GMAC1_BASE		(0xFE110000)
#define GMAC0_BASE		(0xFE100000)
#define A6_IRQCTL_BASE		(0xFE001000)
#define A7_IRQCTL_BASE		(0xFE002000)
#define VMAC_REG_BASEADDR	(0xC0FC2000)

#endif
