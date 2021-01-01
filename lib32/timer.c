/* $Id: timer.c,v 1.1.1.1 2003/07/02 13:57:04 eleph Exp $ */
/*
 * OS/2 implementation of Linux timer kernel functions
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 * (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
 */

#include "linux.h"
#include <linux/init.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>
#include <asm/io.h>
#include <linux/time.h>
#include <linux/math64.h>
#include <linux/clocksource.h>

#define LINUX
#include <ossidc.h>
#include <irqos2.h>
#include <dbgos2.h>

static   long          jiffiems    = 1000/HZ;
static   long          lasttime    = 0;
unsigned long volatile jiffies     = 0;

//******************************************************************************
//Timer handler that is called on each timer tick (32 times/second)
//******************************************************************************
void ALSA_TIMER()
{ 
    long delta, newtime, remainder;

    newtime    = os2gettimemsec();
    delta      = newtime - lasttime;

    jiffies   += delta/jiffiems;
    remainder  = delta%jiffiems;

    lasttime   = newtime - remainder;
}
//******************************************************************************
//timeout is in 'jiffies', linux talk for units of 1000/HZ ms 
//******************************************************************************
signed long schedule_timeout(signed long timeout)
{
    dprintf2(("schedule_timeout %d jiffies %x", timeout, jiffies));
    mdelay(timeout*jiffiems);
    return 0;
}
//******************************************************************************
//iodelay is in 500ns units
void iodelay32(unsigned long);
#pragma aux iodelay32 parm nomemory [ecx] modify nomemory exact [eax ecx];
//******************************************************************************
//microsecond delay
//******************************************************************************
void __udelay(unsigned long usecs)
{
    if(usecs == 0) {
        DebugInt3();
        usecs = 1;
    }
    iodelay32(usecs*2);
}
//******************************************************************************
//millisecond delay
//******************************************************************************
void mdelay(unsigned long msecs)
{
    if(msecs == 0) {
        DebugInt3();
        msecs = 1;
    }
    iodelay32(msecs*2*1000);
}
//******************************************************************************
//******************************************************************************
void do_gettimeofday(struct timeval *tv)
{
#if 0
    tv->tv_sec  = 0; //os2gettimesec();
    tv->tv_usec = os2gettimemsec() * 1000;
#else /* r.ihle patch */
    unsigned u = os2gettimemsec();
    tv->tv_sec  = u / 1000;
    tv->tv_usec = (u % 1000) * 1000;
#endif
}
//******************************************************************************
//******************************************************************************
void add_timer(struct timer_list * timer)
{

}
//******************************************************************************
//******************************************************************************
int  del_timer(struct timer_list * timer)
{
  return 0;
}
//******************************************************************************
/*
 * mod_timer is a more efficient way to update the expire field of an
 * active timer (if the timer is inactive it will be activated)
 * mod_timer(a,b) is equivalent to del_timer(a); a->expires = b; add_timer(a)
 */
void mod_timer(struct timer_list *timer, unsigned long expires)
{

}
//******************************************************************************
//******************************************************************************
#include <linux/delay.h>
void msleep(unsigned int msecs)
{
	unsigned long timeout = ((msecs) * HZ + 999) / 1000;

	while (timeout) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		timeout = schedule_timeout(timeout);
	}
}

//******************************************************************************
//******************************************************************************

/**
 * ns_to_timespec - Convert nanoseconds to timespec
 * @nsec:       the nanoseconds value to be converted
 *
 * Returns the timespec representation of the nsec parameter.
 */
struct timespec ns_to_timespec(const s64 nsec)
{
	struct timespec ts;
	s32 rem;

	if (!nsec) {
		ts.tv_sec = 0;
		ts.tv_nsec = 0;
		return ts;
	}

	ts.tv_sec = div_s64_rem(nsec, NSEC_PER_SEC, &rem);
	if (unlikely(rem < 0)) {
		ts.tv_sec--;
		rem += NSEC_PER_SEC;
	}
	ts.tv_nsec = rem;

	return ts;
}


//******************************************************************************
//******************************************************************************

void timecounter_init(struct timecounter *tc,
		      const struct cyclecounter *cc,
		      u64 start_tstamp)
{
	tc->cc = cc;
	tc->cycle_last = cc->read(cc);
	tc->nsec = start_tstamp;
}

/**
 * timecounter_read_delta - get nanoseconds since last call of this function
 * @tc:         Pointer to time counter
 *
 * When the underlying cycle counter runs over, this will be handled
 * correctly as long as it does not run over more than once between
 * calls.
 *
 * The first call to this function for a new time counter initializes
 * the time tracking and returns an undefined result.
 */
static u64 timecounter_read_delta(struct timecounter *tc)
{
	cycle_t cycle_now, cycle_delta;
	u64 ns_offset;

	/* read cycle counter: */
	cycle_now = tc->cc->read(tc->cc);

	/* calculate the delta since the last timecounter_read_delta(): */
	cycle_delta = (cycle_now - tc->cycle_last) & tc->cc->mask;

	/* convert to nanoseconds: */
	ns_offset = cyclecounter_cyc2ns(tc->cc, cycle_delta);

	/* update time stamp of timecounter_read_delta() call: */
	tc->cycle_last = cycle_now;

	return ns_offset;
}

u64 timecounter_read(struct timecounter *tc)
{
	u64 nsec;

	/* increment time by nanoseconds since last call */
	nsec = timecounter_read_delta(tc);
	nsec += tc->nsec;
	tc->nsec = nsec;

	return nsec;
}

/**
 * set_normalized_timespec - set timespec sec and nsec parts and normalize
 *
 * @ts:		pointer to timespec variable to be set
 * @sec:	seconds to set
 * @nsec:	nanoseconds to set
 *
 * Set seconds and nanoseconds field of a timespec variable and
 * normalize to the timespec storage format
 *
 * Note: The tv_nsec part is always in the range of
 *	0 <= tv_nsec < NSEC_PER_SEC
 * For negative values only the tv_sec field is negative !
 */
void set_normalized_timespec64(struct timespec64 *ts, time64_t sec, s64 nsec)
{
	while (nsec >= NSEC_PER_SEC) {
		/*
		 * The following asm() prevents the compiler from
		 * optimising this loop into a modulo operation. See
		 * also __iter_div_u64_rem() in include/linux/time.h
		 */
//		asm("" : "+rm"(nsec));
		nsec -= NSEC_PER_SEC;
		++sec;
	}
	while (nsec < 0) {
//		asm("" : "+rm"(nsec));
		nsec += NSEC_PER_SEC;
		--sec;
	}
	ts->tv_sec = sec;
	ts->tv_nsec = nsec;
}
