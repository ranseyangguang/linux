/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ASM_ARC_A_OUT_H
#define _ASM_ARC_A_OUT_H

struct exec {
	unsigned long a_info;	/* Use macros N_MAGIC, etc for access */
	unsigned a_text;	/* length (b) of text, in bytes */
	unsigned a_data;	/* length of data, in bytes */
	unsigned a_bss;		/* length (b) of uninit data area for file */
	unsigned a_syms;	/* length (b) of symbol table data in file */
	unsigned a_entry;	/* start address */
	unsigned a_trsize;	/* length (b) of relocation info for text */
	unsigned a_drsize;	/* length (b) of relocation info for data */
};

#define N_TRSIZE(a)     ((a).a_trsize)
#define N_DRSIZE(a)     ((a).a_drsize)
#define N_SYMSIZE(a)    ((a).a_syms)

#endif /*  _ASM_ARC_A_OUT_H */
