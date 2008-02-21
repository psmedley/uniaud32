#ifndef __SOUND_LOCAL_DRIVER_H
#define __SOUND_LOCAL_DRIVER_H

/*
 *  Main header file for the ALSA driver
 *  Copyright (c) 1994-2000 by Jaroslav Kysela <perex@suse.cz>
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

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 2, 3)
#error "This driver requires Linux 2.2.3 or higher."
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 1)
#  if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 0)
#  error "This code requires Linux 2.4.0-test1 and higher."
#  endif
#define LINUX_2_4__donotuse
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 2, 0)
#define LINUX_2_2
#endif

#ifdef ALSA_BUILD
#if defined(CONFIG_MODVERSIONS) && !defined(__GENKSYMS__) && !defined(__DEPEND__)
#define MODVERSIONS
#include <linux/modversions.h>
#include "sndversions.h"
#endif
#ifdef SNDRV_NO_MODVERS
#undef MODVERSIONS
#undef _set_ver
#endif
#endif /* ALSA_BUILD */

#include <linux/module.h>

#ifdef CONFIG_HAVE_OLD_REQUEST_MODULE
#include <linux/kmod.h>
#undef request_module
void snd_compat_request_module(const char *name, ...);
#define request_module(name, args...) snd_compat_request_module(name, ##args)
#endif

#ifndef TARGET_OS2
#include <linux/compiler.h>
#endif /* !TARGET_OS2 */
#ifndef __user
#define __user
#endif

#ifdef CONFIG_PCI
#include <linux/pci.h>
#endif

#ifndef __iomem
#define __iomem
#endif

#ifdef LINUX_2_2
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 2, 18)
#include <linux/init.h>
#endif
#ifndef LINUX_2_4__donotuse
#include "compat_22.h"
#endif
#endif /* LINUX_2_2 */

#ifdef LINUX_2_4__donotuse
#include <linux/init.h>
#include <linux/pm.h>
#include <asm/page.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 3)
#define pci_set_dma_mask(pci, mask) (pci->dma_mask = mask, 0)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 7)
#define PCI_OLD_SUSPEND
#endif
#ifndef virt_to_page
#define virt_to_page(x) (&mem_map[MAP_NR(x)])
#endif
#define snd_request_region request_region
#ifndef rwlock_init
#define rwlock_init(x) do { *(x) = RW_LOCK_UNLOCKED; } while(0)
#endif
#ifndef list_for_each_safe
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)
#endif
#endif /* LINUX_2_4__donotuse */

#ifndef __devexit_p
#define __devexit_p(x) x
#endif

#include <linux/kdev_t.h>
#ifndef major
#define major(x) MAJOR(x)
#endif
#ifndef minor
#define minor(x) MINOR(x)
#endif
#ifndef imajor
#define imajor(x) major((x)->i_rdev)
#endif
#ifndef iminor
#define iminor(x) minor((x)->i_rdev)
#endif
#ifndef mk_kdev
#define mk_kdev(maj, min) MKDEV(maj, min)
#endif
#ifndef DECLARE_BITMAP
#define DECLARE_BITMAP(name,bits) \
	unsigned long name[((bits)+BITS_PER_LONG-1)/BITS_PER_LONG]
#endif

#include <linux/sched.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 3) && !defined(need_resched)
#define need_resched() (current->need_resched)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 4) && !defined(WE_ARE_WOLK) && !defined TARGET_OS2
#include <linux/fs.h>
static inline struct proc_dir_entry *PDE(const struct inode *inode)
{
	return (struct proc_dir_entry *) inode->u.generic_ip;
}
#endif
#ifndef cond_resched
#define cond_resched() \
	do { \
		if (1) { \
                        set_current_state(TASK_RUNNING); \
                        schedule(); \
                } \
        } while (0)
#endif
#include <asm/io.h>
#if !defined(isa_virt_to_bus)
#if defined(virt_to_bus) || defined(__alpha__)
#define isa_virt_to_bus virt_to_bus
#endif
#endif

/* isapnp support for 2.2 kernels */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 0)
#undef CONFIG_ISAPNP
#ifdef CONFIG_SND_ISAPNP
#define CONFIG_ISAPNP
#endif
#endif

/* support of pnp compatible layer for 2.2/2.4 kernels */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)
#undef CONFIG_PNP
#ifdef CONFIG_SND_PNP
#define CONFIG_PNP
#endif
#endif

#if !defined(CONFIG_ISA) && defined(CONFIG_SND_ISA)
#define CONFIG_ISA
#endif

#ifndef MODULE_LICENSE
#define MODULE_LICENSE(license)
#endif

#if !defined CONFIG_HAVE_STRLCPY && !defined TARGET_OS2
size_t snd_compat_strlcpy(char *dest, const char *src, size_t size);
#define strlcpy(dest, src, size) snd_compat_strlcpy(dest, src, size)
size_t snd_compat_strlcat(char *dest, const char *src, size_t size);
#define strlcat(dest, src, size) snd_compat_strlcat(dest, src, size)
#endif

#if !defined CONFIG_HAVE_SNPRINTF && !defined TARGET_OS2
#include <stdarg.h>
int snd_compat_snprintf(char * buf, size_t size, const char * fmt, ...);
int snd_compat_vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
#define snprintf(buf,size,fmt,args...) snd_compat_snprintf(buf,size,fmt,##args)
#define vsnprintf(buf,size,fmt,args) snd_compat_vsnprintf(buf,size,fmt,args)
#endif

#if defined(__alpha__) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 14)
#include <asm/io.h>
#undef writeb
#define writeb(v, a) do { __writeb((v),(a)); mb(); } while(0)
#undef writew
#define writew(v, a) do { __writew((v),(a)); mb(); } while(0)
#undef writel
#define writel(v, a) do { __writel((v),(a)); mb(); } while(0)
#undef writeq
#define writeq(v, a) do { __writeq((v),(a)); mb(); } while(0)
#endif

#define IRQ_NONE      (0)  /*void*/
#define IRQ_HANDLED   (1)  /*void*/
#define IRQ_RETVAL(x) ((x) != 0)  /*void*/
typedef int irqreturn_t;


/* workqueue-alike; 2.5.45 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
#include <linux/workqueue.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 45) && !defined(__WORK_INITIALIZER)
#define SND_WORKQUEUE_COMPAT
struct workqueue_struct;
struct work_struct {
	unsigned long pending;
	struct list_head entry;
	void (*func)(void *);
	void *data;
	void *wq_data;
	struct timer_list timer;
};
#define INIT_WORK(_work, _func, _data)			\
	do {						\
		(_work)->func = _func;			\
		(_work)->data = _data;			\
		init_timer(&(_work)->timer);		\
        } while (0)
#define __WORK_INITIALIZER(n, f, d) {			\
		.func = (f),				\
		.data = (d),				\
	}
#define DECLARE_WORK(n, f, d)				\
        struct work_struct n = __WORK_INITIALIZER(n, f, d)
int snd_compat_schedule_work(struct work_struct *work);
#define schedule_work(w) snd_compat_schedule_work(w)
struct workqueue_struct *snd_compat_create_workqueue(const char *name);
#define create_workqueue(name) snd_compat_create_workqueue((name))
void snd_compat_flush_workqueue(struct workqueue_struct *wq);
#define flush_workqueue(wq) snd_compat_flush_workqueue((wq));
void snd_compat_destroy_workqueue(struct workqueue_struct *wq);
#define destroy_workqueue(wq) snd_compat_destroy_workqueue((wq));
int snd_compat_queue_work(struct workqueue_struct *wq, struct work_struct *work);
#define queue_work(wq, work) snd_compat_queue_work((wq), (work))
int snd_compat_queue_delayed_work(struct workqueue_struct *wq, struct work_struct *work, unsigned long delay);
#define queue_delayed_work(wq, work, delay) snd_compat_queue_delayed_work((wq), (work), (delay))
#endif /* < 2.5.45 */
#endif /* < 2.6.0 */

/* 2.5 new modules */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)
#define try_module_get(x) try_inc_mod_count(x)
static inline void module_put(struct module *module)
{
	if (module)
		__MOD_DEC_USE_COUNT(module);
}
#endif /* 2.5.0 */

/* gameport - 2.4 has different defines */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)
#ifdef CONFIG_INPUT_GAMEPORT
#define CONFIG_GAMEPORT
#endif
#ifdef CONFIG_INPUT_GAMEPORT_MODULE
#define CONFIG_GAMEPORT_MODULE
#endif
#endif /* 2.5.0 */

/* vmalloc_to_page wrapper */
#ifndef CONFIG_HAVE_VMALLOC_TO_PAGE
struct page *snd_compat_vmalloc_to_page(void *addr);
#define vmalloc_to_page(addr) snd_compat_vmalloc_to_page(addr)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 69)
#include <linux/vmalloc.h>
static inline void *snd_compat_vmap(struct page **pages, unsigned int count, unsigned long flags, pgprot_t prot)
{
	return vmap(pages, count);
}
#undef vmap
#define vmap snd_compat_vmap
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0) /* correct version? */
#define EXPORT_NO_SYMBOLS
#endif

/* MODULE_ALIAS & co. */
#ifndef MODULE_ALIAS
#define MODULE_ALIAS(x)
#define MODULE_ALIAS_CHARDEV_MAJOR(x)
#endif

#ifndef CONFIG_HAVE_PCI_CONSISTENT_DMA_MASK
#define pci_set_consistent_dma_mask(p,x) 0 /* success */
#endif

/* sysfs */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 2)
struct class_simple;
static inline void class_simple_device_add(struct class_simple *class, int devnum, ...) { return; }
static inline void class_simple_device_remove(int devnum) { return; }
#endif

#ifndef CONFIG_HAVE_MSLEEP_INTERRUPTIBLE
#include <linux/delay.h>
unsigned long snd_compat_msleep_interruptible(unsigned int msecs);
#define msleep_interruptible snd_compat_msleep_interruptible
#ifndef ssleep
#define ssleep(x) msleep((unsigned int)(x) * 1000)
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
#define snd_card_set_dev(card,dev) /* no struct device */
#endif

/* pm_message_t type */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
#include <linux/pm.h>
#ifndef PMSG_FREEZE
typedef u32 /* __bitwise */ pm_message_t;
#define PMSG_FREEZE	3
#define PMSG_SUSPEND	3
#define PMSG_ON		0
#endif
#endif
#ifndef PCI_D0
#define PCI_D0     0
#define PCI_D1     1
#define PCI_D2     2
#define PCI_D3hot  3
#define PCI_D3cold 4
#define pci_choose_state(pci,state)     ((state) ? PCI_D3hot : PCI_D0)
#endif

/* wrapper for getnstimeofday()
 * it's needed for recent 2.6 kernels, too, due to lack of EXPORT_SYMBOL
 */
#define getnstimeofday(x) do { \
	struct timeval __x; \
	do_gettimeofday(&__x); \
	(x)->tv_sec = __x.tv_sec;	\
	(x)->tv_nsec = __x.tv_usec * 1000; \
} while (0)

#endif /* __SOUND_LOCAL_DRIVER_H */
