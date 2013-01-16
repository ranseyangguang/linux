/* Abilis Systems interrupt controller driver
 *
 * Copyright (C) Abilis Systems 2012
 *
 * Author: Christian Ruppert <christian.ruppert@abilis.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <plat/irq.h>
#include <plat/memmap.h>
#include <linux/io.h>

#define AB_IRQCTL_INT_ENABLE   (0x00)
#define AB_IRQCTL_INT_STATUS   (0x04)
#define AB_IRQCTL_SRC_MODE     (0x08)
#define AB_IRQCTL_SRC_POLARITY (0x0C)
#define AB_IRQCTL_INT_MODE     (0x10)
#define AB_IRQCTL_INT_POLARITY (0x14)
#define AB_IRQCTL_INT_FORCE    (0x18)

#define ab_irqctl_writereg(REG, VAL) writel((VAL), \
					(u32 *)(A7_IRQCTL_BASE + (REG)))
#define ab_irqctl_readreg(REG)       readl((u32 *)(A7_IRQCTL_BASE + (REG)))

int tb100_irq_set_type(struct irq_data *data, unsigned int flow_type)
{
	unsigned int irq = data->irq;
	uint32_t im, mod, pol;

	if (!IS_EXTERNAL_IRQ(irq))
		return 0;

	im = 1UL << irq;

	mod = ab_irqctl_readreg(AB_IRQCTL_SRC_MODE) | im;
	pol = ab_irqctl_readreg(AB_IRQCTL_SRC_POLARITY) | im;

	switch (flow_type & IRQF_TRIGGER_MASK) {
	case IRQ_TYPE_EDGE_FALLING:
		pol ^= im;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		mod ^= im;
		break;
	case IRQ_TYPE_NONE:
	case IRQ_TYPE_LEVEL_LOW:
		mod ^= im;
		pol ^= im;
		break;
	case IRQ_TYPE_EDGE_RISING:
		break;
	default:
		printk(KERN_ERR "%s: IRQ %d requested with more than "
			"one trigger conditions.\n", __func__, irq);
		return -EBADR;
	}

	ab_irqctl_writereg(AB_IRQCTL_SRC_MODE, mod);
	ab_irqctl_writereg(AB_IRQCTL_SRC_POLARITY, pol);
	ab_irqctl_writereg(AB_IRQCTL_INT_STATUS, im);

	return 0;
}

void tb100_irq_unmask(struct irq_data *data)
{
	unsigned int irq = data->irq;

	arch_unmask_irq(irq);

	if (!IS_EXTERNAL_IRQ(irq))
		return;

	ab_irqctl_writereg(AB_IRQCTL_INT_ENABLE,
			ab_irqctl_readreg(AB_IRQCTL_INT_ENABLE) | (1 << irq));
}

void tb100_irq_mask(struct irq_data *data)
{
	unsigned int irq = data->irq;

	arch_mask_irq(irq);

	if (!IS_EXTERNAL_IRQ(irq))
		return;

	ab_irqctl_writereg(AB_IRQCTL_INT_ENABLE,
			ab_irqctl_readreg(AB_IRQCTL_INT_ENABLE) & ~(1 << irq));
}

struct irq_chip irq_tb100_chip = {
	.name          = "TB100-ICTL",
	.irq_mask      = tb100_irq_mask,
	.irq_unmask    = tb100_irq_unmask,
	.irq_set_type  = tb100_irq_set_type,
};

void __init plat_tb100_init_IRQ(void)
{
	int i;

	for (i = 0; i < NR_IRQS; i++)
		irq_set_chip_and_handler(i, &irq_tb100_chip, handle_level_irq);

	ab_irqctl_writereg(AB_IRQCTL_INT_ENABLE, 0);
	ab_irqctl_writereg(AB_IRQCTL_INT_MODE, 0);
	ab_irqctl_writereg(AB_IRQCTL_INT_POLARITY, 0);
	ab_irqctl_writereg(AB_IRQCTL_INT_STATUS, ~0UL);
}
