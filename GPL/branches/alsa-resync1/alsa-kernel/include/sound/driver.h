#ifndef __SOUND_DRIVER_H
#define __SOUND_DRIVER_H

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

#ifdef ALSA_BUILD
#include "config.h"
#endif

#include <linux/config.h>
#include <linux/version.h>

#define SNDRV_CARDS		8	/* number of supported soundcards - don't change - minor numbers */

#ifndef CONFIG_SND_MAJOR	/* standard configuration */
#define CONFIG_SND_MAJOR	116
#endif

#ifndef CONFIG_SND_DEBUG
#undef CONFIG_SND_DEBUG_MEMORY
#endif

#ifdef ALSA_BUILD
#include "adriver.h"
#endif

#include <linux/module.h>

#ifndef __iomem
#define __iomem
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

#define IRQ_NONE      (0)  /*void*/
#define IRQ_HANDLED   (1)  /*void*/
#define IRQ_RETVAL(x) ((x) != 0)  /*void*/
typedef int irqreturn_t;
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

struct work_struct {
        unsigned long pending;
        struct list_head entry;
        void (*func)(void *);
        void *data;
        void *wq_data;
        struct timer_list timer;
};

#define INIT_WORK(_work, _func, _data)                  \
        do {                                            \
                (_work)->func = _func;                  \
                (_work)->data = _data;                  \
                init_timer(&(_work)->timer);            \
        } while (0)
#define __WORK_INITIALIZER(n, f, d) {                   \
        .func = (f),                                    \
        .data = (d),                                    \
        }
#define DECLARE_WORK(n, f, d)                           \
        struct work_struct n = __WORK_INITIALIZER(n, f, d)
int snd_compat_schedule_work(struct work_struct *work);
#define schedule_work(w) snd_compat_schedule_work(w)

/*
 *  ==========================================================================
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 0)
#if defined(__i386__) || defined(__ppc__)
/*
 * Here a dirty hack for 2.4 kernels.. See kernel/memory.c.
 */
#define HACK_PCI_ALLOC_CONSISTENT
#include <linux/pci.h>
void *snd_pci_hack_alloc_consistent(struct pci_dev *hwdev, size_t size,
				    dma_addr_t *dma_handle);
#undef pci_alloc_consistent
#define pci_alloc_consistent snd_pci_hack_alloc_consistent
#endif /* i386 or ppc */
#endif /* 2.4.0 */

#ifdef CONFIG_SND_DEBUG_MEMORY
#include <linux/slab.h>
#include <linux/vmalloc.h>
void *snd_wrapper_kmalloc(size_t, int);
#undef kmalloc
void snd_wrapper_kfree(const void *);
#undef kfree
void *snd_wrapper_vmalloc(size_t);
#undef vmalloc
void snd_wrapper_vfree(void *);
#undef vfree
#endif
#ifdef TARGET_OS2
typedef struct snd_pcm_volume snd_pcm_volume_t;
#endif

#include "sndmagic.h"

#ifdef TARGET_OS2
#define snd_enter_user() 0
#define snd_leave_user(a)
#define dec_mod_count(a)	(*(unsigned long *)a)--
#endif

/*
 * Temporary hack, until linux/init.h is fixed.
 */

#include <linux/init.h>
#ifndef __devexit_p
#define __devexit_p(x) x
#endif

#endif /* __SOUND_DRIVER_H */
