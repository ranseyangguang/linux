/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARC_IRQ_H
#define __ASM_ARC_IRQ_H

#include <plat/irq.h>		/* Board Specific IRQ assignments */
#include <asm-generic/irq.h>

extern void __init arc_init_IRQ(void);
extern void __init plat_init_IRQ(void);
extern int get_hw_config_num_irq(void);

void __cpuinit arc_clocksource_init(void);
void __cpuinit arc_clockevent_init(void);

#endif
