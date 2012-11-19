/******************************************************************************
 * Copyright ARC International (www.arc.com) 2007-2009
 *
 *
 *
 *
 *
 *
 * Vineetg: May 16th, 2008
 *  - Current macro is now implemented as "global register" r25
 *
 *****************************************************************************/
/******************************************************************************
 * Copyright Codito Technologies (www.codito.com) Oct 01, 2004
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *****************************************************************************/

/*
 *  Copyright (C)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Authors : Rahul Trivedi, Sameer Dhavale
 */

#ifndef _ASM_ARC_CURRENT_H
#define _ASM_ARC_CURRENT_H

#ifndef __ASSEMBLY__

#ifdef CONFIG_ARCH_ARC_CURR_IN_REG

register struct task_struct *curr_arc asm ("r25");
#define current (curr_arc)

#else  /* ! CONFIG_ARCH_ARC_CURR_IN_REG */

#include <linux/thread_info.h>

static inline struct task_struct * get_current(void)
{
	return current_thread_info()->task;
}
#define current (get_current())

#endif  /* ! CONFIG_ARCH_ARC_CURR_IN_REG */

#endif /* ! __ASSEMBLY__ */

#endif /* _ASM_ARC_CURRENT_H */
