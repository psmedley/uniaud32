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

