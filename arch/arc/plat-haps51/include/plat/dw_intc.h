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

/* ------------------------------------------------------------------------- */

#define IRQ_FLG_LOCK	(0x0001)	/* handler is not replaceable */
#define IRQ_FLG_REPLACE	(0x0002)	/* replace existing handler */
#define IRQ_FLG_STD		(0x8000)	/* internally used */

/* ------------------------------------------------------------------------- */

struct dw_int_struct {

	/* Interrupt source enable register (low) */
	unsigned long dw_int_irq_en_l;

	/* Interrupt source enable register (High) */
	unsigned long dw_int_irq_en_h;

	/* Interrupt source mask register (low) */
	unsigned long dw_int_irq_intmask_l;

	/* Interrupt source mask register (high) */
	unsigned long dw_int_irq_intmask_h;

	/* Interrupt force register (low) */
	unsigned long dw_int_irq_intforce_l;

	/* Interrupt force register (high) */
	unsigned long dw_int_irq_intforce_h;

	/* Interrupt raw status register (low) */
	unsigned long dw_int_irq_intraw_l;

	/* Interrupt raw status register (high) */
	unsigned long dw_int_irq_intraw_h;

	/* Interrupt status register (low) */
	unsigned long dw_int_irq_int_status_l;

	/* Interrupt status register (high) */
	unsigned long dw_int_irq_int_status_h;

	/* Interrupt mask status register (low) */
	unsigned long dw_int_irq_int_mask_status_l;

	/* Interrupt mask status register (high) */
	unsigned long dw_int_irq_int_mask_status_h;

	/* Interrupt final status register (low) */
	unsigned long dw_int_irq_int_final_status_l;

	/* Interrupt final status register (high) */
	unsigned long dw_int_irq_int_final_status_h;

	/* Gives an interrupt vector when an interrupt occurs */
	unsigned long dw_int_vector;

	unsigned long res1;

	/* Interrupt vector registers priority register followed by a reserved
	 * register. Reset Value: If ICT_HC_VECTOR_0 = 0, this is a read/write
	 * register used to program the interrupt vector
	 */
	unsigned long dw_int_irq_vector[32];

	/* Fast interrupt enable register */
	unsigned long dw_int_fiq_en;

	/* Fast interrupt mask register */
	unsigned long dw_int_fiq_mask;

	/* Fast interrupt force register */
	unsigned long dw_int_fiq_force;

	/* Fast interrupt source raw register */
	unsigned long dw_int_fiq_raw;

	/* Fast interrupt stutus register */
	unsigned long dw_int_fiq_stat;

	/* Fast interrupt final status register */
	unsigned long dw_int_fiq_final;

	/* System priority level register */
	unsigned long dw_int_plevel;

	/* If ICT_ADD_VECTOR_PORT is set to true, this register reflects
	 * the priority level currently being applied by the DW_ahb_ictl.
	 * In the period between the completion of vector port handshaking
	 * and a write to irq_internal_plevel or arrival of a higher priority
	 * IRQ, it is set to one priority level greater than the priority
	 * level currently being handled by the processor. At all other times,
	 * it reflects the value of irq_plevel. This register should only be
	 * written to at the end of an interrupt service routine in order to
	 * reset the priority level to that of irq_plevel. Reads to this
	 * register return the current priority level. Writes to this register
	 * reset its value to irq_plevel; bus write data is ignored.
	 */
	unsigned long dw_int_internal_plevel;

	unsigned long res2[2];

	/* Individual interrupt priority level registers */
	unsigned long dw_int_priority[32];

	unsigned long res3[162];

	/* Parmameter 2 register */
	unsigned long dw_int_parms2;

	/* Parmameter 1 register */
	unsigned long dw_int_parms1;

	/* Version register */
	unsigned long dw_int_version;

	/* Component type register */
	unsigned long dw_int_type;
};

/* ------------------------------------------------------------------------- */

void dw_intc_init(void);
void dw_intc_enable_int(int);
void dw_intc_disable_int(int);
void dw_intc_do_handle_irq(void);

#endif
