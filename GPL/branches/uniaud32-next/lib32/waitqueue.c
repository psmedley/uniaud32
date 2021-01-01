/* $Id: waitqueue.c,v 1.1.1.1 2003/07/02 13:57:04 eleph Exp $ */
/*
 * OS/2 implementation of Linux wait queue kernel services (stubs)
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
void init_waitqueue_head(wait_queue_head_t *q)
{
//    dprintf(("WARNING: init_waitqueue_head STUB"));
}
//******************************************************************************
//******************************************************************************
void add_wait_queue(struct wait_queue_head *q, struct wait_queue_entry * wait)
{
//    dprintf3(("WARNING: add_wait_queue STUB"));
}
//******************************************************************************
//******************************************************************************
void add_wait_queue_exclusive(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
{
//    dprintf3(("WARNING: add_wait_queue_exclusive STUB"));
}
//******************************************************************************
//******************************************************************************
void remove_wait_queue(struct wait_queue_head *q, struct wait_queue_entry * wait)
{
//    dprintf(("WARNING: remove_wait_queue STUB"));
}
//******************************************************************************
//******************************************************************************
long interruptible_sleep_on_timeout(wait_queue_head_t *q, signed long timeout)
{
    dprintf(("WARNING: interruptible_sleep_on_timeout STUB"));
    return 1;
}
//******************************************************************************
//******************************************************************************
void interruptible_sleep_on(wait_queue_head_t *q)
{
    dprintf(("WARNING: interruptible_sleep_on STUB"));
}
//******************************************************************************
//******************************************************************************
void __wake_up(wait_queue_head_t *q, unsigned int mode)
{
    dprintf3(("WARNING: __wake_up STUB"));
}
//******************************************************************************
//******************************************************************************
void init_waitqueue_entry(struct wait_queue_entry *wq_entry, struct task_struct *p)
{
//    dprintf(("WARNING: init_waitqueue_entry STUB"));
}
//******************************************************************************
//******************************************************************************
int waitqueue_active(wait_queue_head_t *q)
{
//    dprintf(("WARNING: waitqueue_active STUB"));
    return 0;
}
//******************************************************************************
//******************************************************************************
/**
 * complete: - signals a single thread waiting on this completion
 * @x:  holds the state of this particular completion
 *
 * This will wake up a single thread waiting on this completion. Threads will be
 * awakened in the same order in which they were queued.
 *
 * See also complete_all(), wait_for_completion() and related routines.
 *
 * It may be assumed that this function implies a write memory barrier before
 * changing the task state if and only if any tasks are woken up.
 */
void complete(struct completion *x)
{
	unsigned long flags;

	spin_lock_irqsave(&x->wait.lock, flags);
	x->done++;
	__wake_up_locked(&x->wait, TASK_NORMAL, 1);
	spin_unlock_irqrestore(&x->wait.lock, flags);
}
//******************************************************************************
//******************************************************************************
void __wake_up_locked(wait_queue_head_t *q, unsigned int mode, int nr)
{
    dprintf3(("WARNING: __wake_up_locked STUB"));
}
