/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * vineetg: Jan 1011
 *  -sched_clock( ) no longer jiffies based. Uses the same clocksource
 *   as gtod
 *
 * Rajeshwarr/Vineetg: Mar 2008
 *  -Implemented CONFIG_GENERIC_TIME (rather deleted arch specific code)
 *   for arch independent gettimeofday()
 *  -Implemented CONFIG_GENERIC_CLOCKEVENTS as base for hrtimers
 *
 * Vineetg: Mar 2008: Forked off from time.c which now is time-jiff.c
 */

#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/timex.h>
#include <linux/profile.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <asm/irq.h>
#include <asm/arcregs.h>

/* ARC700 has two 32bit independent prog Timers: TIMER0 and TIMER1
 * Each can programmed to go from @count to @limit and optionally
 * interrupt when that happens
 *
 * We've designated TIMER0 for events (clockevents)
 * while TIMER1 for free running (clocksource)
 */

/******************************************************************
 * Hardware Interface routines to program the ARC TIMERs
 ******************************************************************/

/*
 * Arm the timer to interrupt after @limit cycles
 */
static void arc_timer0_setup_event(unsigned int limit)
{
	/* setup start and end markers */
	write_aux_reg(ARC_REG_TIMER0_LIMIT, limit);
	write_aux_reg(ARC_REG_TIMER0_CNT, 0);	/* start from 0 */

	/* IE: Interrupt on count = limit,
	 * NH: Count cycles only when CPU running (NOT Halted)
	 */
	write_aux_reg(ARC_REG_TIMER0_CTRL, TIMER_CTRL_IE | TIMER_CTRL_NH);
}

/*
 * Acknowledge the interrupt & enable/disable the interrupt
 */
static void arc_timer0_ack_event(unsigned int irq_reenable)
{
	/* 1. Ack the interrupt by writing to CTRL reg.
	 *    Any write will cause intr to be ack, however it has to be one of
	 *    writable bits (NH: Count when not halted)
	 * 2. If required by caller, re-arm timer to Interrupt at the end of
	 *    next cycle.
	 *
	 * Small optimisation:
	 * Normal code would have been
	 *  if (irq_reenable) CTRL_REG = (IE | NH); else CTRL_REG = NH;
	 * However since IE is BIT0 we can fold the branch
	 */
	write_aux_reg(ARC_REG_TIMER0_CTRL, irq_reenable | TIMER_CTRL_NH);
}

/*
 * Arm the timer to keep counting monotonically
 */
static void arc_timer1_setup_free_flow(unsigned int limit)
{
	/* although for free flowing case, limit would alway be max 32 bits
	 * still we've kept the interface open, just in case ...
	 */
	write_aux_reg(ARC_REG_TIMER1_LIMIT, limit);
	write_aux_reg(ARC_REG_TIMER1_CNT, 0);
	write_aux_reg(ARC_REG_TIMER1_CTRL, TIMER_CTRL_NH);
}

/********** Clock Source Device *********/

static cycle_t cycle_read_t1(struct clocksource *cs)
{
	return (cycle_t) read_aux_reg(ARC_REG_TIMER1_CNT);
}

static struct clocksource clocksource_t1 = {
	.name = "ARC Timer1",
	.rating = 300,
	.read = cycle_read_t1,
	.mask = CLOCKSOURCE_MASK(32),
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __init arc_clocksource_init(void)
{
	u64 temp;
	u32 shift;
	struct clocksource *cs = &clocksource_t1;

	/* Find a shift value */
	for (shift = 32; shift > 0; shift--) {
		temp = (u64) NSEC_PER_SEC << shift;
		do_div(temp, CONFIG_ARC_PLAT_CLK);
		if ((temp >> 32) == 0)
			break;
	}
	cs->shift = shift;
	cs->mult = (u32) temp;

	arc_timer1_setup_free_flow(0xFFFFFFFF);

	clocksource_register(&clocksource_t1);
}

/********** Clock Event Device *********/

struct clock_event_device arc_clockevent_device;

static irqreturn_t timer_irq_handler(int irq, void *dev_id);

static struct irqaction arc_timer_irq = {
	.name = "ARC Timer0",
	.flags = IRQF_TIMER | IRQF_DISABLED,
	.handler = timer_irq_handler,
	.dev_id = &arc_clockevent_device,
};

irqreturn_t timer_irq_handler(int irq, void *dev_id)
{
	struct clock_event_device *evt = dev_id;

	arc_timer0_ack_event(evt->mode == CLOCK_EVT_MODE_PERIODIC);
	evt->event_handler(evt);
	return IRQ_HANDLED;
}

static int arc_next_event(unsigned long delta, struct clock_event_device *dev)
{
	arc_timer0_setup_event(delta);
	return 0;
}

static void arc_set_mode(enum clock_event_mode mode,
			 struct clock_event_device *dev)
{
	pr_info("clockevent mode switch to [%d]\n", mode);
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		arc_timer0_setup_event(CONFIG_ARC_PLAT_CLK / HZ);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		break;
	default:
		break;
	}

	return;
}

static void __cpuinit arc_clockevent_init(void)
{
	u64 temp;
	u32 shift;
	struct clock_event_device *cd = &arc_clockevent_device;

	cd->name = "ARC Clock";
	cd->features = CLOCK_EVT_FEAT_ONESHOT;
	cd->features |= CLOCK_EVT_FEAT_PERIODIC;
	cd->mode = CLOCK_EVT_MODE_UNUSED;

	/* Find a shift value */
	for (shift = 32; shift > 0; shift--) {
		temp = (u64) CONFIG_ARC_PLAT_CLK << shift;
		do_div(temp, NSEC_PER_SEC);
		if ((temp >> 32) == 0)
			break;
	}
	cd->shift = shift;
	cd->mult = (u32) temp;

	/* configuring min_delta_ns as nanoseconds for 1 tick
	 * and max_delta_ns as 0xffffffff, max limit value accepted by hardware
	 * for any event at less than the min_delta_ns is rounded to this time
	 * and for any event at more than the max_delta_ns we configure the max
	 * and end up having intermediate ticks before the event
	 * This is taken care in clockevent_program_event
	 */

	cd->max_delta_ns = clockevent_delta2ns(0xffffffff, cd);
	cd->min_delta_ns = clockevent_delta2ns(0x1, cd);

	cd->rating = 300;
	cd->irq = TIMER0_INT;
	cd->cpumask = cpumask_of(0);
	cd->set_next_event = arc_next_event;
	cd->set_mode = arc_set_mode;

	clockevents_register_device(cd);

	setup_irq(TIMER0_INT, &arc_timer_irq);
}

void __init time_init(void)
{
	arc_clocksource_init();
	arc_clockevent_init();
}

static int arc_finished_booting;

/*
 * Scheduler clock - returns current time in nanosec units.
 * It's return value must NOT wrap around.
 *
 * Although the return value is nanosec units based, what's more important
 * is whats the "source" of this value. The orig jiffies based computation
 * was only as granular as jiffies itself (10ms on ARC).
 * We need something that is more granular, so use the same mechanism as
 * gettimeofday(), which uses ARC Timer T1 wrapped as a clocksource.
 * Unfortunately the first call to sched_clock( ) is way before that subsys
 * is initialiased, thus use the jiffies based value in the interim.
 */
unsigned long long sched_clock(void)
{
	if (!arc_finished_booting) {
		return (unsigned long long)(jiffies - INITIAL_JIFFIES)
		    * (NSEC_PER_SEC / HZ);
	} else {
		struct timespec ts;
		getrawmonotonic(&ts);
		return (unsigned long long)timespec_to_ns(&ts);
	}
}

static int __init arc_clocksource_done_booting(void)
{
	arc_finished_booting = 1;
	return 0;
}

fs_initcall(arc_clocksource_done_booting);
