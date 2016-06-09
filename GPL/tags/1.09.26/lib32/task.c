/* $Id: task.c,v 1.1.1.1 2003/07/02 13:57:04 eleph Exp $ */
/*
 * OS/2 implementation of Linux task kernel services
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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/tqueue.h>
#include <linux/interrupt.h>
#define LINUX
#include <ossdefos2.h>

struct task_struct current_task = {0};
struct task_struct *current = &current_task;

void tasklet_hi_schedule(struct tasklet_struct *t)
{
  if(t && t->func) {
	t->func(t->data);
  }
}

void tasklet_init(struct tasklet_struct *t,
		  void (*func)(unsigned long), unsigned long data)
{
	t->next = NULL;
	t->sync = 0;
	t->func = (void *)func; /* DAZ cast added to stop compiler warning */
	t->data = (void *)data; /* DAZ cast added to stop compiiler warning */
}

//Not pretty, but sblive driver compares pointers
ULONG OSS32_SetFileId(ULONG fileid)
{
 ULONG oldfileid = (ULONG)current_task.files;

  current_task.files = (struct files_struct *)fileid;
  return oldfileid;
}
