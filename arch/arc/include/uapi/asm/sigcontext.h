/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ASM_ARC_SIGCONTEXT_H
#define _ASM_ARC_SIGCONTEXT_H

#include <asm/ptrace.h>

/*
 * Signal context structure - contains all info to do with the state
 * before the signal handler was invoked.
 */
struct sigcontext {
	/*
	 * open coded pt_regs as it is no longer part of userspace ABI
	 * But this is binary compatible with pt_regs
	 * upstream 3.9 does this in cleanest way
	 */
	struct {
		long pad;
		long bta, lp_start, lp_end, lp_count;
		long status32, ret, blink, fp, gp;
		long r12, r11, r10, r9, r8, r7, r6, r5, r4, r3, r2, r1, r0;
		long sp;
		long orig_r0, orig_r8_word;
	} regs;
};

#endif /* _ASM_ARC_SIGCONTEXT_H */
