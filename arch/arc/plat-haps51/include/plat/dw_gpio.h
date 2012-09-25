/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __DW_GPIO_PDATA_H__
#define __DW_GPIO_PDATA_H__

#define ARCH_NR_DW_GPIOS 32

#define DW_GPIO_PORTA_MASK  (1 << 0)
#define DW_GPIO_PORTB_MASK  (1 << 1)
#define DW_GPIO_PORTC_MASK  (1 << 2)
#define DW_GPIO_PORTD_MASK  (1 << 3)

struct dw_gpio_pdata {
	int           gpio_base;          /* first GPIO in the chip */
	int           ngpio;              /* number of GPIO pins */
	unsigned long port_mask;          /* mask of ports to use for GPIO */
	unsigned long gpio_int_en;        /* enable interrupts mask for GPIO */
	unsigned long gpio_int_mask;      /* mask pins generate an interrupt */
	unsigned long gpio_int_type;      /* intrpr level or edge triggered */
	unsigned long gpio_int_pol;       /* Low/High level trigger */
	unsigned long gpio_int_both_edge; /* rising & falling edge triggered */
	unsigned long gpio_int_deb;       /* interrupt debounce enabled mask */
};

#endif /* __DW_GPIO_PDATA_H__ */
