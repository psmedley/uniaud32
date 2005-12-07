/*
 *  Do sleep inside a spin-lock
 *  Copyright (c) 1999 by Takashi Iwai <tiwai@suse.de>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <sound/driver.h>
#include "seq_lock.h"


void snd_seq_sleep_in_lock(wait_queue_head_t *p, spinlock_t *lock)
{
	wait_queue_t wait;

	set_current_state(TASK_INTERRUPTIBLE);
	init_waitqueue_entry(&wait, current);

	add_wait_queue(p, &wait);

	spin_unlock(lock);
	schedule();
	spin_lock_irq(lock);

	remove_wait_queue(p, &wait);
}

long snd_seq_sleep_timeout_in_lock(wait_queue_head_t *p, spinlock_t *lock, long timeout)
{
	wait_queue_t wait;

	set_current_state(TASK_INTERRUPTIBLE);
	init_waitqueue_entry(&wait, current);

	add_wait_queue(p, &wait);

	spin_unlock(lock);
	timeout = schedule_timeout(timeout);
	spin_lock_irq(lock);

	remove_wait_queue(p, &wait);

	return timeout;
}

