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

#include <plat/dw_intc.h>
#include <plat/irq.h>
#include <plat/memmap.h>


struct irqaction      node;
struct dw_int_struct *intc_ptr = (void *)DW_INTC_BASE;

/* ------------------------------------------------------------------------- */

void  dw_intc_init(void)
{
	int ret;

	printk(KERN_INFO " init DW_INTC with (%d) interrupts\n",
		NR_IRQS - OFFSET_DW_INT_CTRL);

	/* set DW interrupt handler in default state */
	intc_ptr->dw_int_irq_en_l = 0x0;      /* disable all interrupts */
	intc_ptr->dw_int_irq_intmask_l = 0x0; /* no interrupts masked out */
	intc_ptr->dw_int_irq_en_h = 0x0;      /* disable all interrupts */
	intc_ptr->dw_int_irq_intmask_h = 0x0; /* no interrupts masked out */
	intc_ptr->dw_int_plevel = 0x0;        /* set intlevel to 0 */

	/* install the DW interrupt handler */
	node.handler = (void *)dw_intc_do_handle_irq;
	node.flags = IRQ_FLG_LOCK;
	node.dev_id = NULL;
	node.name = "dw_intc";
	node.next = NULL;

	/* request an irq for the kernel list */
	ret = request_irq(CONFIG_DW_INTC_NR,
		node.handler, node.flags, node.name, node.dev_id);
	if (ret)
		panic("Unable to attach to chained DW Interrupt Controller.\n");
}

void dw_intc_enable_int(int vector)
{
	unsigned long reg;

	/* set controller's interrupt enable register */
	reg = intc_ptr->dw_int_irq_en_l;
	reg |= (1 << vector);
	intc_ptr->dw_int_irq_en_l = reg;
}

void dw_intc_disable_int(int vector)
{
	unsigned long reg;

	/* disable controller interrupt to the CPU */
	reg = intc_ptr->dw_int_irq_en_l;
	reg &= ~(1 << vector);
	intc_ptr->dw_int_irq_en_l = reg;
}

void dw_intc_do_handle_irq(void)
{
	unsigned long intsrc;
	int intnum;

	intsrc = intc_ptr->dw_int_irq_int_final_status_h;
	intnum = OFFSET_DW_INT_CTRL;

	while (intnum != 0)	{
		/* hunt for the real interrupt number */
		if (intnum == OFFSET_DW_INT_CTRL)
			intsrc = intc_ptr->dw_int_irq_int_final_status_l;

		if (intsrc & 0x80000000)
			break;

		intnum--;
		intsrc = intsrc << 1;
	}

	/* when found handle the interrupt */
	generic_handle_irq(intnum + OFFSET_DW_INT_CTRL - 1);
}
