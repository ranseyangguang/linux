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
#include <plat/dw_intc.h>

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

#ifdef CONFIG_DW_INTC

static void dw_intc_irq_enable(struct irq_data *data)
{
	dw_intc_enable_int(TO_INTC_IRQ(data->irq));
}

static void dw_intc_irq_disable(struct irq_data *data)
{
	dw_intc_disable_int(TO_INTC_IRQ(data->irq));
}

static void dw_intc_irq_mask(struct irq_data *data)
{
	dw_intc_disable_int(TO_INTC_IRQ(data->irq));
}

static void dw_intc_irq_unmask(struct irq_data *data)
{
	dw_intc_enable_int(TO_INTC_IRQ(data->irq));
}

/*
 * DesignWare Interrupt Controller
 */
static struct irq_chip dw_intc = {
	.name        = DW_INTC_NM,
	.irq_enable  = dw_intc_irq_enable,
	.irq_disable = dw_intc_irq_disable,
	.irq_mask    = dw_intc_irq_mask,
	.irq_unmask  = dw_intc_irq_unmask,
};
#endif

void __init plat_init_IRQ()
{
	int i;
	printk(KERN_INFO "Init ARC Platform IRQ's\n");

	for (i = 0; i < NR_IRQS; i++) {
		/*
		 * Be aware IRQ  0-31 is ARC Core INTC
		 *          IRQ 32.. is DW_INTC
		 */
		if (!IS_EXTERNAL_IRQ(i))
			irq_set_chip_and_handler(i, &incore_intc,
						 handle_level_irq);
#ifdef CONFIG_DW_INTC
		else
			irq_set_chip_and_handler(i, &dw_intc,
						 handle_level_irq);
#endif
	}

	dw_intc_init();

	/*
	 * SMP Hack because UART IRQ hardwired to cpu0 (boot-cpu) but if the
	 * request_irq() comes from any other CPU, the low level IRQ unamsking
	 * essential for getting Interrupts won't be enabled on cpu0, locking
	 * up the UART state machine.
	 */
#ifdef CONFIG_SMP
	arch_unmask_irq(UART0_IRQ);
#endif
}
