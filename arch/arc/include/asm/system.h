/******************************************************************************
 * Copyright ARC International (www.arc.com) 2007-2009
 *
 * Vineetg: Mar 2009 (Critical look at file)
 *  -Improved local_irq_save() and friends to not use r20 (CALLLEE Saved)
 *      but let the compiler do register allocation
 *  -[mask|unmask]_interrupt( ) need not do inline IRQ disable as caller holds
 *      spin_lock_irqsave( ) before invoking them
 *
 * Vineetg: Mar 2009 (Supporting 2 levels of Interrupts)
 *  -irqs_enabled( ) need to consider both L1 and L2
 *  -local_irq_enable shd be cognizant of current IRQ state
 *    and must not re-enable lower priorty IRQs
 *   It is lot more involved now and needs to be written in "C"
 *
 * Vineetg: Feb 2009
 *  -schedular hook prepare_arch_switch( ) is now macro instead of inline func
 *   so that builtin_ret_addr( ) can correctly identify the caller of schdule()
 *
 * Vineetg: Oct 3rd 2008
 *  -Got rid of evil cli()/sti() from architecture code
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
 *  linux/include/asm-arc/system.h
 *
 *  Copyright (C)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Authors : Amit Bhor, Sameer Dhavale
 */

#ifndef __ASM_ARC_SYSTEM_H
#define __ASM_ARC_SYSTEM_H

#include <asm/arcregs.h>
#include <asm/ptrace.h>


#ifndef __ASSEMBLY__

#ifdef __KERNEL__

/******************************************************************
 * IRQ Control Macros
 ******************************************************************/

/*
 * Save IRQ state and disable IRQs
 */
#define local_irq_save(x) { x = _local_irq_save(); }
static inline long _local_irq_save(void) {
    unsigned long temp, flags;

    __asm__ __volatile__ (
        "lr  %0, [status32]\n\t"
        "mov %1, %0\n\t"
        "and %0, %0, %2\n\t"
        "flag %0\n\t"
        :"=&r" (temp), "=r" (flags)
        :"n" (~(STATUS_E1_MASK | STATUS_E2_MASK))
    );

    return flags;
}

/*
 * restore saved IRQ state
 */
static inline void local_irq_restore(unsigned long flags) {

    __asm__ __volatile__ (
        "flag %0\n\t"
        :
        :"r" (flags)
    );
}

/*
 * Conditionally Enable IRQs
 */
extern void local_irq_enable(void);

/*
 * Unconditionally Disable IRQs
 */
static inline void local_irq_disable(void) {
    unsigned long temp;

    __asm__ __volatile__ (
        "lr  %0, [status32]\n\t"
        "and %0, %0, %1\n\t"
        "flag %0\n\t"
        :"=&r" (temp)
        :"n" (~(STATUS_E1_MASK | STATUS_E2_MASK))
    );
}

/*
 * save IRQ state
 */
#define local_save_flags(x) { x = _local_save_flags(); }
static inline long _local_save_flags(void) {
    unsigned long temp;

    __asm__ __volatile__ (
        "lr  %0, [status32]\n\t"
        :"=&r" (temp)
    );

    return temp;
}

/*
 * mask/unmask an interrupt (@x = IRQ bitmap)
 * e.g. to Disable IRQ 3 and 4, pass 0x18
 *
 * mask = disable IRQ = CLEAR bit in AUX_I_ENABLE
 * unmask = enable IRQ = SET bit in AUX_I_ENABLE
 */

#define mask_interrupt(x)  __asm__ __volatile__ (   \
    "lr r20, [auxienable] \n\t"                     \
    "and    r20, r20, %0 \n\t"                      \
    "sr     r20,[auxienable] \n\t"                  \
    :                                               \
    :"r" (~(x))                                     \
    :"r20", "memory")

#define unmask_interrupt(x)  __asm__ __volatile__ ( \
    "lr r20, [auxienable] \n\t"                     \
    "or     r20, r20, %0 \n\t"                      \
    "sr     r20, [auxienable] \n\t"                 \
    :                                               \
    :"r" (x)                                        \
    :"r20", "memory")

/*
 * Query IRQ state
 */
static inline int irqs_disabled(void)
{
    unsigned long flags;
    local_save_flags(flags);
    return (!(flags & (STATUS_E1_MASK
#ifdef CONFIG_ARCH_ARC_LV2_INTR
                        | STATUS_E2_MASK
#endif
            )));
}


/******************************************************************
 * Barriers
 ******************************************************************/

//TODO-vineetg: Need to see what this does, dont we need sync anywhere
#define mb() __asm__ __volatile__ ("" : : : "memory")
#define rmb() mb()
#define wmb() mb()
#define set_mb(var, value)  do { var = value; mb(); } while (0)
#define set_wmb(var, value) do { var = value; wmb(); } while (0)

/* TODO-vineetg verify the correctness of macros here */
#ifdef CONFIG_SMP
#define smp_mb()        mb()
#define smp_rmb()       rmb()
#define smp_wmb()       wmb()
#else
#define smp_mb()        barrier()
#define smp_rmb()       barrier()
#define smp_wmb()       barrier()
#endif

#define smp_read_barrier_depends()      do { } while(0)

/******************************************************************
 * Arch Depenedent Context Switch Macro  called by sched
 * This in turn calls the regfile switching macro __switch_to ( )
 ******************************************************************/
struct task_struct; // to prevent cyclic dependencies

/* switch_to macro based on the ARM implementaion */
extern struct task_struct *__switch_to(struct task_struct *prev,
                                    struct task_struct *next);

#define switch_to(prev, next, last)         \
{                                           \
    do {                                    \
        last = __switch_to( prev, next);    \
        mb();                               \
                                            \
    }while (0);                             \
}

/* Hook into Schedular to be invoked prior to Context Switch
 *  -If ARC H/W profiling enabled it does some stuff
 *  -If event logging enabled it takes a event snapshot
 *
 *  Having a funtion would have been cleaner but to get the correct caller
 *  (from __builtin_return_address) it needs to be inline
 */

/* Things to do for event logging prior to Context switch */
#ifdef CONFIG_ARC_DBG_EVENT_TIMELINE
#define PREP_ARCH_SWITCH_ACT1(next)                                 \
do {                                                                \
    if (next->mm)                                                   \
        take_snap(SNAP_PRE_CTXSW_2_U,                               \
                      (unsigned int) __builtin_return_address(0),   \
                      current_thread_info()->preempt_count);        \
    else                                                            \
        take_snap(SNAP_PRE_CTXSW_2_K,                               \
                      (unsigned int) __builtin_return_address(0),   \
                      current_thread_info()->preempt_count);        \
}                                                                   \
while(0)
#else
#define PREP_ARCH_SWITCH_ACT1(next)
#endif


/* Things to do for hardware based profiling prior to Context Switch */
#ifdef CONFIG_ARC_PROFILING
extern void arc_ctx_callout(struct task_struct *next);
#define PREP_ARCH_SWITCH_ACT2(next)    arc_ctx_callout(next)
#else
#define PREP_ARCH_SWITCH_ACT2(next)
#endif

/* This def is the one used by schedular */
#define prepare_arch_switch(next)   \
do {                                \
    PREP_ARCH_SWITCH_ACT1(next);     \
    PREP_ARCH_SWITCH_ACT2(next);    \
}                                   \
while(0)


/******************************************************************
 * Miscll stuff
 ******************************************************************/

/*
 * On SMP systems, when the scheduler does migration-cost autodetection,
 * it needs a way to flush as much of the CPU's caches as possible.
 *
 * TODO: fill this in!
 */
static inline void sched_cacheflush(void)
{
}

extern inline unsigned long __xchg (unsigned long with,
                                    __volatile__ void *ptr, int size)
{
    __asm__ __volatile__ (" ex  %0, [%1]"
                  : "=r" (with),"=r"(ptr)
                  : "0" (with), "1" (ptr)
                  : "memory" );

    return (with);
}

#define xchg(ptr, with) \
  ((__typeof__ (*(ptr)))__xchg ((unsigned long)(with), (ptr), sizeof (*(ptr))))

#define arch_align_stack(x) (x)

/******************************************************************
 * Piggyback stuff
 * #defines/headers to be made avail to rest of code w/o explicit include
 * e.g. to call event log macros from any kernel file w/o including
 *     eventlog.h in that file
 ******************************************************************/


#include <plat_memmap.h>    // Peripherals Memory Map
#include <asm/event-log.h>  // event log from "C"

void show_stacktrace(struct task_struct *tsk, struct pt_regs *regs);
void raw_printk(const char *str, unsigned int num);
void raw_printk5(const char *str, unsigned int n1, unsigned int n2,
                    unsigned int n3, unsigned int n4);

/***** Diagnostic routines *******/

void show_fault_diagnostics(const char *str, struct pt_regs *regs,
    struct callee_regs *cregs, unsigned long address, unsigned long cause_reg);

#define show_kernel_fault_diag(a,b,c,d,e) show_fault_diagnostics(a,b,c,d,e)

/* So that LMBench numbers for Signal handling are not affected
 * by diagnostic stuff
 */
#ifdef CONFIG_ARC_USER_FAULTS_DBG
#define show_user_fault_diag(a,b,c,d,e) show_fault_diagnostics(a,b,c,d,e)
#else
#define show_user_fault_diag(a,b,c,d,e)
#endif

/******************************************************************
 * printk calls in __init code, so that their literal strings go into
 * .init.rodata (which gets reclaimed) instead of in .rodata
 ******************************************************************/
#define INIT_PRINT 0

#if (INIT_PRINT == 2)
#define printk_init(fmt, args...) 	printk(fmt, ## args)
#elif (INIT_PRINT == 1)
#define printk_init(fmt, args...)
#else
#define printk_init(fmt, args...) 					\
({ 													\
	static const __initconst char __fmt[] = fmt; 	\
	printk(__fmt, ## args); 						\
})
#endif

#endif /*__KERNEL__*/

#else  /* !__ASSEMBLY__ */

#include <asm/event-log.h>  // event log from Assembly


#endif /* __ASSEMBLY__ */


#endif /* ASM_ARC_SYSTEM_H */
