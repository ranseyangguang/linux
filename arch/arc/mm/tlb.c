/******************************************************************************
 * Copyright ARC International (www.arc.com) 2007-2009
 *
 * vineetg: April 2011 :
 *  -MMU v3: PD{0,1} bits layout changed: They don't overlap anymore,
        helps avoid a shift when preparing PD0 from PTE
 *  -CONFIG_ARC_MMU_SASID: support for ARC MMU Shared Address spaces
 *      update_mmu_cache can create shared TLB entries now.
 *
 * vineetg: April 2011 : Preparing for MMU V3
 *  -MMU v2/v3 BCRs decoded differently
 *  -Remove TLB_SIZE hardcoding as it's variable now: 256 or 512
 *  -tlb_entry_erase( ) can be void
 *  -local_flush_tlb_range( ):
 *      = need not "ceil" @end
 *      = walks MMU only if range spans < 32 entries, as opposed to 256
 *
 * Vineetg: Sept 10th 2008
 *  -Changes related to MMU v2 (Rel 4.8)
 *
 * Vineetg: Aug 29th 2008
 *    In TLB Flush operations (Metal Fix MMU) there is a explict command to
 *    flush Micro-TLBS. If TLB Index Reg is invalid prior to TLBIVUTLB cmd,
 *    it fails. Thus need to load it with ANY valid value before invoking
 *    TLBIVUTLB cmd
 *
 * Vineetg: Aug 21th 2008:
 *  -Reduced the duration of IRQ lockouts in TLB Flush routines
 *  -Multiple copies of TLB erase code seperated into a "single" function
 *
 * Vineetg: Aug 11th 2008:
 *    In TLB Flush routines, interrupt disabling moved UP to retrieve ASID
 *       in interrupt-safe region.
 *
 * Vineetg: April 23rd Bug #93131
 *    Problem: tlb_flush_kernel_range() doesnt do anything if the range to
 *              flush is more than the size of TLB itself.
 *    Fix: Flush the entire TLB as that is the only thing we can do
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
 * arch/arc/mm/tlb.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Authors: Rahul Trivedi
 *
 * a700 specific mmu/tlb code.
 */

#include <linux/module.h>
#include <asm/arcregs.h>
#include <asm/mmu_context.h>
#include <asm/system.h>
#include <asm/tlb.h>

/* Need for MMU v2
 *
 * ARC700 MMU-v1 has a Joint-TLB for Code and Data and is 2 way set-assoc.
 * For a memcpy operation with 3 players (src/dst/code) such that all 3 pages
 * map into same set, there would be contention for the 2 ways causing severe
 * Thrashing.
 *
 * Although J-TLB is 2 way set assoc, ARC700 caches J-TLB into uTLBS
 * The assoc of uTLB is much higer. u-D-TLB is 8 ways, u-I-TLB is 4 ways.
 * Given this, the thrasing problem should never happen because once the 3
 * J-TLB entries are created (even though 3rd will knock out one of the prev
 * two), the u-D-TLB and u-I-TLB will have what is required to accomplish memcpy
 *
 * Yet we still see the Thrashing because a J-TLB Write cause flush of u-TLBs.
 * This is a simple design for keeping them in sync. So what do we do?
 * The solution which James came up was pretty neat. It utilised the assoc
 * of uTLBs by not invalidating always but only when absolutely necessary.
 *
 *===========================================================================
 *
 * General fix: option 1 : New command for uTLB write without clearing uTLBs
 *
 * - Existing TLB commands work as before
 * - New command (TLBWriteNI) for TLB write without clearing uTLBs
 * - New command (TLBIVUTLB) to invalidate uTLBs.
 *
 * The uTLBs need only be invalidated when pages are being removed from the
 * OS page table. If a 'victim' TLB entry is being overwritten in the main TLB
 * as a result of a miss, the removed entry is still allowed to exist in the
 * uTLBs as it is still valid and present in the OS page table. This allows the
 * full associativity of the uTLBs to hide the limited associativity of the main
 * TLB.
 *
 * During a miss handler, the new "TLBWriteNI" command is used to load
 * entries without clearing the uTLBs.
 *
 * When the OS page table is updated, TLB entries that may be associated with a
 * removed page are removed (flushed) from the TLB using TLBWrite. In this
 * circumstance, the uTLBs must also be cleared. This is done by using the
 * existing TLBWrite command. An explicit IVUTLB is also required for those
 * corner cases when TLBWrite was not executed at all because the corresp
 * J-TLB entry got evicted/replaced.
 *
 *
 *===========================================================================
 *
 * For some customer this hardware change was not acceptable so a smaller
 * hardware change (dubbed Metal Fix) was done.
 *
 * Metal fix option 2 : New command for uTLB clearing
 *
 *  - The existing TLBWrite command no longer clears uTLBs.
 *  - A new command (TLBIVUTLB) is added to invalidate uTLBs.
 *
 * When the OS page table is updated, TLB entries that may be associated with a
 * removed page are removed (flushed) from the TLB using TLBWrite. In this
 * circumstance, the uTLBs must also be cleared. This is done explicitly with
 * the new TLBIVUTLB command (5).
 *
 * The TLBProbe command is used to find any TLB entries matching the affected
 * address. If an entry matches, it is removed by over-writing with a blank TLB
 * entry. To ensure that micro-TLBs are cleared, the new command TLBIVUTLB
 * is used to invalidate the micro-TLBs after all main TLB updates have been
 * completed. It is necessary to clear the micro-TLBs even if the TLBProbe found
 * no matching entries. With the metal-fix option 2 revised MMU, a page-table
 * entry can exist in micro-TLBs without being in the main TLB. TLBProbe does
 * not return results from the micro-TLBs. Hence to ensure micro-TLBs are
 * coherent with the OS page table, micro-TLBs must be cleared on removal or
 * alteration of any entry in the OS page table.
 */


/* Sameer: We have to define this as we are using asm-generic code for TLB.
           This will go once we stop using generic code. */
struct mmu_gather mmu_gathers;

/* A copy of the ASID from the PID reg is kept in asid_cache */
volatile int asid_cache;


/* ASID to mm struct mapping. We have one extra entry corresponding to
 * NO_ASID to save us a compare when clearing the mm entry for old asid
 * see get_new_mmu_context (asm-arc/mmu_context.h)
 */
struct mm_struct *asid_mm_map[NUM_ASID + 1];

/* Needed to avoid Cache aliasing */
unsigned int ARC_shmlba;

static struct cpuinfo_arc_mmu *mmu = &cpuinfo_arc700[0].mmu;


/*=========================================================================
 * ARC700 MMU has 256 J-TLB  entries organised as 128 SETs with 2 WAYs each.
 * For TLB Operations, Linux uses "HW" Index to write entry into a particular
 * location, which is a linear value from 0 to 255.
 *
 * MMU converts it into Proper SET+WAY
 * e.g. Index 5 is SET: 5/2 = 2 and WAY: 5%2 = 1.
 *
*=========================================================================*/

/****************************************************************************
 * Utility Routine to erase a J-TLB entry
 * The procedure is to look it up in the MMU. If found, ERASE it by
 *  issuing a TlbWrite CMD with PD0 = PD1 = 0
 ****************************************************************************/
static void tlb_entry_erase(unsigned int vaddr_n_asid)
{
    unsigned int idx;

    /* Locate the TLb entry for this vaddr + ASID */
    write_new_aux_reg(ARC_REG_TLBPD0, vaddr_n_asid);
    write_new_aux_reg(ARC_REG_TLBCOMMAND, TLBProbe);
    idx = read_new_aux_reg(ARC_REG_TLBINDEX);

    /* No error means entry found, zero it out */
    if (likely(!(idx & TLB_LKUP_ERR))) {
        write_new_aux_reg(ARC_REG_TLBPD1, 0);
        write_new_aux_reg(ARC_REG_TLBPD0, 0);
        write_new_aux_reg(ARC_REG_TLBCOMMAND, TLBWrite);
    }
    else {  /* Some sort of Error */

        /* Duplicate entry error */
        if ( idx & 0x1 ) {
            // TODO we need to handle this case too
            printk("#### unhandled Duplicate flush for %x\n", vaddr_n_asid);
        }
        else {
            /* else entry not found so nothing to do */
        }
    }
}

/****************************************************************************
 * ARC700 MMU caches recently used J-TLB entries (RAM) as uTLBs (FLOPs)
 *
 * New IVUTLB cmd in MMU v2 explictly invalidates the uTLB
 *
 * utlb_invalidate ( )
 *  -For v2 MMU calls Flush uTLB Cmd
 *  -For v1 MMU does nothing (except for Metal Fix v1 MMU)
 *      This is because in v1 TLBWrite itself invalidate uTLBs
 ***************************************************************************/

static void utlb_invalidate(void)
{
#if (METAL_FIX || CONFIG_ARC_MMU_VER >= 2)
    {
        unsigned int idx;

        /* make sure INDEX Reg is valid */
        idx = read_new_aux_reg(ARC_REG_TLBINDEX);

        /* If not write some dummy val */
        if (unlikely(idx & TLB_LKUP_ERR)) {
            write_new_aux_reg(ARC_REG_TLBINDEX, 0xa);
        }

        write_new_aux_reg(ARC_REG_TLBCOMMAND, TLBIVUTLB);
    }
#endif

}

/*
 * Un-conditionally (without lookup) erase the entire MMU contents
 */

void noinline local_flush_tlb_all(void)
{
    unsigned long flags;
    unsigned int entry;

    take_snap(SNAP_TLB_FLUSH_ALL, 0, 0);

    local_irq_save(flags);

    /* Load PD0 and PD1 with template for a Blank Entry */
    write_new_aux_reg(ARC_REG_TLBPD1, 0);
    write_new_aux_reg(ARC_REG_TLBPD0, 0);

    for (entry = 0; entry < mmu->num_tlb; entry++) {
        /* write this entry to the TLB */
        write_new_aux_reg(ARC_REG_TLBINDEX, entry);
        write_new_aux_reg(ARC_REG_TLBCOMMAND, TLBWrite);
    }

    utlb_invalidate();

    local_irq_restore(flags);
}

/*
 * Flush the entrie MM for userland. The fastest way is to move to Next ASID
 */
void noinline local_flush_tlb_mm(struct mm_struct *mm)
{
    get_new_mmu_context(mm, 1);
}

/*
 * Flush a Range of TLB entries for userland.
 * @start is inclusive, while @end is exclusive
 * Difference between this and Kernel Range Flush is
 *  -Here the fastest way (if range is too large) is to move to next ASID
 *      without doing any explicit Shootdown
 *  -In case of kernel Flush, entry has to be shot down explictly
 */
void local_flush_tlb_range(struct vm_area_struct *vma, unsigned long start,
               unsigned long end)
{
    unsigned long flags;

    /* If range @start to @end is more than 32 TLB entries deep,
     * its better to move to a new ASID rather than searching for
     * individual entries and then shooting them down
     *
     * The calc above is rough, doesn;t account for unaligned parts,
     * since this is heuristics based anyways
     */
    if (likely((end - start) < PAGE_SIZE * 32)) {

        /* @start moved to page start: this alone suffices for checking loop
         * end condition below, w/o need for aligning @end to page end
         * e.g. 2000 to 4001 will anyhow loop twice
         */
        start &= PAGE_MASK;

        local_irq_save(flags);

        if (vma->vm_mm->context.asid != NO_ASID) {
            while (start < end) {
                tlb_entry_erase(start | (vma->vm_mm->context.asid & 0xff));
                start += PAGE_SIZE;
            }
        }

        utlb_invalidate();

        local_irq_restore(flags);
    }
    else {
        local_flush_tlb_mm(vma->vm_mm);
    }

}

/* Flush the kernel TLB entries - vmalloc/modules (Global from MMU perspective)
 *  @start, @end interpreted as kvaddr
 * Interestingly, shared TLB entries can also be flushed using just
 * @start,@end alone (interpreted as user vaddr), although technically SASID
 * is also needed. However our smart TLbProbe lookup takes care of that.
 */
void local_flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
    unsigned long flags;

    /* exactly same as above, except for TLB entry not taking ASID */

    if (likely((end - start) < PAGE_SIZE * 32)) {
        start &= PAGE_MASK;

        local_irq_save(flags);
        while (start < end) {
            tlb_entry_erase(start);
            start += PAGE_SIZE;
        }

        utlb_invalidate();

        local_irq_restore(flags);
    }
    else {
        local_flush_tlb_all();
    }
}

/*
 * Delete TLB entry in MMU for a given page (??? address)
 * NOTE One TLB entry contains translation for single PAGE
 */

void local_flush_tlb_page(struct vm_area_struct *vma, unsigned long page)
{
    unsigned long flags;

    /* Note that it is critical that interrupts are DISABLED between
     * checking the ASID and using it flush the TLB entry
     */
    local_irq_save(flags);

    if (vma->vm_mm->context.asid != NO_ASID) {
        tlb_entry_erase((page & PAGE_MASK) | (vma->vm_mm->context.asid & 0xff));
        utlb_invalidate();
    }

    local_irq_restore(flags);
}

/*
 * Routine to create a TLB entry
 */
void create_tlb(struct vm_area_struct *vma, unsigned long address, pte_t *ptep)
{
    unsigned long flags;
    unsigned int idx, asid_or_sasid;
    unsigned long pd0_flags;

    local_irq_save(flags);

#ifdef CONFIG_ARC_TLB_PARANOIA
    /* vineetg- April 2008
     * Diagnostic Code to verify if sw and hw ASIDs are in lockstep
     */
    {
        unsigned int pid_sw, pid_hw;

        void print_asid_mismatch(int is_fast_path);

        pid_sw = vma->vm_mm->context.asid;
        pid_hw = read_new_aux_reg(ARC_REG_PID) & 0xff;

        if (address < 0x70000000 &&
            ((pid_hw != pid_sw) || (pid_sw == NO_ASID)))
        {
            print_asid_mismatch(0);
        }
    }
#endif

    address &= PAGE_MASK;

    /* update this PTE credentials */
    pte_val(*ptep) |= (_PAGE_PRESENT | _PAGE_ACCESSED);

#if (CONFIG_ARC_MMU_VER <= 2)
    /* Create HW TLB entry Flags (in PD0) from PTE Flags */
    pd0_flags = ((pte_val(*ptep) & PTE_BITS_IN_PD0) >> 1);
#else
    pd0_flags = ((pte_val(*ptep) & PTE_BITS_IN_PD0));
#endif

#ifdef CONFIG_ARC_MMU_SASID
    if (pte_val(*ptep) & _PAGE_SHARED_CODE) {

        unsigned int tsk_sasids;

        pd0_flags |= _PAGE_SHARED_CODE_H;

        /* SASID for this vaddr mapping */
        asid_or_sasid = pte_val(*(ptep + PTRS_PER_PTE));

        /* All the SASIDs for this task */
        tsk_sasids = is_any_mmapcode_task_subscribed(vma->vm_mm);
        BUG_ON(tsk_sasids == 0);

        write_new_aux_reg(ARC_REG_SASID, tsk_sasids);
    } else
#endif
        /* ASID for this task */
        asid_or_sasid = read_new_aux_reg(ARC_REG_PID) & 0xff;


    write_new_aux_reg(ARC_REG_TLBPD0, address|pd0_flags|asid_or_sasid);

    /* Load remaining info in PD1 (Page Frame Addr and Kx/Kw/Kr Flags etc) */
    write_new_aux_reg(ARC_REG_TLBPD1, (pte_val(*ptep) & PTE_BITS_IN_PD1));

    /* First verify if entry for this vaddr+ASID already exists */
    write_new_aux_reg(ARC_REG_TLBCOMMAND, TLBProbe);
    idx = read_new_aux_reg(ARC_REG_TLBINDEX);

    /* If Not already present get a free slot from MMU.
     * Otherwise, Probe would have located the entry and set INDEX Reg
     * with existing location. This will cause Write CMD to over-write
     * existing entry with new PD0 and PD1
     */
    if (likely(idx & TLB_LKUP_ERR)) {
        write_new_aux_reg(ARC_REG_TLBCOMMAND, TLBGetIndex);
    }

    /* Commit the Entry to MMU
     * It doesnt sound safe to use the TLBWriteNI cmd here
     * which doesn't flush uTLBs. I'd rather be safe than sorry.
     */
    write_new_aux_reg(ARC_REG_TLBCOMMAND, TLBWrite);

    local_irq_restore(flags);
}

/*
 * Things to do when a new PTE is entered in Page Tables or an existing one
 * is modified.
 * (1) for VIPT caches, deal with Cache flushing.
 * (2) Also as an optimisation, create/modify the TLB entry corresponding
 *    to vaddr+pfn. This routine, most likely got called as a result of
 *    TLB Miss or Prot Violation (to break COW) with fast Path TLB refill code
 *    unable to create the TLB.
 *   TLB Miss => page fault hdlr => VM code => chg PTE => update_mmu_cache()
 *   So it provides an opportunity to create the TLB. Otherwise if we simply fix
 *   the VM data structures (Page Tables) in this control flow and return away
 *   from page fault handler, SAME TLB Miss will be taken again, and
 *   Fast Path TLB Refill code will create the TLB. Creating the TLB here will
 *   save one TLB Miss exception
 */

void update_mmu_cache(struct vm_area_struct *vma, unsigned long vaddress, pte_t *ptep)
{
    /* XXX: This definitely seems useless */
    // BUG_ON(!pfn_valid(pte_pfn(*ptep)));

    /* XXX: This is useful - but only once during execve - check why?
     *  handle_mm_fault()
     *      __get_user_pages
     *          copy_strings()
     *              do_execve()
     */
    if (current->active_mm != vma->vm_mm)
        return;

    create_tlb(vma, vaddress, ptep);

    flush_cache_page(vma, vaddress, pfn);
}


/* Read the Cache Build Confuration Registers, Decode them and save into
 * the cpuinfo structure for later use.
 * No Validation is done here, simply read/convert the BCRs
 */
void __init read_decode_mmu_bcr(void)
{
    unsigned int tmp;
    struct bcr_mmu_1_2  *mmu2;      // encoded MMU2 attributes
    struct bcr_mmu_3    *mmu3;      // encoded MMU3 attributes
    struct cpuinfo_arc_mmu *mmu;    // simplified attributes

    mmu = &cpuinfo_arc700[0].mmu;

    tmp = read_new_aux_reg(ARC_REG_MMU_BCR);
    mmu->ver = (tmp >>24);

    if (mmu->ver <= 2) {
        mmu2 = (struct bcr_mmu_1_2 *)&tmp;
        mmu->pg_sz = PAGE_SIZE;
        mmu->sets = 1 << mmu2->sets;
        mmu->ways = 1 << mmu2->ways;
        mmu->u_dtlb = mmu2->u_dtlb;
        mmu->u_itlb = mmu2->u_itlb;
    }
    else {
        mmu3 = (struct bcr_mmu_3 *)&tmp;
        mmu->pg_sz = 512 << mmu3->pg_sz;
        mmu->sets = 1 << mmu3->sets;
        mmu->ways = 1 << mmu3->ways;
        mmu->u_dtlb = mmu3->u_dtlb;
        mmu->u_itlb = mmu3->u_itlb;
    }

    mmu->num_tlb = mmu->sets * mmu->ways;

}

char * arc_mmu_mumbojumbo(int cpu_id, char *buf)
{
    int num=0;
    struct cpuinfo_arc_mmu *p_mmu = &cpuinfo_arc700[0].mmu;

    num += sprintf(buf+num, "ARC700 MMU Ver [%x]\n",p_mmu->ver);

    num += sprintf(buf+num, "   PAGE SIZE %x\n",p_mmu->pg_sz);
    num += sprintf(buf+num, "   JTLB %d x %d = %d entries\n",
                        p_mmu->sets, p_mmu->ways, p_mmu->num_tlb);
    num += sprintf(buf+num, "   uDTLB %d entr, uITLB %d entr\n",
                        p_mmu->u_dtlb, p_mmu->u_itlb);
    num += sprintf(buf+num,"TLB Refill \"will %s\" Flush uTLBs\n",
                        p_mmu->ver >= 2 ? "NOT":"");
	return buf;
}

void __init arc_mmu_init(void)
{
    int i;
    static int one_time_init;
    char str[512];

    printk(arc_mmu_mumbojumbo(0, str));

    /* For efficiency sake, kernel is compile time built for a MMU ver
     * This must match the hardware it is running on.
     * Linux built for MMU V2, if run on MMU V1 will break down because V1
     *  hardware doesn't understand cmds such as WriteNI, or IVUTLB
     * On the other hand, Linux built for V1 if run on MMU V2 will do
     *   un-needed workarounds to prevent memcpy thrashing.
     * Similarly MMU V3 has new features which won't work on older MMU
     */
    if (mmu->ver != CONFIG_ARC_MMU_VER) {
        panic("MMU ver %d doesn't match kernel built for %d...\n",
            mmu->ver, CONFIG_ARC_MMU_VER);
    }

    /* Setup data structures for ASID management */
    if ( ! one_time_init ) {

        asid_cache = FIRST_ASID;

        /* clear asid to mm map table */
        for (i = 0; i < MAX_ASID; i++) {
            asid_mm_map[i] = (struct mm_struct *)NULL;
        }

        one_time_init = 1;
    }

    local_flush_tlb_all();

    /* Enable the MMU */
    write_new_aux_reg(ARC_REG_PID, MMU_ENABLE);

#ifndef CONFIG_SMP    // In smp we use this reg for interrupt 1 scratch

    /* swapper_pg_dir is the pgd for the kernel, used by vmalloc */
    write_new_aux_reg(ARC_REG_SCRATCH_DATA0, swapper_pg_dir);
#endif
}

/* Duplicate TLB entry means software gone wrong
 * Old code tried to fix it, by removing one of the entires, that is wrong
 * Lets be pedantic and call it over
 */
void do_tlb_overlap_fault(unsigned long cause, unsigned long address,
                  struct pt_regs *regs)
{
    panic("EV_MachineCheck : Duplicate PD in TLB\n");
}

/***********************************************************************
 * Diagnostic Routines
 *  -Called from Low Level TLB Hanlders if things don;t look good
 **********************************************************************/

#ifdef CONFIG_ARC_TLB_PARANOIA

/*
 * Low Level ASM TLB handler calls this if it finds that HW and SW ASIDS
 * don't match
 */
void print_asid_mismatch(int is_fast_path)
{
    int pid_sw, pid_hw;
    pid_sw = current->active_mm->context.asid;
    pid_hw = read_new_aux_reg(ARC_REG_PID) & 0xff;

#ifdef CONFIG_ARC_DBG_EVENT_TIMELINE
    sort_snaps(1);
#endif

    printk("ASID Mismatch in %s Path Handler: sw-pid=0x%x hw-pid=0x%x\n",
                is_fast_path ? "Fast" : "Slow",
                pid_sw, pid_hw);

    __asm__ __volatile__ ("flag 1");
}

void print_pgd_mismatch(void)
{
    extern struct task_struct *_current_task;

    unsigned int reg = read_new_aux_reg(ARC_REG_SCRATCH_DATA0);
    printk("HW PDG %x  SW PGD %p CURR PDG %p\n", reg, current->active_mm->pgd,
                    _current_task->active_mm->pgd);

    __asm__ __volatile__ ("flag 1");
}

#endif


/***********************************************************************
 * Dumping Debug Routines
 **********************************************************************/

#ifdef CONFIG_ARC_TLB_DBG_V

typedef struct {
    unsigned int pd0, pd1;
    int idx;
}
ARC_TLB_T;

/*
 * Read TLB entry from a particulat index in MMU
 * Caller "MUST" Disable interrupts
 */
void fetch_tlb(int idx, ARC_TLB_T *ptlb)
{
    write_new_aux_reg(ARC_REG_TLBINDEX, idx);
    write_new_aux_reg(ARC_REG_TLBCOMMAND, TLBRead);
    ptlb->pd0 = read_new_aux_reg(ARC_REG_TLBPD0);
    ptlb->pd1 = read_new_aux_reg(ARC_REG_TLBPD1);
    ptlb->idx = idx;
}

/*
 * Decode HW Page Descriptors in "Human understandable form"
 */
void decode_tlb(ARC_TLB_T *ptlb)
{
    printk("# %2x # ", ptlb->idx);

    if (!(ptlb->pd0 & 0x400)) {
        printk("\n");
        return;
    }

    printk("PD0: %s %s ASID %3d  V-addr %8x",
                        (ptlb->pd0 & 0x100) ? "G" : " ",
                        (ptlb->pd0 & 0x400) ? "V" : " ",
                        (ptlb->pd0 & 0xff), (ptlb->pd0 & ~0x1FFF));

    printk(" PD1: %s %s %s %s %s %s %s PHY %8x\n",
                        (ptlb->pd1 & 0x100) ? "KR" : "  ",
                        (ptlb->pd1 & 0x80) ? "KW"  : "  ",
                        (ptlb->pd1 & 0x40) ? "KX"  : "  ",
                        (ptlb->pd1 & 0x20) ? "UR"  : "  ",
                        (ptlb->pd1 & 0x10) ? "UW"  : "  ",
                        (ptlb->pd1 & 0x8) ? "UX"   : "  ",
                        (ptlb->pd1 & 0x4) ? "C "   : "NC",
                        (ptlb->pd1 & ~0x1FFF));
}

void print_tlb_at_mmu_idx(int idx)
{
    ARC_TLB_T  tlb;

    fetch_tlb(idx, &tlb);
    decode_tlb(&tlb);
}

ARC_TLB_T arr[512];
ARC_TLB_T micro[12];

/*
 * Read TLB Entry from MMU for range of virtual address : @start to @vend
 *
 * Note interrupts are disabled to get a consistent snapshot of MMU,
 *  so that no changes can be made while we read the entirity
 *
 * Also we dont print in this routine because printk can cause lot of side
 *  effects including IRQ enabling, reschedule etc
 * There is a seperate routine to dump a snapshot.
 *
 * JTLB entries are indexed from 0 to 255 (independent of geometry)
 * Micro-iTLB entries 0x200 to 0x203
 * Micro-dTLB entries 0x400 to 0x407
 */
void mmu_snapshot(unsigned int vstart, unsigned int vend)
{
    int ctr=0, idx, i;
    unsigned int flags;

    local_irq_save(flags);

    memset(arr, 0, sizeof(arr));
    memset(micro, 0, sizeof(micro));

    // This is the set index (0 to 127), not MMU index
    idx = ( vstart / 8192 ) % 128;
    // MMU index (o to 255) Way 0
    idx *= 2;

    do
    {
        fetch_tlb(idx++, &arr[ctr++]);

        // MMU index (o to 255) Way 1
        fetch_tlb(idx++, &arr[ctr++]);

        vstart += PAGE_SIZE;
    }
    while (vend >= vstart);

    ctr = 0;
    for ( i = 0x200; i <= 0x203; i++)  {
        fetch_tlb(i, &micro[ctr++]);
    }

    ctr = 4;  // Four I-uTLBs above
    for ( i = 0x400; i <= 0x407; i++) {
        fetch_tlb(i, &micro[ctr++]);
    }

    local_irq_restore(flags);
}

/*
 * Dump a previously taken snapshot of MMU
 */
void dump_mmu_snapshot()
{
    int i, ctr = 0;

    printk("###J-TLB Entry for MMU\n");

    for ( i = 0; i < 32; i++) {
        decode_tlb(&arr[i]);
    }

    ctr = 0;
    printk("***Dumping Micro I-TLBs\n");
    for ( i = 0; i < 4; i++) {
        decode_tlb(&micro[ctr++]);
    }

    printk("***Dumping Micro D-TLBs\n");
    for ( i = 0; i < 8; i++) {
        decode_tlb(&micro[ctr++]);
    }

}


/*
 * Print both HW and SW Translation entries for a given virtual address
 */
void print_virt_to_phy(unsigned int vaddr, int hw_only)
{
    unsigned long addr = (unsigned long) vaddr;
    struct task_struct *tsk = current;
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *ptep, pte;
    unsigned int pte_v, phy_addr;
    int pass = 1;

    if (hw_only) goto TLB_LKUP;

    pgd = pgd_offset(tsk->active_mm, addr);
    printk("========Virtual Addr %x  PGD Ptr %p=====\n",
                    vaddr, tsk->active_mm->pgd);

PGTBL_2ND_PASS:
    if (pass == 2) {
        pgd = pgd_offset_k(addr);
        printk("========[KERNEL] Virtual Addr %x  PGD Ptr %p=====\n",
                    vaddr, init_mm.pgd);
    }

    if (!pgd_none(*pgd)) {
        pud = pud_offset(pgd, addr);
        if (!pud_none(*pud)) {
            pmd = pmd_offset(pud, addr);
            if (!pmd_none(*pmd)) {
                ptep = pte_offset(pmd, addr);
                pte = *ptep;
                if (pte_present(pte)) {
                    pte_v = *((unsigned int*)&pte);
                    printk("  PTE Val %x", pte_v);
                    printk("  Flags %x, PHY %x\n",(pte_v & 0x1FFF), (pte_v & ~0x1FFF));
                    // Use this to print from phy addr correspondign to vaddr
                    phy_addr = (pte_v & ~0x1FFF) + (vaddr & 0x1FFF);
                    goto TLB_LKUP;
                }
            }
        }
    }

    printk("PTE Not present\n");

TLB_LKUP:

    if ( pass == 1 ) {
        pass = 2;
        goto PGTBL_2ND_PASS;
    }

    mmu_snapshot(vaddr, vaddr+PAGE_SIZE);
    dump_mmu_snapshot();

}

EXPORT_SYMBOL(print_virt_to_phy);

#endif
