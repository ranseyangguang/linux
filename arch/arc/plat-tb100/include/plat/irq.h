/*
 * Abilis Systems: interrupt controller driver
 * Copyright (C) 2008-2012 Christian Ruppert <christian.ruppert@abilis.com>
 */

#ifndef PLAT_IRQ_H
#define PLAT_IRQ_H

#define EXT_IRQ_MIN (5)
#define EXT_IRQ_MAX (31)
#define IS_EXTERNAL_IRQ(INT) (((INT) >= EXT_IRQ_MIN) && ((INT) <= EXT_IRQ_MAX))

#define GMAC0_INT  (6)
#define GMAC1_INT  (7)
#define I2C_IRQ    (12)
#define UART0_IRQ  (25)

extern void __init plat_tb100_init_IRQ(void);

#endif
