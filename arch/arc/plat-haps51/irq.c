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
static struct irq_chip core_fpga_chip = {
	.name		= "core_intc",
	.irq_mask	= core_intc_mask_irq,
	.irq_unmask	= core_intc_unmask_irq,
};

void __init plat_init_IRQ()
{
	int i;
	printk(KERN_INFO "Init ARC Platform IRQ's\n");

	for (i = 0; i < NR_IRQS; i++) {
		/*
		 * Be aware IRQ  0-31 is ARC Core INTC
		 *          IRQ 32-95 is DW_INTC
		 */
		irq_set_chip_and_handler(i, &core_fpga_chip, handle_level_irq);
	}

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
