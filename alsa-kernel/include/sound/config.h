#ifndef __ALSA_CONFIG_H__
#define __ALSA_CONFIG_H__

/*
 *  Configuration header file for compilation of the ALSA driver
 */


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
#include <linux/pid.h>
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
#include <linux/config.h>
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
#include <linux/lockdep.h>
#include <linux/smp_lock.h>
#include "compat_22.h"

#include <sound/asound.h>
#include <sound/asoundef.h>

/*
 *  ==========================================================================
 */

#define ATTRIBUTE_UNUSED
#define CONFIG_HAS_DMA
#define CONFIG_HAVE_KZALLOC
#define CONFIG_HAVE_KCALLOC
#define CONFIG_HAVE_KSTRDUP
#define CONFIG_HAVE_MSLEEP
#define CONFIG_HAVE_PCI_DEV_PRESENT
#define CONFIG_PCI
#define CONFIG_PM
#define CONFIG_PM_SLEEP
#ifdef DEBUG
#define CONFIG_SND_DEBUG_DETECT
#endif
#define CONFIG_SND_DEBUG_VERBOSE
#define CONFIG_SND_DMA_SGBUF
#define CONFIG_SND_HDA_CODEC_ANALOG
#define CONFIG_SND_HDA_CODEC_CONEXANT
#define CONFIG_SND_HDA_CODEC_CMEDIA
#define CONFIG_SND_HDA_CODEC_REALTEK
#define CONFIG_SND_HDA_CODEC_SIGMATEL
#define CONFIG_SND_HDA_CODEC_SI3054
#define CONFIG_SND_HDA_CODEC_VIA
#define CONFIG_SND_HDA_GENERIC
#define CONFIG_SND_HDA_HWDEP
#define CONFIG_SND_HDA_PREALLOC_SIZE  64
#define CONFIG_SND_OSSEMUL
#define CONFIG_SND_SEQUENCER
#define CONFIG_SOUND
#define CONFIG_SYSFS_DEPRECATED
#define CONFIG_SND_PROC_FS
#define CONFIG_SND_JACK
//#define CONFIG_SND_DYNAMIC_MINORS //new 2020-12-26
//#define CONFIG_SND_MAX_CARDS 8 //new 2020-12-26
#define PCI_NEW_SUSPEND
#define SNDRV_LITTLE_ENDIAN
#define CONFIG_SND_HDA_POWER_SAVE_DEFAULT 1
#define CONFIG_SND_PCM_TIMER 1
#define CONFIG_PROC_FS 1 /* new oct 2020 */
#define CONFIG_SND_PCM_OSS 1

/* Still need to work out where the following really belong */
#undef interrupt
#define __builtin_return_address(a)	0
#define __builtin_expect(x, expected_value) (x)
#define assert(a)
#define no_llseek	NULL
#define noop_llseek	NULL

struct class_device;
struct class_device_attribute {int x; }; /* dummy */
#define __ATTR(cls,perm,read,write) {0 } /* dummy */

#define min_t(type, x, y) ({                    \
        type __min1 = (x);                      \
        type __min2 = (y);                      \
        __min1 < __min2 ? __min1: __min2; })

#define true	1
#define false	0

#define BUILD_BUG_ON(condition)

#endif //__ALSA_CONFIG_H__
