/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * vineetg: Feb 2009
 *  -For AA4 board, IRQ assignments to peripherals
 */

#ifndef __PLAT_IRQ_H
#define __PLAT_IRQ_H

#ifdef CONFIG_SMP
#define NR_IRQS 32
#else
#define NR_IRQS 16
#endif

#define TIMER0_INT      3
#define TIMER1_INT      4

#define VUART_IRQ       5
#define VUART1_IRQ      10
#define VUART2_IRQ      11

#define VMAC_IRQ        6

#define IDE_IRQ         13
#define PCI_IRQ         14

#define PS2_IRQ		(15)

#ifdef CONFIG_SMP
#define IDU_INTERRUPT_0 16
#endif

/* For ARC FPGA Board- nothing fancy needed */

#define platform_setup_irq(irq, flags)		(0)

#define platform_process_interrupt(irq)
#define platform_irq_init()
#define platform_free_irq(irq)
#define platform_enable_irq(irq)
#define platform_disable_irq(irq)

#endif
