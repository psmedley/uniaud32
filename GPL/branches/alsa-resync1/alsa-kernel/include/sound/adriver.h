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
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 3)
#define pci_set_dma_mask(pci, mask) pci->dma_mask = mask
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
#ifndef mk_kdev
#define mk_kdev(maj, min) MKDEV(maj, min)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 3)
#define need_resched() (current->need_resched)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 5) && !defined TARGET_OS2
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

#if defined(CONFIG_ISAPNP) || (defined(CONFIG_ISAPNP_MODULE) && defined(MODULE))
#if (defined(CONFIG_ISAPNP_KERNEL) && defined(ALSA_BUILD)) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 30) && !defined(ALSA_BUILD))
#include <linux/isapnp.h>
#define isapnp_dev pci_dev
#define isapnp_card pci_bus
#else
#include "isapnp.h"
#endif
#undef __ISAPNP__
#define __ISAPNP__
#endif

#if !defined(CONFIG_ISA) && defined(CONFIG_SND_ISA)
#define CONFIG_ISA
#endif

#ifndef MODULE_LICENSE
#define MODULE_LICENSE(license)
#endif

#if 0
static inline mm_segment_t snd_enter_user(void)
{
	mm_segment_t fs = get_fs();
	set_fs(get_ds());
	return fs;
}
static inline void snd_leave_user(mm_segment_t fs)
{
	set_fs(fs);
}

static inline void dec_mod_count(struct module *module)
{
	if (module)
		__MOD_DEC_USE_COUNT(module);
}
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
