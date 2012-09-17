/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __DW_INTC_H
#define __DW_INTC_H

#ifdef CONFIG_DW_INTC

#define DW_INTC_NM		"dw_intc"

struct dw_intc_regmap {

	/* Interrupt source enable register */
	unsigned long int_enb_l;
	unsigned long int_enb_h;

	/* Interrupt source mask register */
	unsigned long mask_ctl_l;
	unsigned long mask_ctl_h;

	/* Interrupt force register */
	unsigned long int_force_l;
	unsigned long int_force_h;

	/* Interrupt raw status register) */
	unsigned long int_status_raw_l;
	unsigned long int_status_raw_h;

	/* Interrupt status register */
	unsigned long int_status_l;
	unsigned long int_status_h;

	/* Interrupt mask status register */
	unsigned long mask_status_l;
	unsigned long mask_status_h;

	/* Interrupt final status register */
	unsigned long int_status_final_l;
	unsigned long int_status_final_h;	/* offset 0x34 */

	/*
	 * The DW Intc allows "Vectored Interrupts" where 16 vectors can be
	 * pre-programmed corresponding to 16 priority levels.
	 * When Interrupt occurs this contains vector of the highest pending
	 * interrupt.
	 */
	unsigned long int_vector;

	unsigned long res1;

	 /*
	  * Vectors for 16 prio levels:
	  * -Semantics depend on h/w Cfg: ICT_HC_VECTOR_n
	  * -ICT_HC_VECTOR_n: This is Read-only (Vector Hardcoded)
	  */
	struct {
		unsigned long addr;
		unsigned long res2;
	}
	vectors[16];

	/* Fast interrupt register file */
	unsigned long fiq_enb;
	unsigned long fiq_mask_setup;
	unsigned long fiq_force;
	unsigned long fiq_status_raw;
	unsigned long fiq_status;
	unsigned long fiq_status_final;

	/* System priority level register */
	unsigned long prio_level;		/* offset 0xd8 */

	/* If ICT_ADD_VECTOR_PORT is set to true, this register reflects
	 * the priority level currently being applied by the DW_ahb_ictl.
	 * In the period between the completion of vector port handshaking
	 * and a write to prio_level_internal or arrival of a higher priority
	 * IRQ, it is set to one priority level greater than the priority
	 * level currently being handled by the processor. At all other times,
	 * it reflects the value of prio_level. This register should only be
	 * written to at the end of an interrupt service routine in order to
	 * reset the priority level to that of prio_level. Reads to this
	 * register return the current priority level. Writes to this register
	 * reset its value to prio_level; bus write data is ignored.
	 */
	unsigned long prio_level_internal;

	unsigned long res3[2];

	/* Individual interrupt priority level registers */
	unsigned long priority[32];

	unsigned long res4[162];

	unsigned long parms2;
	unsigned long parms1;
	unsigned long version;
	unsigned long component_type;
};

void __init dw_intc_init(void);
irqreturn_t dw_intc_do_handle_irq(int, void *);
void dw_intc_enable_int(int);
void dw_intc_disable_int(int);

#else	/* !CONFIG_DW_INTC */

#define dw_intc_init()

#endif	/* CONFIG_DW_INTC */

#endif
