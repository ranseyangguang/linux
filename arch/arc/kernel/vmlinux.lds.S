/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm-generic/vmlinux.lds.h>
#include <asm/vmlinux.lds.h>
#include <asm/cache.h>
#include <asm/page.h>
#include <asm/thread_info.h>
#include <plat/memmap.h>

OUTPUT_ARCH(arc)
ENTRY(_stext)

#ifdef CONFIG_CPU_BIG_ENDIAN
jiffies = jiffies_64 + 4;
#else
jiffies = jiffies_64;
#endif

SECTIONS
{
	/*
	 * ICCM starts at 0x8000_0000. So if kernel is relocated to some other
	 * address, make sure peripheral at 0x8z doesn't clash with ICCM
	 * Essentially vector is also in ICCM.
	 */

	. = CONFIG_LINUX_LINK_BASE;

	_int_vec_base_lds = .;
	.vector : {
		*(.vector)
		. = ALIGN(PAGE_SIZE);
	}

#ifdef CONFIG_ARC_HAS_ICCM
	.text.arcfp : {
		*(.text.arcfp)
		. = ALIGN(ICCM_COMPILE_SZ);
	}
#endif

	/*
	 * The reason for having a seperate subsection .init.ramfs is to
	 * prevent objump from including it in kernel dumps
	 *
	 * Reason for having .init.ramfs above .init is to make sure that the
	 * binary blob is tucked away to one side, reducing the displacement
	 * between .init.text and .text, avoiding any possible relocation
	 * errors because of calls from .init.text to .text
	 * Yes such calls do exist. e.g.
	 *	decompress_inflate.c:gunzip( ) -> zlib_inflate_workspace( )
	 */

	__init_begin = .;

	.init.ramfs : { INIT_RAM_FS }

	. = ALIGN(PAGE_SIZE);
	_stext = .;

	HEAD_TEXT_SECTION
	INIT_TEXT_SECTION(L1_CACHE_BYTES)

	/* INIT_DATA_SECTION open-coded: special INIT_RAM_FS handling */
	.init.data : {
		INIT_DATA
		INIT_SETUP(L1_CACHE_BYTES)
		INIT_CALLS
		CON_INITCALL
		SECURITY_INITCALL
	}

	PERCPU_SECTION(L1_CACHE_BYTES)

	/*
	 * .exit.text is discard at runtime, not link time, to deal with
	 * references from .debug_frame
	 * It will be init freed, being inside [__init_start : __init_end]
	 */
	.exit.text : { EXIT_TEXT }
	.exit.data : { EXIT_DATA }

	. = ALIGN(PAGE_SIZE);
	__init_end = .;

	.text : {
		_text = .;
		TEXT_TEXT
		SCHED_TEXT
		LOCK_TEXT
		KPROBES_TEXT
		*(.fixup)
		*(.gnu.warning)
	}
	EXCEPTION_TABLE(L1_CACHE_BYTES)
	_etext = .;

	_sdata = .;
	RO_DATA_SECTION(PAGE_SIZE)

	/*
	 * 1. this is .data essentially
	 * 2. THREAD_SIZE for init.task, must be kernel-stk sz aligned
	 */
	RW_DATA_SECTION(L1_CACHE_BYTES, PAGE_SIZE, THREAD_SIZE)

	_edata = .;

	BSS_SECTION(0, 0, 0)

#ifdef CONFIG_ARC_DW2_UNWIND
	. = ALIGN(PAGE_SIZE);
	.debug_frame  : {
		__start_unwind = .;
		*(.debug_frame)
		__end_unwind = .;
	}
#else
	/DISCARD/ : {	*(.debug_frame) }
#endif

	NOTES

	. = ALIGN(PAGE_SIZE);
	_end = . ;

	STABS_DEBUG
	DISCARDS

	.arcextmap 0 : {
		*(.gnu.linkonce.arcextmap.*)
		*(.arcextmap.*)
	}

	/* open-coded because we need .debug_frame seperately for unwinding */
	.debug_aranges 0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	.debug_info 0 : { *(.debug_info) }
	.debug_abbrev 0 : { *(.debug_abbrev) }
	.debug_line 0 : { *(.debug_line) }
	.debug_str 0 : { *(.debug_str) }
	.debug_loc 0 : { *(.debug_loc) }
	.debug_macinfo 0 : { *(.debug_macinfo) }

#ifdef CONFIG_ARC_HAS_DCCM
	. = DCCM_COMPILE_BASE;
	__arc_dccm_base = .;
	.data.arcfp : {
		*(.data.arcfp)
	}
	. = ALIGN(DCCM_COMPILE_SZ);
#endif
}
