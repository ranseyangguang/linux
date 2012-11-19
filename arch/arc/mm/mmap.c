/* mmap for ARC
 *
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Vineet Gupta <vineet.gupta@arc.com>
 *  -mmap randomisation (borrowed from x86/ARM/s390 implementations)
 *  -dedicated mmap code virtual address space to ensure libs are mapped
 *   at same vaddr across processes: this is perf boost for VIPT I$ which
 *   suffer from aliasing.
 */

#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/random.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/syscalls.h>
#include <linux/profile.h>
#include <linux/mman.h>
#include <linux/uaccess.h>
#include <asm/mmapcode.h>
#include <asm/tlbflush.h>

#if 0
#define MMAP_DBG(fmt, args...)  pr_debug(fmt , ## args)
#else
#define MMAP_DBG(fmt, args...)
#endif

static unsigned long __do_mmap2(struct file *, unsigned long, unsigned long,
				unsigned long, unsigned long, unsigned long);

SYSCALL_DEFINE6(mmap2, unsigned long, addr_hint, unsigned long, len,
		unsigned long, prot, unsigned long, flags, unsigned long, fd,
		unsigned long, pgoff)
{
	unsigned long vaddr = -EBADF;
	struct file *file = NULL;

	flags &= ~(MAP_EXECUTABLE | MAP_DENYWRITE);

	if (!(flags & MAP_ANONYMOUS)
#ifdef CONFIG_ARC_CMN_MMAP
	    || (flags & MAP_SHARED_CODE)
#endif
	    ) {
		file = fget(fd);
		if (!file)
			goto out;
	}

	down_write(&current->mm->mmap_sem);

	vaddr = __do_mmap2(file, addr_hint, len, prot, flags, pgoff);

	up_write(&current->mm->mmap_sem);

	if (file)
		fput(file);

out:
	return vaddr;
}

/* ARC mmap core:
 * -does special case handling for code mmaps (CONFIG_ARC_CMN_MMAP)
 * -calls generic vm entry-pt do_mmap_pgoff(()
 */
static unsigned long __do_mmap2(struct file *file, unsigned long addr_hint,
				unsigned long len, unsigned long prot,
				unsigned long flags, unsigned long pgoff)
{
	unsigned long vaddr = -EINVAL;

#ifdef CONFIG_ARC_CMN_MMAP

	/* 1. What are we doing here ?
	 * -------------------------------------
	 *  If mmap is for code (right now only shared libs-dso)
	 *  allocate a chunk of vaddr in cmn-code-vaddr space and pass that as
	 *  MAP_FIXED to mmap core to ensure that shared lib code is mmaped at
	 *  addr of our choice and not where Linux vm wants to.
	 *  Doing it here turned out to be much easier than in arch specific
	 *  arc_get_unmapped_area( )
	 *
	 *  If the library was already mmaped by some other task - the existing
	 *  alloc addr is returned - that is how a dso is mapped at same addr,
	 *  across multiple processes.
	 *
	 *  It shd be obvious that this addr alloc is not plugged into VM as
	 *  such, it is merely a helper, for allocating the desired vaddr in
	 *  process-pvt vaddr space.
	 *
	 * 2. Mechanics (part #a - for shared libs loaded by user-space loader)
	 * ------------------------------------------------------------------
	 *  user-space dyn-loader passes a special mmap flag MAP_SHARED_CODE
	 *  (possibly piggybacking a MAP_ANON), with fd of dso and it's total
	 *  addr space foot-print (code+data).
	 *
	 *  Although we are interested only in code-part for ease of addr space
	 *  mgmt, because our allocator is unaware of any non-code mmaps, we
	 *  up reserving the full dso addr-space span in cmn-vaddr allocator
	 *  e.g. imagine process p1 mmaping dso1 and process p2 mapping dso2.
	 *  If our allco was only resev for code, there would be addr space
	 *  collision when code vaddr was alloc for dso2 (with data/anon mmaps
	 *  for dso1)
	 *
	 *  This would go away if user-space loader supports independently
	 *  relocatable lib segment - as opposed to a single load addr for
	 *  entire  dso. Anyhow the existing scheme doesn't hurt - we are
	 *  not wasting any vaddr.
	 */

	if (flags & MAP_SHARED_CODE ||	/* Callers aware of cmn mmap */
	    ((prot & PROT_EXEC) &&	/* others */
	     !(flags & MAP_FIXED) && !(prot & PROT_WRITE))) {
		if (mmapcode_alloc_vaddr
		    (file, pgoff, PAGE_ALIGN(len), &addr_hint) >= 0) {

			/* force this mmap to take the cmn-vaddr just alloc */
			flags |= MAP_FIXED;
		}

		/*
		 * if aboved failed because we ran out of cmn vaddr slots,
		 * fall-thru to normal mmap
		 */
	}
#endif

	vaddr = do_mmap_pgoff(file, addr_hint, len, prot, flags, pgoff);

#ifdef CONFIG_ARC_CMN_MMAP
	if (!(IS_ERR_VALUE(vaddr)) && (prot & PROT_EXEC)) {

		if (mmapcode_enab_vaddr(file, pgoff, PAGE_ALIGN(len), vaddr) ==
		    -3) {
			pr_notice("cmn-addr alloc err: svaddr[%lx] pvt%lx\n",
			       addr_hint, vaddr);
		}
	}
#endif

	return vaddr;
}

/* Gets dragged in due to __ARCH_WANT_SYSCALL_OFF_T */
SYSCALL_DEFINE6(mmap, unsigned long, addr_hint, unsigned long, len,
		unsigned long, prot, unsigned long, flags, unsigned long, fd,
		unsigned long, off)
{
	pr_err("old mmap not supported\n");
	return -EINVAL;
}

/*
 * cloned from stock munmap to have arch specific processing
 * This can go away once mm->unmap_area starts taking more args
 */
SYSCALL_DEFINE2(arc_munmap, unsigned long, addr, size_t, len)
{
	int ret;
	struct mm_struct *mm = current->mm;

	profile_munmap(addr);

	down_write(&mm->mmap_sem);
	ret = do_munmap(mm, addr, len);

#ifdef CONFIG_ARC_CMN_MMAP
	if (is_any_mmapcode_task_subscribed(mm))
		mmapcode_free(mm, addr, len);
#endif

	up_write(&mm->mmap_sem);
	return ret;
}

static inline int mmap_is_legacy(void)
{
#ifdef CONFIG_ARC_ADDR_SPACE_RND
	/* ELF loader sets this flag way early.
	 * So no need to check for multiple things like
	 *   !(current->personality & ADDR_NO_RANDOMIZE)
	 *   randomize_va_space
	 */
	return !(current->flags & PF_RANDOMIZE);
#else
	return 1;
#endif
}

static inline unsigned long arc_mmap_base(void)
{
	int rnd = 0;
	unsigned long base;

	/* 8 bits of randomness: 255 * 8k = 2MB */
	if (!mmap_is_legacy()) {
		rnd = (long)get_random_int() % (1 << 8);
		rnd <<= PAGE_SHIFT;
	}

	base = PAGE_ALIGN(TASK_UNMAPPED_BASE + rnd);

	return base;
}

void arch_pick_mmap_layout(struct mm_struct *mm)
{
	mm->mmap_base = arc_mmap_base();
	mm->get_unmapped_area = arch_get_unmapped_area;
	mm->unmap_area = arch_unmap_area;
}
EXPORT_SYMBOL_GPL(arch_pick_mmap_layout);

#ifdef CONFIG_ARC_CMN_MMAP

/********************************************************************
 *
 * Arch specific mmap quirk
 * Mapping lib at same vaddr across processes independent of loading
 * order by userspace dynamic loader
 *
 *******************************************************************/

/* Leave some head room for non-cmn mmaps
 */
#define ROOM_FOR_NON_CMN_MAPS   0x0F0F0000
#define MMAP_CODE_SPC_START     (TASK_UNMAPPED_BASE + ROOM_FOR_NON_CMN_MAPS)
#define MMAP_CODE_SPC_SZ        0x20000000	/* 512 MB */

/* For now track only 32 dsos */
#define MMAP_CODE_SPC_MAX       32
#define MMAP_CODE_SPC_MAX_IDX   (MMAP_CODE_SPC_MAX - 1)

/* The mmap code address space is managed in 3 data structures
 *  -A "pool" of addr space, using bitmaps - each bit corresp to a
 *   virtual page. Only deals with address space alloc/dealloc
 *  -the file/offset/len =>vaddr rlationship needs to be maintained
 *   to enable lookups by file
 *  -A pvt-only vaddr "x" can exist which doesn't correspond to a common
 *   mmap vaddr "x" (e.g. for tasks which don't use any shared libs).
 *   Thus need to track process subscription to common vaddrs.
 */

static struct gen_pool *mmapcode_addr_pool;

struct cmn_mmap_mapping {

	unsigned long ino;	/* inode corresp to sh lib mmaped */
	int refcnt;		/* num of times shared */

	struct {
		unsigned long pgoff;	/* offset in file mapped from */
		unsigned long len;	/* size of full dso */
		unsigned long vaddr;	/* start vaddr */
	} dso;

	/* Right now we only support one code-segment per dso */
	struct {
		unsigned long len;	/* len of code mapping */
		unsigned long vaddr;	/* vaddr of shared code */
	} code;
};

static struct cmn_mmap_mapping cmn_mappings[MMAP_CODE_SPC_MAX];

/* TODO:
 * -what if same file is mmaped via its symlink: will filp be same
 * -do we need locking: already called under mm semaphore
 */

/* Allocate a vaddr to a dso's code mmap, such that vaddr itself is usable
 * across processes (doesn't overlap with any pvt maps).
 * If the dso is already loaded, return the existing vaddr.
 *
 * As of now it allocates vaddr space to cover the entire dso, not just code.
 * This happens becoz we can't have a dedicated code vaddr space YET - becoz
 * this will mean that dso loading is split - uClibc dyn-loader (for ARC)
 * assumes a single virtual addr space for entire dso.
 */

int mmapcode_alloc_vaddr(struct file *filp, unsigned long pgoff,
			 unsigned long len, unsigned long *vaddr)
{
	struct cmn_mmap_mapping *db;
	int i, free_slot = -1;
	unsigned long ino, sh_vaddr;

	db = &cmn_mappings[MMAP_CODE_SPC_MAX_IDX];

	ino = filp->f_path.dentry->d_inode->i_ino;

	for (i = MMAP_CODE_SPC_MAX_IDX; i >= 0; i--, db--) {

		/* dso already loaded - mapped by some other process(es)
		 * returns it's global mapped vaddr
		 */
		if (db->ino == ino && db->dso.pgoff == pgoff &&
		    db->dso.len == len) {
			*vaddr = db->dso.vaddr;
			goto done;
		}

		/* remember a free slot in table
		 * XXX: len checked because valid inode-id can be zero ?
		 */
		if (free_slot == -1) {
			if (db->ino == 0 && db->dso.len == 0)
				free_slot = i;
		}
	}

	/* dso not registered, do it now */
	if (free_slot != -1) {

		db = &cmn_mappings[free_slot];

		/* alloc a block of addr space from pool */
		sh_vaddr = gen_pool_alloc(mmapcode_addr_pool, len);

		if (sh_vaddr != 0) {
			*vaddr = db->dso.vaddr = sh_vaddr;
			db->refcnt = 0;
			db->ino = ino;
			db->dso.pgoff = pgoff;
			db->dso.len = len;
			MMAP_DBG("ALLOC (%d) ino %lx, len %lx => [%lx]\n",
				 free_slot, ino, len, sh_vaddr);
			i = free_slot;
		}
	}

done:
	return i;
}

/* Called upon mmap for shared code itself - PROT_EXEC
 * The common vaddr has already been allocated, just enable it
 */
int mmapcode_enab_vaddr(struct file *filp, unsigned long pgoff,
			unsigned long len, unsigned long vaddr)
{
	struct cmn_mmap_mapping *db;
	int i;
	unsigned long ino;

	/* uClibc-libpthread does a anon mmap with prot-exec, which "mis-leads"
	 * in here
	 */
	if (!filp)
		return -2;

	ino = filp->f_path.dentry->d_inode->i_ino;
	db = &cmn_mappings[MMAP_CODE_SPC_MAX_IDX];

	for (i = MMAP_CODE_SPC_MAX_IDX; i >= 0; i--, db--) {

		/* An addr space already reserved for this dso */
		if (db->ino != ino)
			continue;

		if (likely(vaddr >= db->dso.vaddr && len <= db->dso.len)) {
			db->code.len = len;
			db->code.vaddr = vaddr;

			db->refcnt++;
			mmapcode_task_subscribe(current->mm, i);

			MMAP_DBG("ENAB #%d ino %lx, len %lx : [%lx] refs %d\n",
				     i, ino, len, vaddr, db->refcnt);

			return i;
		} else {
			return -3;	/* This is fatal */
		}
	}

	/* this inode (code) was not registered for cmd addr */
	return -1;
}

static void mmapcode_free_one(struct mm_struct *mm, struct cmn_mmap_mapping *db,
			      int sasid)
{
	mmapcode_task_unsubscribe(mm, sasid);

	db->refcnt--;
	BUG_ON(db->refcnt < 0);
	if (db->refcnt == 0) {
		/* Flush the cmn-code TLB entries
		 * the Probe semantics are same as that for global entries
		 * XXX: we can leave the non-code entries behind.
		 * Since process pvt ASID will get invalid, the entries will
		 * be used anyways
		 */
		local_flush_tlb_kernel_range(db->code.vaddr,
					     db->code.vaddr + db->code.len);
		gen_pool_free(mmapcode_addr_pool, db->dso.vaddr, db->dso.len);
		memset(db, 0, sizeof(*db));
	}
}

/* Called upon unmap of a cmn-vaddr egion */

int mmapcode_free(struct mm_struct *mm, unsigned long vaddr, unsigned long len)
{
	struct cmn_mmap_mapping *db;
	int i;

	db = &cmn_mappings[MMAP_CODE_SPC_MAX_IDX];

	for (i = MMAP_CODE_SPC_MAX_IDX; i >= 0; i--, db--) {

		/* check if vaddr being mmaped exist in com-mmap space
		 * and that task has subscribed to it
		 *    -it is perfectly legal for task to have pvt-only vaddr "x"
		 *     which also exists independently in cmn-mmap addr space
		 *     e.g. a task which doesn't user any cmn-mappings at all
		 */

		if (db->code.vaddr == vaddr && db->code.len == len &&
		    is_mmapcode_task_subscribed(mm, i)) {

			MMAP_DBG("UNMAP #%d ino %lx, len %lx : [%lx] refs %d\n",
				i, db->ino, len, vaddr, db->refcnt - 1);

			mmapcode_free_one(mm, db, i);
			break;
		}
	}

	return i;
}

/* Called upon task exit to relinquish all it's cmn-mmaps */

int mmapcode_free_all(struct mm_struct *mm)
{
	struct cmn_mmap_mapping *db;
	int i;

	db = &cmn_mappings[MMAP_CODE_SPC_MAX_IDX];

	for (i = MMAP_CODE_SPC_MAX_IDX; i >= 0; i--, db--) {

		if (is_mmapcode_task_subscribed(mm, i)) {

			MMAP_DBG("EXIT #%d ino %lx, [%lx] refs %d\n",
				 i, db->ino, db->code.vaddr, db->refcnt - 1);

			mmapcode_free_one(mm, db, i);
		}
	}

	return 0;
}

void __init mmapcode_space_init(void)
{
	pr_info("Common mmap addr-space starts %lx\n", MMAP_CODE_SPC_START);

	mmapcode_addr_pool = gen_pool_create(PAGE_SHIFT, -1);
	gen_pool_add(mmapcode_addr_pool, MMAP_CODE_SPC_START, MMAP_CODE_SPC_SZ,
		     -1);
}

/* Get rid of cmn-maps: Two scenarios
 * 1. sys_execve => flush_old_exec => mmput => exit_mmap => arch_exit_mmap
 * 2. do_exit    => mmput => exit_mmap => arch_exit_mmap
 */
void arch_exit_mmap(struct mm_struct *mm)
{
	if (is_any_mmapcode_task_subscribed(mm))
		mmapcode_free_all(mm);
}

/* Replicate the cmn-mmaps in case of fork
 * do_fork => dup_mm => dup_mmap => arch_dup_mmap
 */
void arch_dup_mmap(struct mm_struct *oldmm, struct mm_struct *mm)
{
	struct cmn_mmap_mapping *db;
	int i;

	db = &cmn_mappings[MMAP_CODE_SPC_MAX_IDX];

	for (i = MMAP_CODE_SPC_MAX_IDX; i >= 0; i--, db--) {

		if (is_mmapcode_task_subscribed(oldmm, i)) {

			mmapcode_task_subscribe(mm, i);
			db->refcnt++;

			MMAP_DBG("DUP #%d ino %lx, [%lx] refs %d\n",
				 i, db->ino, db->code.vaddr, db->refcnt);
		}
	}
}

/* Checks if the @vaddr contained in @mm belongs to a cmn-vaddr-space
 * instance.
 * If yes returns the "sasid" of that address space, else -1
 */
int mmapcode_find_mm_vaddr(struct mm_struct *mm, unsigned long vaddr)
{
	struct cmn_mmap_mapping *db;
	int i;

	if (is_any_mmapcode_task_subscribed(mm)) {

		db = &cmn_mappings[MMAP_CODE_SPC_MAX_IDX];

		for (i = MMAP_CODE_SPC_MAX_IDX; i >= 0; i--, db--) {

			if (is_mmapcode_task_subscribed(mm, i) &&
			    vaddr >= db->code.vaddr &&
			    vaddr < db->code.vaddr + db->code.len) {

				return i;
			}
		}
	}

	return -1;
}

#endif
