/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/interrupt.h>
#include <asm/irq.h>

#include <plat/irq.h>
#include <plat/memmap.h>

static void core_intc_mask_irq(struct irq_data *data)
{
	arch_mask_irq(data->irq);
}

static void core_intc_unmask_irq(struct irq_data *data)
{
	arch_unmask_irq(data->irq);
}

/*
 * ARC Core Interrupt Controller
 */
static struct irq_chip core_intc_chip = {
	.name		= "core_intc",
	.irq_mask	= core_intc_mask_irq,
	.irq_unmask	= core_intc_unmask_irq,
};

#ifdef CONFIG_DW_INTC
#include <plat/dw_intc.h>

static void dw_intc_enable_irq(struct irq_data *data)
{
	dw_intc_enable_int(data->irq - OFFSET_DW_INT_CTRL);
}

static void dw_intc_disable_irq(struct irq_data *data)
{
	dw_intc_disable_int(data->irq - OFFSET_DW_INT_CTRL);
}

static void dw_intc_mask_irq(struct irq_data *data)
{
	dw_intc_disable_int(data->irq - OFFSET_DW_INT_CTRL);
}

static void dw_intc_unmask_irq(struct irq_data *data)
{
	dw_intc_enable_int(data->irq - OFFSET_DW_INT_CTRL);
}

/*
 * DesignWare Interrupt Controller
 */
static struct irq_chip dw_intc_chip = {
	.name        = "dw_intc",
	.irq_enable  = dw_intc_enable_irq,
	.irq_disable = dw_intc_disable_irq,
	.irq_mask    = dw_intc_mask_irq,
	.irq_unmask  = dw_intc_unmask_irq,
};
#endif

void __init plat_init_IRQ()
{
	int i;
	printk(KERN_INFO "Init ARC Platform IRQ's\n");

	for (i = 0; i < NR_IRQS; i++) {
		/*
		 * Be aware IRQ  0-31 is ARC Core INTC
		 *          IRQ 32-95 is DW_INTC
		 */
#ifdef CONFIG_DW_INTC
		if (i < OFFSET_DW_INT_CTRL) {
#endif
			irq_set_chip_and_handler(i, &core_intc_chip,
				handle_level_irq);
#ifdef CONFIG_DW_INTC
		} else {
			irq_set_chip_and_handler(i, &dw_intc_chip,
				handle_level_irq);
		}
#endif
	}

#ifdef CONFIG_DW_INTC
	/* register the 'dw_intc' device */
	dw_intc_init();
#endif

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
