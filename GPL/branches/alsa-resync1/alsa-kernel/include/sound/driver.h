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
 *  ==========================================================================
 */

/* device allocation stuff */

#define SNDRV_DEV_TYPE_RANGE_SIZE		0x1000

typedef enum {
    SNDRV_DEV_TOPLEVEL =		(0*SNDRV_DEV_TYPE_RANGE_SIZE),
    SNDRV_DEV_CONTROL,
	SNDRV_DEV_LOWLEVEL_PRE,
	SNDRV_DEV_LOWLEVEL_NORMAL =	(1*SNDRV_DEV_TYPE_RANGE_SIZE),
	SNDRV_DEV_PCM,
	SNDRV_DEV_RAWMIDI,
	SNDRV_DEV_TIMER,
	SNDRV_DEV_SEQUENCER,
        SNDRV_DEV_HWDEP,
        SNDRV_DEV_INFO,
        SNDRV_DEV_BUS,
        SNDRV_DEV_CODEC,
	SNDRV_DEV_LOWLEVEL =		(2*SNDRV_DEV_TYPE_RANGE_SIZE)
} snd_device_type_t;

typedef enum {
	SNDRV_DEV_BUILD,
        SNDRV_DEV_REGISTERED,
        SNDRV_DEV_DISCONNECTED
} snd_device_state_t;

typedef enum {
	SNDRV_DEV_CMD_PRE = 0,
	SNDRV_DEV_CMD_NORMAL = 1,
	SNDRV_DEV_CMD_POST = 2
} snd_device_cmd_t;


struct snd_device;

struct snd_device_ops {
        int (*dev_free)(struct snd_device *dev);
        int (*dev_register)(struct snd_device *dev);
        int (*dev_disconnect)(struct snd_device *dev);
        int (*dev_unregister)(struct snd_device *dev);
};

struct snd_device {
	struct list_head list;		/* list of registered devices */
	struct snd_card *card;		/* card which holds this device */
	snd_device_state_t state;	/* state of the device */
	snd_device_type_t type;		/* device type */
	void *device_data;		/* device structure */
	struct snd_device_ops *ops;		/* operations */
};

#define snd_device(n) list_entry(n, struct snd_device, list)

/* various typedefs */

typedef struct snd_info_entry snd_info_entry_t;
#ifdef CONFIG_SND_OSSEMUL
typedef struct _snd_oss_mixer snd_mixer_oss_t;
#endif

/* main structure for soundcard */

/* monitor files for graceful shutdown (hotplug) */

struct snd_monitor_file {
        struct file *file;
        struct snd_monitor_file *next;
};

struct snd_card {
	int number;			/* number of soundcard (index to snd_cards) */

	char id[16];			/* id string of this card */
	char driver[16];		/* driver name */
	char shortname[32];		/* short name of this soundcard */
	char longname[80];		/* name of this soundcard */
	char mixername[80];		/* mixer name */
	char components[80];		/* card components delimited with space */

	struct module *module;		/* top-level module */

	void *private_data;		/* private data for soundcard */
	void (*private_free) (struct snd_card *card); /* callback for freeing of private data */

	struct list_head devices;	/* devices */

        unsigned int last_numid;	/* last used numeric ID */
        struct rw_semaphore controls_rwsem;     /* controls list lock */	rwlock_t control_rwlock;	/* control list lock */
	rwlock_t ctl_files_rwlock;	/* ctl_files list lock */
        int controls_count;		/* count of all controls */
        int user_ctl_count;             /* count of all user controls */
	struct list_head controls;	/* all controls for this card */
	struct list_head ctl_files;	/* active control files */

	snd_info_entry_t *proc_root;	/* root for soundcard specific files */
	snd_info_entry_t *proc_id;	/* the card id */
        struct proc_dir_entry *proc_root_link;	/* number link to real id */

        struct snd_monitor_file *files; /* all files associated to this card */
        struct snd_shutdown_f_ops *s_f_ops; /* file operations in the shutdown state */
        spinlock_t files_lock;          /* lock the files for this card */
        int shutdown;                   /* this card is going down */
        wait_queue_head_t shutdown_sleep;
        struct work_struct free_workq;	/* for free in workqueue */
        struct device *dev;
#ifdef CONFIG_PM
        int (*pm_suspend)(struct snd_card *card, unsigned int state);
        int (*pm_resume)(struct snd_card *card, unsigned int state);
        struct pm_dev *pm_dev;          /* for ISA */
        void *pm_private_data;
	unsigned int power_state;	/* power state */
	struct semaphore power_lock;	/* power lock */
	wait_queue_head_t power_sleep;
#endif

#if defined(CONFIG_SND_MIXER_OSS) || defined(CONFIG_SND_MIXER_OSS_MODULE)
	snd_mixer_oss_t *mixer_oss;
	int mixer_oss_change_count;
#endif
};

#ifdef CONFIG_PM
static inline void snd_power_lock(struct snd_card *card)
{
	down(&card->power_lock);
}

static inline void snd_power_unlock(struct snd_card *card)
{
	up(&card->power_lock);
}

int snd_power_wait(struct snd_card *card, unsigned int power_state, struct file *file);

static inline unsigned int snd_power_get_state(struct snd_card *card)
{
	return card->power_state;
}

static inline void snd_power_change_state(struct snd_card *card, unsigned int state)
{
	card->power_state = state;
	wake_up(&card->power_sleep);
}
int snd_card_set_pm_callback(struct snd_card *card,
                             int (*suspend)(struct snd_card *, unsigned int),
                             int (*resume)(struct snd_card *, unsigned int),
                             void *private_data);
int snd_card_set_isa_pm_callback(struct snd_card *card,
                                 int (*suspend)(struct snd_card *, unsigned int),
                                 int (*resume)(struct snd_card *, unsigned int),
                                 void *private_data);
#ifndef SND_PCI_PM_CALLBACKS
int snd_card_pci_suspend(struct pci_dev *dev, u32 state);
int snd_card_pci_resume(struct pci_dev *dev);
#define SND_PCI_PM_CALLBACKS \
    snd_card_pci_suspend, \
    snd_card_pci_resume
#endif
#else
#define snd_power_lock(card) do { ; } while (0)
#define snd_power_unlock(card) do { ; } while (0)
#define snd_power_wait(card) do { ; } while (0)
#define snd_power_get_state(card) SNDRV_CTL_POWER_D0
#define snd_power_change_state(card, state) do { ; } while (0)
#define snd_card_set_pm_callback(card,suspend,resume,data) -EINVAL
#define snd_card_set_isa_pm_callback(card,suspend,resume,data) -EINVAL
#define SND_PCI_PM_CALLBACKS
#endif

/* device.c */

struct _snd_minor {
	struct list_head list;		/* list of all minors per card */
	int number;			/* minor number */
	int device;			/* device number */
	const char *comment;		/* for /proc/asound/devices */
	snd_info_entry_t *dev;		/* for /proc/asound/dev */
        struct file_operations *f_ops;	/* file operations */
        char name[1];
};

/* sound.c */

extern int snd_ecards_limit;
extern int snd_device_mode;
extern int snd_device_gid;
extern int snd_device_uid;

void snd_request_card(int card);

int snd_register_device(int type, struct snd_card *card, int dev, snd_minor_t *reg, const char *name);
int snd_unregister_device(int type, struct snd_card *card, int dev);

#ifdef CONFIG_SND_OSSEMUL
int snd_register_oss_device(int type, struct snd_card *card, int dev, snd_minor_t *reg, const char *name);
int snd_unregister_oss_device(int type, struct snd_card *card, int dev);
#endif

int snd_minor_info_init(void);
int snd_minor_info_done(void);

/* sound_oss.c */

#ifdef CONFIG_SND_OSSEMUL

int snd_minor_info_oss_init(void);
int snd_minor_info_oss_done(void);

int snd_oss_init_module(void);
void snd_oss_cleanup_module(void);

#endif

/* memory.c */

#ifdef CONFIG_SND_DEBUG_MEMORY
void snd_memory_init(void);
void snd_memory_done(void);
int snd_memory_info_init(void);
int snd_memory_info_done(void);
void *snd_hidden_kmalloc(size_t size, int flags);
void *snd_hidden_kcalloc(size_t n, size_t size, int flags);
void snd_hidden_kfree(const void *obj);
void *snd_hidden_vmalloc(unsigned long size);
void snd_hidden_vfree(void *obj);
char *snd_hidden_kstrdup(const char *s, int flags);
#define kmalloc(size, flags) snd_hidden_kmalloc(size, flags)
#define kcalloc(n, size, flags) snd_hidden_kcalloc(n, size, flags)
#define kfree(obj) snd_hidden_kfree(obj)
#define vmalloc(size) snd_hidden_vmalloc(size)
#define vfree(obj) snd_hidden_vfree(obj)
#define kstrdup(s, flags)  snd_hidden_kstrdup(s, flags)
#endif

char *snd_hidden_kstrdup(const char *s, int flags);
#define kstrdup(s, flags)  snd_hidden_kstrdup(s, flags)

char *snd_kmalloc_strdup(const char *string, int flags);
int copy_to_user_fromio(void __user *dst, const volatile void __iomem *src, size_t count);
int copy_from_user_toio(volatile void __iomem *dst, const void __user *src, size_t count);

void *kzalloc(size_t size, unsigned int flags);

/* init.c */

extern int snd_cards_count;
extern struct snd_card *snd_cards[SNDRV_CARDS];
extern rwlock_t snd_card_rwlock;
#ifdef CONFIG_SND_OSSEMUL
extern int (*snd_mixer_oss_notify_callback)(struct snd_card *card, int free_flag);
#endif

struct snd_card *snd_card_new(int idx, const char *id,
			 struct module *module, int extra_size);
int snd_card_free(struct snd_card *card);
int snd_card_register(struct snd_card *card);
int snd_card_info_init(void);
int snd_card_info_done(void);
int snd_component_add(struct snd_card *card, const char *component);
int snd_card_file_add(struct snd_card *card, struct file *file);
int snd_card_file_remove(struct snd_card *card, struct file *file);

/* device.c */

int snd_device_new(struct snd_card *card, snd_device_type_t type,
		   void *device_data, struct snd_device_ops *ops);
int snd_device_register(struct snd_card *card, void *device_data);
int snd_device_register_all(struct snd_card *card);
int snd_device_free(struct snd_card *card, void *device_data);
int snd_device_free_all(struct snd_card *card, snd_device_cmd_t cmd);

/* isadma.c */

#define DMA_MODE_NO_ENABLE	0x0100

void snd_dma_program(unsigned long dma, const void *buf, unsigned int size, unsigned short mode);
void snd_dma_disable(unsigned long dma);
unsigned int snd_dma_residue(unsigned long dma);

/* misc.c */

int snd_task_name(struct task_struct *task, char *name, size_t size);

/* --- */

#define snd_printk(format, args...) do { \
	printk("ALSA %s:%d: ", __FILE__, __LINE__); \
	printk(format, ##args); \
} while (0)

#ifdef CONFIG_SND_DEBUG

#define snd_printd(format, args...) snd_printk(format, ##args)
#define snd_assert(expr, args...) do {\
	if (!(expr)) {\
		snd_printk("BUG? (%s) (called from %p)\n", __STRING(expr), __builtin_return_address(0));\
		args;\
	}\
} while (0)
#define snd_runtime_check(expr, args...) do {\
	if (!(expr)) {\
		snd_printk("ERROR (%s) (called from %p)\n", __STRING(expr), __builtin_return_address(0));\
		args;\
	}\
} while (0)

#else /* !CONFIG_SND_DEBUG */

#define snd_printd(format, args...)	/* nothing */
#define snd_assert(expr, args...)	/* nothing */
#define snd_runtime_check(expr, args...) do { if (!(expr)) { args; } } while (0)

#endif /* CONFIG_SND_DEBUG */

#ifdef CONFIG_SND_DEBUG_DETECT
#define snd_printdd(format, args...) snd_printk(format, ##args)
#else
#define snd_printdd(format, args...) /* nothing */
#endif

#define snd_BUG() snd_assert(0, )


#ifdef DEBUG
#define snd_BUG() _asm int 3;
#else
#define snd_BUG()
#endif

static inline void snd_timestamp_now(struct timespec *tstamp, int timespec)
{
        struct timeval val;
        /* FIXME: use a linear time source */
        do_gettimeofday(&val);
        tstamp->tv_sec = val.tv_sec;
        tstamp->tv_nsec = val.tv_usec;
        if (timespec)
                tstamp->tv_nsec *= 1000L;
}

#define snd_timestamp_zero(tstamp) do { (tstamp)->tv_sec = 0; (tstamp)->tv_usec = 0; } while (0)
#define snd_timestamp_null(tstamp) ((tstamp)->tv_sec == 0 && (tstamp)->tv_usec ==0)

#define SNDRV_OSS_VERSION         ((3<<16)|(8<<8)|(1<<4)|(0))	/* 3.8.1a */

/* kcalloc */
#ifndef CONFIG_HAVE_KCALLOC
#ifndef CONFIG_SND_DEBUG_MEMORY
void *snd_compat_kcalloc(size_t n, size_t size, int gfp_flags);
#define kcalloc(n,s,f) snd_compat_kcalloc(n,s,f)
#endif
#endif

#define __builtin_expect(x, expected_value) (x)
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

typedef u32 /*__bitwise*/ pm_message_t;

#ifndef __devexit_p
#define __devexit_p(x) x
#endif

#define printk_ratelimit()      1

typedef unsigned gfp_t;

#ifndef cpu_relax
#define cpu_relax()
#endif

static inline int snd_pci_enable_msi(struct pci_dev *dev) { return -1; }
#undef pci_enable_msi
#define pci_enable_msi(dev) snd_pci_enable_msi(dev)
#undef pci_disable_msi
#define pci_disable_msi(dev)

#undef pci_intx
#define pci_intx(pci,x)


#endif				/* __DRIVER_H */
