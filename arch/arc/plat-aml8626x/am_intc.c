/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <plat/am_intc.h>
#include <plat/irq.h>
#include <plat/am_regs.h>
#include <plat/memmap.h>

#define DRIVER_NAME	AM_INTC_NAME
#define IRQ_SETS	4
#define IRQS_PER_SET	32

void __init am_intc_init(void)
{
	int ret, irq_set, reg;
	
	for (irq_set = 0; irq_set < IRQ_SETS; irq_set++) {
		reg = SYS_CPU_0_IRQ_IN0_INTR_MASK + (irq_set << 2);
		CLEAR_CBUS_REG_MASK(reg, 0xffffffff);
		reg = SYS_CPU_0_IRQ_IN0_INTR_FIRQ_SEL + (irq_set << 2);
		CLEAR_CBUS_REG_MASK(reg, 0xffffffff);
	}

	/* Hookup the cascaded interrupt controller to a CPU IRQ */
	ret = request_irq(INT_AM_INTC_IRQ, am_intc_do_handle_irq, 0,
			  DRIVER_NAME, NULL);

	if (ret)
		panic("AM_INTC: request_irq failed\n");
}

static void am_intc_change_int(int irq, bool enable)
{
	int irq_set, reg;
	long unsigned int mask;

	irq -= CORE_IRQ;
	irq_set = irq >> 5;	/* which set of IRQ registers? */
	
	reg = SYS_CPU_0_IRQ_IN0_INTR_MASK + (irq_set << 2);
	mask = 1 << (irq & 31);

	if (enable)
		SET_CBUS_REG_MASK(reg, mask);
	else
		CLEAR_CBUS_REG_MASK(reg, mask);
}

void am_intc_enable_int(int irq)
{
	am_intc_change_int(irq, true);
}

void am_intc_disable_int(int irq)
{
	am_intc_change_int(irq, false);
}

/*
 * Vanilla ISR [Non "vector-mode"]
 */
irqreturn_t am_intc_do_handle_irq(int irq, void *arg)
{
	long unsigned int intsrc[IRQ_SETS];
	int irq_set, intnum, result;
	int rc = -1;

	/* fetch interrupt status for all irq sets */
	for (irq_set = 0; irq_set < IRQ_SETS; irq_set ++)
		intsrc[irq_set] = READ_CBUS_REG(SYS_CPU_0_IRQ_IN0_INTR_STAT +
						(irq_set << 2));


	for (intnum = find_first_bit(&intsrc, 32 * IRQ_SETS);
	     intnum < (32 * IRQ_SETS);
	     intnum = find_next_bit(&intsrc, 32 * IRQ_SETS, intnum+1)) {
		result = generic_handle_irq(intnum + CORE_IRQ);

		/* NOTE: we cannot determine if IRQ is handled, we can only
		 *       find out if generic_handle_irq complains (-EINVAL)
		 *       So, if OK, we assume it has been handled correctly */
		if (result == 0) {
			//printk ("clearing intnum %d (irq %d) \n", intnum, intnum + CORE_IRQ);
			WRITE_CBUS_REG(SYS_CPU_0_IRQ_IN0_INTR_STAT_CLR +
				       ((intnum >> 5) << 2), 
				       1 << (intnum & 31));
		} else {
			printk ("interrupt layer complains about intnum %d (irq %d)!\n", intnum, intnum + CORE_IRQ);
		}

		rc &= result;
	}

	/* if there was at least one interrupt OK, we say we handled it */
	return rc ? IRQ_NONE : IRQ_HANDLED;
}
