#ifndef __ALSA_CONFIG_H__
#define __ALSA_CONFIG_H__

/*
 *  Configuration header file for compilation of the ALSA driver
 */


#include <linux/version.h>

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
#define assert(a)
#define no_llseek	NULL
#define noop_llseek	NULL

#define min_t(type, x, y) ({                    \
        type __min1 = (x);                      \
        type __min2 = (y);                      \
        __min1 < __min2 ? __min1: __min2; })

#define true	1
#define false	0

#define BUILD_BUG_ON(condition)

#endif //__ALSA_CONFIG_H__
