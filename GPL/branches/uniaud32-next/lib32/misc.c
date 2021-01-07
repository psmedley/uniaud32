/* $Id: misc.c,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
/*
 * OS/2 implementation of misc. Linux kernel services
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 * (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
 *
 * hweight32 based on Linux code (bitops.h)
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
#include <limits.h>
#include "linux.h"
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/poll.h>
#define CONFIG_PROC_FS 1
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>
#include <linux\ioport.h>
#include <linux\utsname.h>
#include <linux\module.h>
#include <linux/workqueue.h>
#include <linux/firmware.h>
#include <linux/ctype.h>
#include <dbgos2.h>

void StringOut(char *DbgStr);

struct new_utsname system_utsname = {0};
struct resource ioport_resource = {NULL, 0, 0, IORESOURCE_IO, NULL, NULL, NULL};
struct resource iomem_resource  = {NULL, 0, 0, IORESOURCE_MEM, NULL, NULL, NULL};
mem_map_t *mem_map = 0;
int this_module[64] = {0};

#include <stdarg.h>

char szLastALSAError1[128] = {0};
char szOverrunTest1        = 0xCC;
char szLastALSAError2[128] = {0};
char szOverrunTest2        = 0xCC;
int  iLastError            = 0;

//******************************************************************************
//Save error message in szLastALSAError; if card init failed, then we will
//print it in drv32\init.cpp
//******************************************************************************
int printk(const char * fmt, ...)
{
    va_list argptr;                /* -> variable argument list */

    char *pszLastALSAError;

    pszLastALSAError= iLastError ? szLastALSAError2 : szLastALSAError1;

    va_start(argptr, fmt);           /* get pointer to argument list */
    vsprintf(pszLastALSAError, fmt, argptr);
//    strcat(pszLastALSAError, "\r");
    va_end(argptr);                /* done with variable arguments */

    if(szOverrunTest1 != 0xCC || szOverrunTest2 != 0xCC) {
        DebugInt3();
    }

	StringOut(pszLastALSAError);
//    rprintf( (pszLastALSAError) );
    if(++iLastError > 1) {
        iLastError = 0;
    }
    return 0;
}

//******************************************************************************
//Save error message in szLastALSAError; if card init failed, then we will
//print it in drv32\init.cpp
//******************************************************************************
int dev_dbg(const struct device *dev, const char * fmt, ...)
{
    va_list argptr;                /* -> variable argument list */

    char *pszLastALSAError;

    pszLastALSAError= iLastError ? szLastALSAError2 : szLastALSAError1;

    va_start(argptr, fmt);           /* get pointer to argument list */
    vsprintf(pszLastALSAError, fmt, argptr);
//    strcat(pszLastALSAError, "\r");
    va_end(argptr);                /* done with variable arguments */

    if(szOverrunTest1 != 0xCC || szOverrunTest2 != 0xCC) {
        DebugInt3();
    }

	StringOut(pszLastALSAError);
//    rprintf( (pszLastALSAError) );
    if(++iLastError > 1) {
        iLastError = 0;
    }
    return 0;
}
//******************************************************************************
int pcm_dbg(struct snd_pcm *dev, const char * fmt, ...)
{
    va_list argptr;                /* -> variable argument list */

    char *pszLastALSAError;

    pszLastALSAError= iLastError ? szLastALSAError2 : szLastALSAError1;

    va_start(argptr, fmt);           /* get pointer to argument list */
    vsprintf(pszLastALSAError, fmt, argptr);
//    strcat(pszLastALSAError, "\r");
    va_end(argptr);                /* done with variable arguments */

    if(szOverrunTest1 != 0xCC || szOverrunTest2 != 0xCC) {
        DebugInt3();
    }

	StringOut(pszLastALSAError);
//    rprintf( (pszLastALSAError) );
    if(++iLastError > 1) {
        iLastError = 0;
    }
    return 0;
}
//******************************************************************************
int codec_dbg(struct hda_codec *dev, const char * fmt, ...)
{
    va_list argptr;                /* -> variable argument list */

    char *pszLastALSAError;

    pszLastALSAError= iLastError ? szLastALSAError2 : szLastALSAError1;

    va_start(argptr, fmt);           /* get pointer to argument list */
    vsprintf(pszLastALSAError, fmt, argptr);
//    strcat(pszLastALSAError, "\r");
    va_end(argptr);                /* done with variable arguments */

    if(szOverrunTest1 != 0xCC || szOverrunTest2 != 0xCC) {
        DebugInt3();
    }

	StringOut(pszLastALSAError);
//    rprintf( (pszLastALSAError) );
    if(++iLastError > 1) {
        iLastError = 0;
    }
    return 0;
}
//******************************************************************************
//******************************************************************************
void schedule(void)
{

}
//******************************************************************************
//******************************************************************************
void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p)
{

}
//******************************************************************************
//******************************************************************************
int __check_region(struct resource *a, unsigned long b, unsigned long c)
{
    DebugInt3();
    return 0;
}

/* --------------------------------------------------------------------- */
/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#ifdef hweight32
#undef hweight32
#endif

unsigned int hweight32(unsigned int w)
{
	unsigned int res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
	res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
	res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
	return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
}
//******************************************************************************
//******************************************************************************
#if 0
mem_map_t *virt_to_page(int x)
{
    static mem_map_t map = {0};
    return &map;
}
#endif
//******************************************************************************
//******************************************************************************
struct proc_dir_entry proc_root = {0};
//******************************************************************************
//******************************************************************************
struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode,
				         struct proc_dir_entry *parent)
{
    struct proc_dir_entry *proc;
    proc = (struct proc_dir_entry *)kmalloc(sizeof(struct proc_dir_entry), 0);
    memset(proc, 0, sizeof(struct proc_dir_entry));

    proc->name   = name;
    proc->mode   = mode;
    proc->parent = parent;

    return proc;
}
//******************************************************************************
//******************************************************************************
void remove_proc_entry(const char *name, struct proc_dir_entry *parent)
{
    return; //memory leak
}
//******************************************************************************
//******************************************************************************
int proc_register(struct proc_dir_entry *parent, struct proc_dir_entry *proc)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
int proc_unregister(struct proc_dir_entry *proc, int bla)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
struct proc_dir_entry *proc_mkdir_mode(const char *name, umode_t mode,
	struct proc_dir_entry *parent){
    struct proc_dir_entry *proc;
    proc = (struct proc_dir_entry *)kmalloc(sizeof(struct proc_dir_entry), 0);
    memset(proc, 0, sizeof(struct proc_dir_entry));

    proc->name   = name;
    proc->parent = parent;

    return proc;
}
//******************************************************************************
//******************************************************************************
struct proc_dir_entry *proc_mkdir(const char *name, struct proc_dir_entry *parent)
{
    return proc_mkdir_mode(name, S_IRUGO | S_IXUGO, parent);
}
//******************************************************************************
//******************************************************************************
struct proc_dir_entry *proc_symlink(const char *name, struct proc_dir_entry *parent, const char *dest)
{
    struct proc_dir_entry *proc;
    proc = (struct proc_dir_entry *)kmalloc(sizeof(struct proc_dir_entry), 0);
    memset(proc, 0, sizeof(struct proc_dir_entry));

    proc->name   = name;
    proc->parent = parent;
    proc->data = (void*)dest;

    return proc;
}
//******************************************************************************
//******************************************************************************
int fasync_helper(int a, struct file *b, int c, struct fasync_struct **d)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
void kill_fasync(struct fasync_struct **a, int b, int c)
{
}
//******************************************************************************
//******************************************************************************
int request_dma(unsigned int dmanr, const char * device_id)	/* reserve a DMA channel */
{
    DebugInt3();
    return 0;
}
//******************************************************************************
//******************************************************************************
void free_dma(unsigned int dmanr)
{
    DebugInt3();
}
//******************************************************************************
/* enable/disable a specific DMA channel */
//******************************************************************************
void enable_dma(unsigned int dmanr)
{
    DebugInt3();
}
//******************************************************************************
//******************************************************************************
void disable_dma(unsigned int dmanr)
{
    DebugInt3();
}
//******************************************************************************
//static struct notifier_block *reboot_notify_list = NULL;
// No need to implement this right now. The ESS Maestro 3 driver uses it
// to call pci_unregister_driver, which is always called from the shutdown
// notification sent by OS2.
// Same goes for es1968 & Yamaha's DS1/DS1E.
//******************************************************************************
int register_reboot_notifier(struct notifier_block *pnblock)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
int unregister_reboot_notifier(struct notifier_block *pnblock)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
static void __x_queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	unsigned long flags;

	spin_lock_irqsave(&wq->lock, flags);
	work->wq_data = wq;
	list_add_tail(&work->entry, &wq->worklist);
	wake_up(&wq->more_work);
	spin_unlock_irqrestore(&wq->lock, flags);
}
//******************************************************************************
//******************************************************************************
int queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	if (!test_and_set_bit(0, &work->pending)) {
		__x_queue_work(wq, work);
		return 1;
	}
	return 0;
}
//******************************************************************************
//******************************************************************************
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
//******************************************************************************
//******************************************************************************
void flush_workqueue(struct workqueue_struct *wq)
{

	if (wq->task == current) {
		run_workqueue(wq);
	} else {
		struct wait_queue_entry wait;

		init_waitqueue_entry(&wait, current);
		set_current_state(TASK_UNINTERRUPTIBLE);
		spin_lock_irq(&wq->lock);
		add_wait_queue(&wq->work_done, &wait);
#ifndef TARGET_OS2
		while (!list_empty(&wq->worklist)) {
			spin_unlock_irq(&wq->lock);
			schedule();  // DAZ system hangs here because this function does nothing
			spin_lock_irq(&wq->lock);
		}
#endif
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&wq->work_done, &wait);
		spin_unlock_irq(&wq->lock);
	}
}
//******************************************************************************
//******************************************************************************
struct workqueue_struct *create_workqueue(const char *name)
{
	struct workqueue_struct *wq;
	
	BUG_ON(strlen(name) > 10);
	
	wq = kmalloc(sizeof(*wq), GFP_KERNEL);
	if (!wq)
		return NULL;
	memset(wq, 0, sizeof(*wq));
	
	spin_lock_init(&wq->lock);
	INIT_LIST_HEAD(&wq->worklist);
	init_waitqueue_head(&wq->more_work);
	init_waitqueue_head(&wq->work_done);
#ifndef TARGET_OS2
	init_completion(&wq->thread_exited);
	wq->name = name;
	wq->task_pid = kernel_thread(xworker_thread, wq, 0);
	if (wq->task_pid < 0) {
		printk(KERN_ERR "snd: failed to start thread %s\n", name);
		snd_compat_destroy_workqueue(wq);
		wq = NULL;
	}
	wq->task = find_task_by_pid(wq->task_pid);
#endif
	return wq;
}
//******************************************************************************
//******************************************************************************
void destroy_workqueue(struct workqueue_struct *wq)
{
#ifndef TARGET_OS2
	snd_compat_flush_workqueue(wq);
	kill_proc(wq->task_pid, SIGKILL, 1);
	if (wq->task_pid >= 0)
		wait_for_completion(&wq->thread_exited);
#endif
	kfree(wq);
}

//******************************************************************************
//******************************************************************************
char *kstrdup(const char *s, unsigned int __nocast gfp_flags)
{
	int len;
	char *buf;

	if (!s) return NULL;

	len = strlen(s) + 1;
	buf = kmalloc(len, gfp_flags);
	if (buf)
		memcpy(buf, s, len);
	return buf;
}
//******************************************************************************
//******************************************************************************
int mod_firmware_load(const char *fn, char **fp)
{
    return 0;
}
//******************************************************************************
//******************************************************************************
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
//******************************************************************************
//******************************************************************************
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
//******************************************************************************
//******************************************************************************
void release_firmware(const struct firmware *fw)
{
	if (fw) {
		vfree(fw->data);
		kfree(fw);
	}
}
//******************************************************************************
//******************************************************************************
void *memdup_user(const void __user *src, size_t len)
{
	void *p = kmalloc(len, GFP_KERNEL);
	if (!p)
		return ERR_PTR(-ENOMEM);
	if (copy_from_user(p, src, len)) {
		kfree(p);
		return ERR_PTR(-EFAULT);
	}
	return p;
}
//******************************************************************************
//******************************************************************************
#define del_timer_sync(t) del_timer(t) /* FIXME: not quite correct on SMP */
int cancel_delayed_work(struct delayed_work *dwork)
{
	struct work_struct *work = &dwork->work;
	int ret;

	ret = del_timer_sync(&work->timer);
	if (ret)
		clear_bit(0, &work->pending);
	return ret;
}
//******************************************************************************
//******************************************************************************
struct workqueue_struct *system_wq;

int schedule_work(struct work_struct *work)
{
#ifndef TARGET_OS2 // crashes
	return queue_work(system_wq, work);
#endif
	return 0;
}

//******************************************************************************
//******************************************************************************
bool flush_delayed_work_sync(struct delayed_work *dwork)
{
	bool ret;
	ret = cancel_delayed_work(dwork);
	if (ret) {
		schedule_delayed_work(dwork, 0);
		flush_scheduled_work();
	}
	return ret;
}
//******************************************************************************
//******************************************************************************
static void delayed_work_timer_fn(unsigned long __data)
{
	struct work_struct *work = (struct work_struct *)__data;
	struct workqueue_struct *wq = work->wq_data;
	
	if (wq)
		__x_queue_work(wq, work);
	else
		schedule_work(work);
}
//******************************************************************************
//******************************************************************************
int queue_delayed_work(struct workqueue_struct *wq, struct delayed_work *dwork, unsigned long delay)
{
	struct work_struct *work = &dwork->work;
	struct timer_list *timer = &work->timer;

	if (!test_and_set_bit(0, &work->pending)) {
		work->wq_data = wq;
		timer->expires = jiffies + delay;
		timer->data = (unsigned long)work;
		timer->function = delayed_work_timer_fn;
		add_timer(timer);
		return 1;
	}
	return 0;
}
//******************************************************************************
//******************************************************************************
/* Greatest common divisor */
unsigned long gcd(unsigned long a, unsigned long b)
{
	unsigned long r;
	if (a < b) {
		r = a;
		a = b;
		b = r;
	}
	while ((r = a % b) != 0) {
		a = b;
		b = r;
	}
	return b;
}

//******************************************************************************
//******************************************************************************
int strict_strtoul(const char *cp, unsigned int base, unsigned long *res)
{
        char *tail;
        unsigned long val;
        size_t len;

        *res = 0;
        len = strlen(cp);
        if (len == 0)
                return -EINVAL;

        val = simple_strtoul(cp, &tail, base);
        if (tail == cp)
                return -EINVAL;

        if ((*tail == '\0') ||
                ((len == (size_t)(tail - cp) + 1) && (*tail == '\n'))) {
                *res = val;
                return 0;
        }

        return -EINVAL;
}
//******************************************************************************
//******************************************************************************

static void u32_swap(void *a, void *b, int size)
{
	u32 t = *(u32 *)a;
	*(u32 *)a = *(u32 *)b;
	*(u32 *)b = t;
}

static void generic_swap(void *a, void *b, int size)
{
	char t;

	do {
		t = *(char *)a;
		*(char *)a++ = *(char *)b;
		*(char *)b++ = t;
	} while (--size > 0);
}

/**
 * sort - sort an array of elements
 * @base: pointer to data to sort
 * @num: number of elements
 * @size: size of each element
 * @cmp_func: pointer to comparison function
 * @swap_func: pointer to swap function or NULL
 *
 * This function does a heapsort on the given array. You may provide a
 * swap_func function optimized to your element type.
 *
 * Sorting time is O(n log n) both on average and worst-case. While
 * qsort is about 20% faster on average, it suffers from exploitable
 * O(n*n) worst-case behavior and extra memory requirements that make
 * it less suitable for kernel use.
 */

void sort(void *base, size_t num, size_t size,
	  int (*cmp_func)(const void *, const void *),
	  void (*swap_func)(void *, void *, int size))
{
	/* pre-scale counters for performance */
	int i = (num/2 - 1) * size, n = num * size, c, r;

	if (!swap_func)
		swap_func = (size == 4 ? u32_swap : generic_swap);

	/* heapify */
	for ( ; i >= 0; i -= size) {
		for (r = i; r * 2 + size < n; r  = c) {
			c = r * 2 + size;
			if (c < n - size &&
					cmp_func(base + c, base + c + size) < 0)
				c += size;
			if (cmp_func(base + r, base + c) >= 0)
				break;
			swap_func(base + r, base + c, size);
		}
	}

	/* sort */
	for (i = n - size; i > 0; i -= size) {
		swap_func(base, base + i, size);
		for (r = 0; r * 2 + size < i; r = c) {
			c = r * 2 + size;
			if (c < i - size &&
					cmp_func(base + c, base + c + size) < 0)
				c += size;
			if (cmp_func(base + r, base + c) >= 0)
				break;
			swap_func(base + r, base + c, size);
		}
	}
}
//******************************************************************************
//******************************************************************************

/**
 * hex_to_bin - convert a hex digit to its real value
 * @ch: ascii character represents hex digit
 *
 * hex_to_bin() converts one hex digit to its actual value or -1 in case of bad
 * input.
 */
int hex_to_bin(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	ch = tolower(ch);
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	return -1;
}
//******************************************************************************


/*
 * stream_open is used by subsystems that want stream-like file descriptors.
 * Such file descriptors are not seekable and don't have notion of position
 * (file.f_pos is always 0). Contrary to file descriptors of other regular
 * files, .read() and .write() can run simultaneously.
 *
 * stream_open never fails and is marked to return int so that it could be
 * directly used as file_operations.open .
 */
int stream_open(struct inode *inode, struct file *filp)
{
	filp->f_mode &= ~(FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE | FMODE_ATOMIC_POS);
	//DAZ f_mode is 16 bit so FMODE_STREAM doesn't fit, but we don't use this on OS/2.
	//filp->f_mode |= FMODE_STREAM;
	return 0;
}

/**
 * alloc_pages_exact - allocate pages with the given size
 * @size: the size to allocate in bytes
 * @gfp_flags: the allocation conditions, GFP_XXX
 *
 * Allocates the physically contiguous pages with the given size.
 *
 * Return: The pointer of the buffer, or %NULL if no enough memory.
 */
void *alloc_pages_exact(size_t size, gfp_t gfp_flags)
{
	int pg;

	if (WARN_ON(!size))
		return NULL;
	if (WARN_ON(!gfp_flags))
		return NULL;
	gfp_flags |= __GFP_COMP;	/* compound page lets parts be mapped */
	pg = get_order(size);
	return (void *) __get_free_pages(gfp_flags, pg);
}

/**
 * free_pages_exact - release the pages
 * @ptr: the buffer pointer to release
 * @size: the allocated buffer size
 *
 * Releases the buffer allocated via snd_malloc_pages().
 */
void free_pages_exact(void *ptr, size_t size)
{
	int pg;

	if (ptr == NULL)
		return;
	pg = get_order(size);
	free_pages((unsigned long) ptr, pg);
}
