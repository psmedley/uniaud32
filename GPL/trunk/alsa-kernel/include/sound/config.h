/*
 *  Configuration header file for compilation of the ALSA driver
 */

#ifndef __ALSA_CONFIG_H__
#define __ALSA_CONFIG_H__

#define inline __inline
#define INLINE inline
#define __attribute__

#include <linux/version.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/signal.h>
#include <linux/kdev_t.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/dcache.h>
#include <linux/vmalloc.h>
#include <linux/tqueue.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/stat.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/gfp.h>
#include <linux/workqueue.h>
#include <linux/byteorder/little_endian.h>
#include <linux/lockdep.h>
#include <linux/string.h>
#include <asm/ioctl.h>
#include <asm/hardirq.h>
#include <asm/processor.h>
#include <asm/siginfo.h>
#include <dbgos2.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <stdio.h>

typedef unsigned int fmode_t;

/*
 *  ==========================================================================
 */

#ifdef ALSA_BUILD
#undef MODULE
#define MODULE
#endif

#include <linux/config.h>
#include <linux/version.h>

#define LINUX_2_2

#ifdef ALSA_BUILD
#if defined(CONFIG_MODVERSIONS) && !defined(__GENKSYMS__) && !defined(__DEPEND__)
#define MODVERSIONS
#include <linux/modversions.h>
#include "sndversions.h"
#endif
#endif /* ALSA_BUILD */

#ifndef SNDRV_MAIN_OBJECT_FILE
#define __NO_VERSION__
#endif
#include <linux/module.h>

#include <linux/utsname.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/malloc.h>
#include <linux/delay.h>

#include <linux/ioport.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/string.h>

#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/pagemap.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/vmalloc.h>
#include <linux/poll.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include "compat_22.h"

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

#include <sound/asound.h>
#include <sound/asoundef.h>


int snd_compat_schedule_work(struct work_struct *work);
#define schedule_work(w) snd_compat_schedule_work(w)
void snd_compat_flush_workqueue(struct workqueue_struct *wq);
#define flush_workqueue(wq) snd_compat_flush_workqueue((wq));

/* Name change */
typedef struct timeval snd_timestamp_t;
typedef union sndrv_pcm_sync_id snd_pcm_sync_id_t;

#ifdef CONFIG_SND_DEBUG_MEMORY
void *snd_wrapper_kmalloc(size_t, int);
#undef kmalloc
void snd_wrapper_kfree(const void *);
#undef kfree
void *snd_wrapper_vmalloc(size_t);
#undef vmalloc
void snd_wrapper_vfree(void *);
#undef vfree
#endif

/*
 *  ==========================================================================
 */

extern int this_module[64];
#define THIS_MODULE (void *)&this_module[0]
#define MODULE_GENERIC_TABLE(gtype,name)
#define MODULE_DEVICE_TABLE(type,name)
#define CONFIG_PROC_FS
#define CONFIG_PM
#define PCI_NEW_SUSPEND
#define CONFIG_PCI
#define CONFIG_HAS_DMA
#define CONFIG_SND_SEQUENCER
#define CONFIG_SND_OSSEMUL
#define CONFIG_SND_PCM_OSS
#define SNDRV_LITTLE_ENDIAN
#define EXPORT_SYMBOL(a)
#define CONFIG_SOUND
#define ATTRIBUTE_UNUSED
#define CONFIG_SND_HDA_CODEC_REALTEK
#define CONFIG_SND_HDA_CODEC_CMEDIA
#define CONFIG_SND_HDA_CODEC_ANALOG
#define CONFIG_SND_HDA_CODEC_SIGMATEL
#define CONFIG_SND_HDA_CODEC_SI3054
#define CONFIG_SND_HDA_CODEC_ATIHDMI
#define CONFIG_SND_HDA_CODEC_CONEXANT
#define CONFIG_SND_HDA_CODEC_VIA
#define CONFIG_SND_HDA_GENERIC
#define CONFIG_SND_HDA_HWDEP
#define CONFIG_PM
#define CONFIG_HAVE_PCI_DEV_PRESENT
#define CONFIG_SND_DEBUG_DETECT
#define CONFIG_SND_DEBUG_VERBOSE
#define CONFIG_SYSFS_DEPRECATED
#undef interrupt

#define __builtin_return_address(a)	0
#define __builtin_expect(x, expected_value) (x)


#ifndef CONFIG_HAVE_KZALLOC
void *snd_compat_kzalloc(size_t n, gfp_t gfp_flags);
#define kzalloc(s,f) snd_compat_kzalloc(s,f)
#endif

#ifndef CONFIG_HAVE_KSTRDUP
char *snd_compat_kstrdup(const char *s, unsigned int __nocast gfp_flags);
#define kstrdup(s,f) snd_compat_kstrdup(s,f)
#endif

#ifndef CONFIG_HAVE_KCALLOC
void *snd_compat_kcalloc(size_t n, size_t size, unsigned int __nocast gfp_flags);
#define kcalloc(n,s,f) snd_compat_kcalloc(n,s,f)
#endif

struct workqueue_struct *snd_compat_create_workqueue(const char *name);
#define create_workqueue(name) snd_compat_create_workqueue((name))
void snd_compat_destroy_workqueue(struct workqueue_struct *wq);
#define destroy_workqueue(wq) snd_compat_destroy_workqueue((wq))
int snd_compat_queue_work(struct workqueue_struct *wq, struct work_struct *work);
#define queue_work(wq, work) snd_compat_queue_work((wq), (work))

#ifndef BUG_ON
#define BUG_ON(x) /* nothing */
#endif

int snd_compat_devfs_mk_dir(const char *dir, ...);
#define devfs_mk_dir snd_compat_devfs_mk_dir
int snd_compat_devfs_mk_cdev(dev_t dev, umode_t mode, const char *fmt, ...);
#define devfs_mk_cdev snd_compat_devfs_mk_cdev
void snd_compat_devfs_remove(const char *fmt, ...);
#define devfs_remove snd_compat_devfs_remove
#define SINGLE_DEPTH_NESTING                    1

#ifndef roundup
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#endif


/* msleep */
#ifndef CONFIG_HAVE_MSLEEP
void snd_compat_msleep(unsigned int msecs);
#define msleep snd_compat_msleep
#endif

#define assert(a)

/* memdup_user() wrapper */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30)
#include <linux/err.h>
#include <asm/uaccess.h>
static inline void *memdup_user(void __user *src, size_t len)
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
#endif

#endif //__ALSA_CONFIG_H__

