/* $Id: spinlock.c,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
/*
 * OS/2 implementation of Linux spinlock kernel services
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


unsigned long __lock(void);
#pragma aux __lock =		\
	"pushfd"		\
	"cli"			\
	"pop eax"		\
	modify exact [eax]	\
	value [eax];


void __unlock(unsigned long cpuflags);
#pragma aux __unlock =		\
	"push eax"		\
	"popfd"			\
	modify exact []		\
	parm [eax];



void spin_lock_init(spinlock_t *lock)
{
  *lock = 0;
}

void spin_lock(spinlock_t *lock)
{
  *lock = __lock();
}

void spin_lock_flag(spinlock_t *lock, unsigned long *flag)
{
  *lock = __lock();
}

int spin_trylock(spinlock_t *lock)
{
  return 0;
}

#if 0
void spin_unlock_wait(spinlock_t *lock)
{

}
#endif

void spin_unlock(spinlock_t *lock)
{
  __unlock(*lock);
}
