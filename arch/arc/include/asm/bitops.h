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

/*
 * There's "significant" micro-optimization in writing our own variants of
 * bitops.
 * (1) The generic APIs have "signed" @nr while we have it "unsigned"
 *     This avoids extra code to be generated for pointer arithmatic, since
 *     is "not sure" that index is NOT -ve
 * (2) Utilize the fact that ARCompact bit fidding insn (BSET/BCLR/ASL) etc
 *     only consider bottom 5 bits of @nr, so NO need to mask them off.
 *     (GCC Quirk: however for constant @nr we still need to do the masking
 *             at compile time)
 */

static inline void set_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, flags;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	bitops_lock(flags);

	temp = *m;
	*m = temp | (1UL << nr);

	bitops_unlock(flags);
}

static inline void clear_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, flags;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	bitops_lock(flags);

	temp = *m;
	*m = temp & ~(1UL << nr);

	bitops_unlock(flags);
}

static inline void change_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, flags;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	bitops_lock(flags);

	temp = *m;
	*m = temp ^ (1UL << nr);

	bitops_unlock(flags);
}

static inline int test_and_set_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long old, temp, flags;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	bitops_lock(flags);

	old = *m;
	*m = old | (1 << nr);

	bitops_unlock(flags);

	return (old & (1 << nr)) != 0;
}

static inline int
test_and_clear_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, old, flags;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	bitops_lock(flags);

	old = *m;
	*m = old | (1 << nr);

	bitops_unlock(flags);

	return (old & (1 << nr)) != 0;
}

static inline int
test_and_change_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp, old, flags;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	bitops_lock(flags);

	old = *m;
	*m = old ^ (1 << nr);

	bitops_unlock(flags);

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
	*m = temp & ~(1UL << nr);
}

static inline void __change_bit(unsigned long nr, volatile unsigned long *m)
{
	unsigned long temp;
	m += nr >> 5;

	if (__builtin_constant_p(nr))
		nr &= 0x1f;

	temp = *m;
	*m = temp ^ (1UL << nr);
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
	*m = old & ~(1 << nr);

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
 * Count the number of zeros, starting from MSB
 * Helper for fls( ) friends
 * This is a pure count, so (1-32) or (0-31) doesn't apply
 * It could be 0 to 32, based on num of 0's in there
 * clz(0x8000_0000) = 0, clz(0xFFFF_FFFF)=0, clz(0) = 32, clz(1) = 31
 */
static inline __attribute__ ((const)) int clz(unsigned int x)
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
 */
static inline __attribute__ ((const)) int fls(unsigned long x)
{
	return 32 - clz(x);
}

/*
 * __fls: Similar to fls, but zero based (0-31)
 */
static inline __attribute__ ((const)) int __fls(unsigned long x)
{
	if (!x)
		return 0;
	else
		return fls(x) - 1;
}

/*
 * ffs = Find First Set in word (LSB to MSB)
 * @result: [1-32], 0 if all 0's
 */
#define ffs(x)	({ unsigned long __t = (x); fls(__t & -__t); })

/*
 * __ffs: Similar to ffs, but zero based (0-31)
 */
static inline __attribute__ ((const)) int __ffs(unsigned long word)
{
	if (!word)
		return word;

	return ffs(word) - 1;
}

/*
 * ffz = Find First Zero in word.
 * @return:[0-31], 32 if all 1's
 */
#define ffz(x)	__ffs(~(x))

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
