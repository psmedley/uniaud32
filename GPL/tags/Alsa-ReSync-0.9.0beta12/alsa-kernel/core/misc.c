/*
 *  Misc and compatibility things
 *  Copyright (c) by Jaroslav Kysela <perex@suse.cz>
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#define __NO_VERSION__
#include <sound/driver.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <sound/core.h>

#ifdef TARGET_OS2
#include <linux/firmware.h>
#endif /* TARGET_OS2 */

int snd_task_name(struct task_struct *task, char *name, size_t size)
{
	unsigned int idx;

	snd_assert(task != NULL && name != NULL && size >= 2, return -EINVAL);
	for (idx = 0; idx < sizeof(task->comm) && idx + 1 < size; idx++)
		name[idx] = task->comm[idx];
	name[idx] = '\0';
	return 0;
}

static void run_workqueue(struct workqueue_struct *wq)
{
	unsigned long flags;

	spin_lock_irqsave(&wq->lock, flags);
	while (!list_empty(&wq->worklist)) {
		struct work_struct *work = list_entry(wq->worklist.next,
						      struct work_struct, entry);
		void (*f) (void *) = work->func;
		void *data = work->data;

		list_del_init(wq->worklist.next);
		spin_unlock_irqrestore(&wq->lock, flags);
		clear_bit(0, &work->pending);
		f(data);
		spin_lock_irqsave(&wq->lock, flags);
		wake_up(&wq->work_done);
	}
	spin_unlock_irqrestore(&wq->lock, flags);
}

#if 0
void flush_workqueue(struct workqueue_struct *wq)
{
	if (0 /* wq->task == current */) {
		run_workqueue(wq);
	} else {
		wait_queue_t wait;

		init_waitqueue_entry(&wait, current);
		set_current_state(TASK_UNINTERRUPTIBLE);
		spin_lock_irq(&wq->lock);
		add_wait_queue(&wq->work_done, &wait);
		while (!list_empty(&wq->worklist)) {
			spin_unlock_irq(&wq->lock);
			schedule();
			spin_lock_irq(&wq->lock);
		}
		remove_wait_queue(&wq->work_done, &wait);
		spin_unlock_irq(&wq->lock);
	}
}
#endif 

void destroy_workqueue(struct workqueue_struct *wq)
{
#if 0
    flush_workqueue(wq);
	kill_proc(wq->task_pid, SIGKILL, 1);
	if (wq->task_pid >= 0)
            wait_for_completion(&wq->thread_exited);
#endif
	kfree(wq);
}

static int xworker_thread(void *data)
{
	struct workqueue_struct *wq = data;

	strcpy(current->comm, wq->name); /* FIXME: different names? */

	do {
            run_workqueue(wq);
#if 0
            wait_event_interruptible(wq->more_work, !list_empty(&wq->worklist));
#endif
	} while (!signal_pending(current));
#if 0
        complete_and_exit(&wq->thread_exited, 0);
#endif
}

struct workqueue_struct *create_workqueue(const char *name)
{
	struct workqueue_struct *wq;
	
	//BUG_ON(strlen(name) > 10);
	
	wq = kmalloc(sizeof(*wq), GFP_KERNEL);
	if (!wq)
		return NULL;
	memset(wq, 0, sizeof(*wq));
	
	spin_lock_init(&wq->lock);
	INIT_LIST_HEAD(&wq->worklist);
	init_waitqueue_head(&wq->more_work);
        init_waitqueue_head(&wq->work_done);
        wq->name = name;
#if 0
	init_completion(&wq->thread_exited);
	wq->task_pid = kernel_thread(xworker_thread, wq, 0);
	if (wq->task_pid < 0) {
		printk(KERN_ERR "snd: failed to start thread %s\n", name);
		destroy_workqueue(wq);
		wq = NULL;
        }
#endif
	return wq;
}

static void __x_queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	unsigned long flags;

	spin_lock_irqsave(&wq->lock, flags);
	work->wq_data = wq;
	list_add_tail(&work->entry, &wq->worklist);
	wake_up(&wq->more_work);
	spin_unlock_irqrestore(&wq->lock, flags);
}

int queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	if (!test_and_set_bit(0, &work->pending)) {
		__x_queue_work(wq, work);
		return 1;
	}
	return 0;
}

static void delayed_work_timer_fn(unsigned long __data)
{
	struct work_struct *work = (struct work_struct *)__data;
	struct workqueue_struct *wq = work->wq_data;
	
	__x_queue_work(wq, work);
}

int queue_delayed_work(struct workqueue_struct *wq, struct work_struct *work, unsigned long delay)
{
	struct timer_list *timer = &work->timer;

	if (!test_and_set_bit(0, &work->pending)) {
		work->wq_data = work;
		timer->expires = jiffies + delay;
		timer->data = (unsigned long)work;
		timer->function = delayed_work_timer_fn;
		add_timer(timer);
		return 1;
	}
	return 0;
}

void release_and_free_resource(struct resource *res)
{
        if (res) {
                release_resource(res);
                kfree(res);
        }
}

int mod_firmware_load(const char *fn, char **fp)
{
    return 0;
}

static int snd_try_load_firmware(const char *path, const char *name,
				 struct firmware *firmware)
{
	char filename[30 + FIRMWARE_NAME_MAX];

	sprintf(filename, "%s/%s", path, name);
	firmware->size = mod_firmware_load(filename, (char **)&firmware->data);
	if (firmware->size)
		printk(KERN_INFO "Loaded '%s'.", filename);
	return firmware->size;
}

int request_firmware(const struct firmware **fw, const char *name,
		     struct device *device)
{
	struct firmware *firmware;

	*fw = NULL;
	firmware = kmalloc(sizeof *firmware, GFP_KERNEL);
	if (!firmware)
		return -ENOMEM;
	if (!snd_try_load_firmware("/lib/firmware", name, firmware) &&
	    !snd_try_load_firmware("/lib/hotplug/firmware", name, firmware) &&
	    !snd_try_load_firmware("/usr/lib/hotplug/firmware", name, firmware)) {
		kfree(firmware);
		return -EIO;
	}
	*fw = firmware;
	return 0;
}

void release_firmware(const struct firmware *fw)
{
	if (fw) {
		vfree(fw->data);
		kfree(fw);
	}
}

/**
 * snd_pci_quirk_lookup - look up a PCI SSID quirk list
 * @pci: pci_dev handle
 * @list: quirk list, terminated by a null entry
 *
 * Look through the given quirk list and finds a matching entry
 * with the same PCI SSID.  When subdevice is 0, all subdevice
 * values may match.
 *
 * Returns the matched entry pointer, or NULL if nothing matched.
 */
const struct snd_pci_quirk *
snd_pci_quirk_lookup(struct pci_dev *pci, const struct snd_pci_quirk *list)
{
	const struct snd_pci_quirk *q;

	for (q = list; q->subvendor; q++)
		if (q->subvendor == pci->subsystem_vendor &&
		    (!q->subdevice || q->subdevice == pci->subsystem_device))
			return q;
	return NULL;
}

