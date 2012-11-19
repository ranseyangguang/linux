/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * vineetg: March 2010
 *  -Rewrote atomic/non-atomic test_and_(set|clear|change)_bit and
 *           (set|clear|change)_bit APIs
 *  -Removed the redundant loads due to multiple ops on volatile args
 *  -Convert non-atomic APIs to "C" for better insn scheduling
 *  -take adv of fact that ARC bit fidding insn (bset/bclr/asl) etc only
 *   use bottom 5 bits of bit pos, avoiding the need to mask them off
 *
 * vineetg: March 2010
 *  -optimised ARC versions of ffz, ffs, fls
 *  -fls in particular is now loopless based on ARC norm insn
 *  - Also all such functions made "const" for gcc to enable CSE/LICM
 *  -find_first(|_zero)_bit no longer based on find_next(|_zero)_bit
 *
 * Vineetg: July 2009 (EXT2 bitops API optimisation)
 *	-Atomic API no longer call spin_lock as we are Uni-processor
 *	-Non Atomix API no longer disables interrupts
 *
 * Amit Bhor, Sameer Dhavale: Codito Technologies 2004
 */

#ifndef _ASM_BITOPS_H
#define _ASM_BITOPS_H

#ifndef _LINUX_BITOPS_H
#error only <linux/bitops.h> can be included directly
#endif

#if defined(__KERNEL__) && !defined(__ASSEMBLY__)

#include <linux/types.h>
#include <linux/compiler.h>

#if defined(CONFIG_ARC_HAS_LLSC)

static inline void set_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned int temp;

	m += nr >> 5;

	__asm__ __volatile__(
	"1:	llock   %0, [%1]	\n"
	"	bset    %0, %0, %2	\n"
	"	scond   %0, [%1]	\n"
	"	bnz     1b	\n"
	: "=&r"(temp)
	: "r"(m), "ir"(nr)
	: "cc");
}

static inline void clear_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned int temp;

	m += nr >> 5;

	__asm__ __volatile__(
	"1:	llock   %0, [%1]	\n"
	"	bclr    %0, %0, %2	\n"
	"	scond   %0, [%1]	\n"
	"	bnz     1b	\n"
	: "=&r"(temp)
	: "r"(m), "ir"(nr)
	: "cc");
}

static inline void change_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned int temp;

	m += nr >> 5;

	__asm__ __volatile__(
	"1:	llock   %0, [%1]	\n"
	"	bxor    %0, %0, %2	\n"
	"	scond   %0, [%1]	\n"
	"	bnz     1b		\n"
	: "=&r"(temp)
	: "r"(m), "ir"(nr)
	: "cc");
}

/*
 * Semantically:
 *    Test the bit
 *    if clear
 *        set it and return 0 (old value)
 *    else
 *        return 1 (old value).
 *
 * Since ARC lacks a equivalent h/w primitive, the bit is set unconditionally
 * and the old value of bit is returned
 */
static inline int test_and_set_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long old, temp;

	m += nr >> 5;

	__asm__ __volatile__(
	"1:	llock   %0, [%2]	\n"
	"	bset    %1, %0, %3	\n"
	"	scond   %1, [%2]	\n"
	"	bnz     1b		\n"
	: "=&r"(old), "=&r"(temp)
	: "r"(m), "ir"(nr)
	: "cc");

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	return (old & (1 << nr)) != 0;
}

static inline int
test_and_clear_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned int old, temp;

	m += nr >> 5;

	__asm__ __volatile__(
	"1:	llock   %0, [%2]	\n"
	"	bclr    %1, %0, %3	\n"
	"	scond   %1, [%2]	\n"
	"	bnz     1b		\n"
	: "=&r"(old), "=&r"(temp)
	: "r"(m), "ir"(nr)
	: "cc");

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	return (old & (1 << nr)) != 0;
}

static inline int
test_and_change_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned int old, temp;

	m += nr >> 5;

	__asm__ __volatile__(
	"1:	llock   %0, [%2]	\n"
	"	bxor    %1, %0, %3	\n"
	"	scond   %1, [%2]	\n"
	"	bnz     1b		\n"
	: "=&r"(old), "=&r"(temp)
	: "r"(m), "ir"(nr)
	: "cc");

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	return (old & (1 << nr)) != 0;
}

#else

#include <asm/smp.h>

static inline void set_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, flags;
	m += nr >> 5;

	bitops_lock(flags);

	__asm__ __volatile__(
	"	ld%U3 %0,%3	\n"
	"	bset %0,%0,%2	\n"
	"	st%U1 %0,%1	\n"
	: "=&r"(temp), "=o"(*m)
	: "ir"(nr), "m"(*m));

	bitops_unlock(flags);
}

static inline void clear_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, flags;
	m += nr >> 5;

	bitops_lock(flags);

	__asm__ __volatile__(
	"	ld%U3 %0,%3	\n"
	"	bclr %0,%0,%2	\n"
	"	st%U1 %0,%1	\n"
	: "=&r"(temp), "=o"(*m)
	: "ir"(nr), "m"(*m));

	bitops_unlock(flags);
}

static inline void change_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, flags;
	m += nr >> 5;

	bitops_lock(flags);

	__asm__ __volatile__(
	"	ld%U3 %0,%3	\n"
	"	bxor %0,%0,%2	\n"
	"	st%U1 %0,%1	\n"
	: "=&r"(temp), "=o"(*m)
	: "ir"(nr), "m"(*m));

	bitops_unlock(flags);
}

static inline int test_and_set_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long old, temp, flags;
	m += nr >> 5;

	bitops_lock(flags);

	old = *m;

	__asm__ __volatile__(
	"	bset  %0,%3,%2	\n"
	"	st%U1 %0,%1	\n"
	: "=&r"(temp), "=o"(*m)
	: "ir"(nr), "r"(old));

	bitops_unlock(flags);

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	return (old & (1 << nr)) != 0;
}

static inline int
test_and_clear_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, old, flags;
	m += nr >> 5;

	bitops_lock(flags);

	old = *m;

	__asm__ __volatile__(
	"	bclr  %0,%3,%2	\n"
	"	st%U1 %0,%1	\n"
	: "=&r"(temp), "=o"(*m)
	: "ir"(nr), "r"(old));

	bitops_unlock(flags);

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	return (old & (1 << nr)) != 0;
}

static inline int
test_and_change_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, old, flags;
	m += nr >> 5;

	bitops_lock(flags);

	old = *m;

	__asm__ __volatile__(
	"	bxor %0,%3,%2	\n"
	"	st%U1 %0,%1	\n"
	: "=&r"(temp), "=o"(*m)
	: "ir"(nr), "r"(old));

	bitops_unlock(flags);

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	return (old & (1 << nr)) != 0;
}

#endif /* CONFIG_ARC_HAS_LLSC */

/***************************************
 * Non atomic variants
 **************************************/

static inline void __set_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	temp = *m;
	*m = temp | (1UL << nr);
}

static inline void __clear_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	temp = *m;
	*m = temp & ~(1UL << (nr & 0x1f));
}

static inline void __change_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	temp = *m;
	*m = temp ^ (1UL << (nr & 0x1f));
}

static inline int
__test_and_set_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long old;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	old = *m;
	*m = old | (1 << nr);

	return (old & (1 << nr)) != 0;
}

static inline int
__test_and_clear_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long old;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	old = *m;
	*m = old & ~(1UL << (nr & 0x1f));

	return (old & (1 << nr)) != 0;
}

static inline int
__test_and_change_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long old;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	old = *m;
	*m = old ^ (1 << nr);

	return (old & (1 << nr)) != 0;
}

/*
 * This routine doesn't need to be atomic.
 */
static inline int
__constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr & 31)) &
		(((const volatile unsigned int *)addr)[nr >> 5])) != 0;
}

static inline int
__test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	unsigned long mask;

	addr += nr >> 5;

	/* ARC700 only considers 5 bits in bit-fiddling insn */
	mask = 1 << nr;

	return ((mask & *addr) != 0);
}

#define test_bit(nr, addr)	(__builtin_constant_p(nr) ? \
					__constant_test_bit((nr), (addr)) : \
					__test_bit((nr), (addr)))

/*
 * ffz = Find First Zero in word.
 * @return:[0-31], 32 if all 1's
 */
static __attribute__ ((const))
inline unsigned long ffz(unsigned long word)
{
	int result = 0;

	/* Given the way inline asm is written, it would infinite loop for
	@word = 0xFFFF_FFFF. So we need to test it anyways.
	The question is what to return ?
	I wanted to return 0, but old while loop used to return 32, so keeping
	that way. Also since this routine is Zero based, it makes sense to
	return 32 as indicative error value.
	*/

	if ((int)word == -1)
		return 32;

	__asm__ __volatile__(
	"1:	bbit1.d  %1, %0, 1b	\n"
	"	add      %0, %0, 1	\n"
	: "+r"(result)
	: "r"(word));

	/* On ARC700, delay slot insn is always executed for XX.d branch.
	 *Thus need to account for 1 extra add insn being executed
	 */
	result -= 1;

	return result;
}

/*
 * ffs = Find First Set in word (LSB to MSB)
 * @result: [1-32], 0 if all 0's
 */
static __attribute__ ((const))
inline unsigned long ffs(unsigned long word)
{
	int result = 0;

	if ((int)word == 0)
		return 0;

	__asm__ __volatile__(
	"1:	bbit0.d  %1, %0, 1b	\n"
	"	add      %0, %0, 1	\n"
	: "+r"(result)
	: "r"(word));

	/* Since it is 1 based, we need not worry abt result -= 1 as in ffz */

	return result;
}

/*
 * __ffs: Similar to ffs, but zero based (0-31)
 */
static __attribute__ ((const))
inline unsigned long __ffs(unsigned long word)
{
	if (!word)
		return word;
	return ffs(word) - 1;
}

/*
 * Count the number of zeros, starting from MSB
 * Helper for fls( ) routines
 * This is a pure count, so (1-32) or (0-31) doesn't apply
 * It could be 0 to 32, based on num of 0's in there
 * clz(0x8000_0000) = 0, clz(0xFFFF_FFFF)=0, clz(0) = 32, clz(1) = 31
 */
static inline int __attribute__ ((const))clz(unsigned int x)
{
	unsigned int res;

	__asm__ __volatile__(
	"	norm.f  %0, %1		\n"
	"	mov.n   %0, 0		\n"
	"	add.p   %0, %0, 1	\n"
	: "=r"(res)
	: "r"(x)
	: "cc");

	return res;
}

/*
 * fls = Find Last Set in word
 * @result: [1-32]
 * fls(1) = 1, fls(0x80000000) = 32, fls(0) = 0
 * Loopless: based on ARC norm insn
 */
static inline int __attribute__ ((const))fls(unsigned long x)
{
	return 32 - clz(x);
}

/*
 * __fls: Similar to fls, but zero based (0-31)
 */
static inline int __attribute__ ((const))__fls(unsigned long x)
{
	if (!x)
		return 0;
	else
		return fls(x) - 1;
}

/* TODO does this affect uni-processor code */
#define smp_mb__before_clear_bit()  barrier()
#define smp_mb__after_clear_bit()   barrier()

#include <asm-generic/bitops/hweight.h>
#include <asm-generic/bitops/fls64.h>
#include <asm-generic/bitops/sched.h>
#include <asm-generic/bitops/lock.h>

#include <asm-generic/bitops/find.h>
#include <asm-generic/bitops/le.h>
#include <asm-generic/bitops/ext2-atomic-setbit.h>

#endif /* __KERNEL__ && !__ASSEMBLY__ */

#endif
