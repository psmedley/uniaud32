/* $Id: semaphore.c,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
/*
 * OS/2 implementation of Linux semaphore kernel services (stubs)
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
#include <dbgos2.h>

//******************************************************************************
//******************************************************************************
void init_MUTEX (struct semaphore *sem)
{
    dprintf3(("Warning: init_MUTEX %x not implemented!!", sem));
}
//******************************************************************************
//******************************************************************************
void init_MUTEX_LOCKED (struct semaphore *sem)
{
    dprintf3(("Warning: init_MUTEX_LOCKED %x not implemented!!", sem));
}
//******************************************************************************
//******************************************************************************
void down(struct semaphore * sem)
{
    dprintf3(("Warning: semaphore down not implemented!!"));
}
//******************************************************************************
//******************************************************************************
void up(struct semaphore * sem)
{
    dprintf3(("Warning: semaphore up not implemented!!"));
}
//******************************************************************************
//******************************************************************************
int down_interruptible(struct semaphore * sem)
{
    dprintf3(("Warning: semaphore down_interruptible not implemented!!"));
    return 0;
}
//******************************************************************************
//******************************************************************************

