/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __AM_INTC_H
#define __AM_INTC_H

#define AM_INTC_NAME		"am_intc"

#ifdef CONFIG_AM_INTC

void __init am_intc_init(void);
irqreturn_t am_intc_do_handle_irq(int, void *);
void am_intc_enable_int(int);
void am_intc_disable_int(int);

#else	/* !CONFIG_AM_INTC */

#define am_intc_init()

#endif	/* CONFIG_AM_INTC */

#endif
