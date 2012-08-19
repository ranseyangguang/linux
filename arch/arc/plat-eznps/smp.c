/*******************************************************************************

  ARC700 Extensions for SMP
  Copyright(c) 2012 EZchip Technologies.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

*******************************************************************************/

#include <linux/smp.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <plat/irq.h>
#include <plat/smp.h>

static char smp_cpuinfo_buf[128];

/*
 *-------------------------------------------------------------------
 * Platform specific callbacks expected by arch SMP code
 *-------------------------------------------------------------------
 */

const char *arc_platform_smp_cpuinfo(void)
{
	sprintf(smp_cpuinfo_buf, "Extn [700-SMP]\t: On\n");

	return smp_cpuinfo_buf;
}

/*
 * Master kick starting another CPU
 */
void arc_platform_smp_wakeup_cpu(int cpu, unsigned long pc)
{
	unsigned int halt_ctrl;

	/* setup the start PC */
	__raw_writel(pc, CPU_SEC_ENTRY_POINT);

	/* Take the cpu out of Halt */
	halt_ctrl = __raw_readl(REG_CPU_HALT_CTL);
	halt_ctrl |= 1 << cpu;
	__raw_writel(halt_ctrl, REG_CPU_HALT_CTL);

}

/*
 * Any SMP specific init any CPU does when it comes up.
 * Here we setup the CPU to enable Inter-Processor-Interrupts
 * Called for each CPU
 * -Master      : init_IRQ()
 * -Other(s)    : start_kernel_secondary()
 */
void arc_platform_smp_init_cpu(void)
{
	int cpu = smp_processor_id();
	int irq;

	/* Check if CPU is configured for more than 16 interrupts */
	if (NR_IRQS <= 16 || get_hw_config_num_irq() <= 16)
		panic("[eznps] IRQ system can't support IPI\n");

	/* Attach the arch-common IPI ISR to our IPI IRQ */
	for (irq = 0; irq < num_possible_cpus(); irq++) {
		unsigned int tmp;
		/* using edge */
		tmp = read_aux_reg(AUX_ITRIGGER);
		write_aux_reg(AUX_ITRIGGER,
				tmp | (1 << (IPI_IRQS_BASE + irq)));

		if (cpu == 0) {
			int rc;

			rc = smp_ipi_irq_setup(cpu, IPI_IRQS_BASE + irq);
			if (rc)
				panic("IPI IRQ %d reg failed on BOOT cpu\n",
					 irq);
		}
		enable_percpu_irq(IPI_IRQS_BASE + irq, 0);
	}
}

void arc_platform_ipi_send(const struct cpumask *callmap)
{
	unsigned int cpu, this_cpu = smp_processor_id();

	for_each_cpu(cpu, callmap) {
		__raw_writel((1 << cpu), REGS_CPU_IPI(this_cpu));
		__raw_writel(0, REGS_CPU_IPI(this_cpu));
	}
}

void arc_platform_ipi_clear(int cpu, int irq)
{
	write_aux_reg(AUX_IPULSE, (1 << irq));
}
