/*
 * Copyright (C) 2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/perf_event.h>
#include <linux/errno.h>
#include <asm/arcregs.h>

static int __init init_hw_perf_events(void)
{
	int i;
	struct bcr_perfctr  unit;

	union cc_nm {
		struct {
			unsigned int word0, word1;
			unsigned int sentinel;
		} indiv;
		char str[16];
	} cc_nm;

	READ_BCR(ARC_REG_PERFCTR_BCR, unit);

	if (!unit.present) {
		pr_info("Extn [700-perfctr]\t: N/A\n");
		return -ENODEV;
	}

	pr_info("Extn [700-perfctr]\t: %d Conditions\n", unit.num_cc);

	cc_nm.str[8] = '\0';

	for ( i=0; i < unit.num_cc; i++) {
		write_aux_reg(ARC_REG_PERFCTR_CC_INDEX, i);
		cc_nm.indiv.word0 = read_aux_reg(ARC_REG_PERFCTR_CC_NM0);
		cc_nm.indiv.word1 = read_aux_reg(ARC_REG_PERFCTR_CC_NM1);
		pr_info("CC #%2d : %s\n", i, cc_nm.str);
	}

	return 0;
}
early_initcall(init_hw_perf_events);
