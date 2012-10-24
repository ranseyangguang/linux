/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/interrupt.h>
#include <plat/irq.h>
#include <plat/am_intc.h>

/*
 * ARC In-Core Interrupt Controller
 */
static void core_intc_mask_irq(struct irq_data *data)
{
	arch_mask_irq(data->irq);
}

static void core_intc_unmask_irq(struct irq_data *data)
{
	arch_unmask_irq(data->irq);
}

static struct irq_chip incore_intc = {
	.name		= "In-core Intc",
	.irq_mask	= core_intc_mask_irq,
	.irq_unmask	= core_intc_unmask_irq,
};

/*
 * AMlogic interrupt controller
 */
#ifdef CONFIG_AM_INTC

static void am_intc_irq_enable(struct irq_data *data)
{
	am_intc_enable_int(data->irq);
}

static void am_intc_irq_disable(struct irq_data *data)
{
	am_intc_disable_int(data->irq);
}

static void am_intc_irq_mask(struct irq_data *data)
{
	am_intc_disable_int(data->irq);
}

static void am_intc_irq_unmask(struct irq_data *data)
{
	am_intc_enable_int(data->irq);
}

static struct irq_chip am_intc = {
	.name        = AM_INTC_NAME,
	.irq_enable  = am_intc_irq_enable,
	.irq_disable = am_intc_irq_disable,
	.irq_mask    = am_intc_irq_mask,
	.irq_unmask  = am_intc_irq_unmask,
};
#endif /* CONFIG_AM_INTC */


void __init plat_init_IRQ()
{
	int i;
	printk(KERN_INFO "Init ARC Platform IRQ's\n");

	for (i = 0; i < NR_IRQS; i++) {
		/*
		 * Be aware IRQ  0-(CORE_IRQ-1) is ARC Core INTC
		 *          IRQ CORE_IRQ.. is AM_INTC
		 */
		if (i < CORE_IRQ)
			irq_set_chip_and_handler(i, &incore_intc,
						 handle_level_irq);
#ifdef CONFIG_AM_INTC
		else
			irq_set_chip_and_handler(i, &am_intc,
						 handle_level_irq);
#endif
	}

	am_intc_init();
}
