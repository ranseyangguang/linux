/*
 * Copyright (C) 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_PERF_EVENT_H
#define __ASM_PERF_EVENT_H

#define ARC_REG_PERFCTR_BCR		0xf6
#define ARC_REG_PERFCTR_CC_INDEX	0x240
#define ARC_REG_PERFCTR_CC_NM0		0x241
#define ARC_REG_PERFCTR_CC_NM1		0x242

struct bcr_perfctr {
#ifdef CONFIG_CPU_BIG_ENDIAN
	unsigned int num_cc:16, res:8, present:8;
#else
	unsigned int present:8, res:8, num_cc:16;
#endif
};

#endif /* __ASM_PERF_EVENT_H */
