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
#include <plat/dw_intc.h>
#include <plat/irq.h>
#include <plat/memmap.h>

#define DW_INTC_READ(reg)		readl((u32 *)&(reg))
#define DW_INTC_WRITE(reg, val)		writel((val), (u32 *)&(reg))

struct dw_intc_regmap *intc = (struct dw_intc_regmap *)DW_INTC_BASE;

void __init dw_intc_init(void)
{
	int ret;

	pr_info("DW_INTC: CPU IRQ #[%x], (%d) Device IRQs\n",
		CONFIG_DW_INTC_IRQ, DW_INTC_IRQS_NUM);

	/* default state */
	DW_INTC_WRITE(intc->int_enb_l, 0);	/* disable all interrupts */
	DW_INTC_WRITE(intc->int_enb_h, 0);
	DW_INTC_WRITE(intc->mask_ctl_l, 0);	/* no interrupts masked out */
	DW_INTC_WRITE(intc->mask_ctl_h, 0);
	DW_INTC_WRITE(intc->prio_level, 0);	/* same priority by default */

	/* Hookup the casceded interrupt controller to a CPU IRQ */
	ret = request_irq(CONFIG_DW_INTC_IRQ, dw_intc_do_handle_irq, 0,
			  DW_INTC_NM, NULL);
	if (ret)
		panic("DW_INTC: request_irq failed\n");
}

void dw_intc_enable_int(int irq)
{
	unsigned long val;

	val = DW_INTC_READ(intc->int_enb_l);
	DW_INTC_WRITE(intc->int_enb_l, val | (1 << irq));
}

void dw_intc_disable_int(int irq)
{
	unsigned long val;

	val = DW_INTC_READ(intc->int_enb_l);
	DW_INTC_WRITE(intc->int_enb_l, val & ~(1 << irq));
}

/*
 * Vanilla ISR [Non "vector-mode"]
 */
irqreturn_t dw_intc_do_handle_irq(int irq, void *arg)
{
	unsigned long intsrc;
	int intnum;
	int rc = 0;

	intsrc = DW_INTC_READ(intc->int_status_final_l);

	for (intnum = find_first_bit(&intsrc, DW_INTC_IRQS_NUM);
	     intnum < DW_INTC_IRQS_NUM;
	     intnum = find_next_bit(&intsrc, DW_INTC_IRQS_NUM, intnum+1))
		rc |= generic_handle_irq(TO_SYS_IRQ(intnum));

	return rc;
}
