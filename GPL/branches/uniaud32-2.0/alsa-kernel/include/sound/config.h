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
#include <linux/byteorder/little_endian.h>
#include <asm/ioctl.h>
#include <asm/hardirq.h>
#include <asm/processor.h>
#include <asm/siginfo.h>
#include <dbgos2.h>
#include <limits.h>
#include <sys/cdefs.h>

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

/*
 *  ==========================================================================
 */

#ifdef ALSA_BUILD
#undef MODULE
#define MODULE
#endif

#include <linux/config.h>
#include <linux/version.h>

#define IRQ_NONE      (0)  /*void*/
#define IRQ_HANDLED   (1)  /*void*/
#define IRQ_RETVAL(x) ((x) != 0)  /*void*/
typedef int irqreturn_t;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 2, 3)
#error "This driver requires Linux 2.2.3 and higher."
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 2, 0)
#define LINUX_2_2
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 3, 1)
#define LINUX_2_3
#endif
#if defined(LINUX_2_3) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 0)
#error "This code requires Linux 2.4.0-test1 and higher."
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

#ifdef LINUX_2_2
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 2, 18)
#include <linux/init.h>
#endif
#include "compat_22.h"
#endif
#ifdef LINUX_2_3
#include <linux/init.h>
#include <linux/pm.h>
#define PCI_GET_DRIVER_DATA(pci) pci->driver_data
#define PCI_SET_DRIVER_DATA(pci, data) pci->driver_data = data
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 3)
//#define pci_set_dma_mask(pci, mask) pci->dma_mask = mask
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 7)
#define PCI_NEW_SUSPEND
#endif
#ifndef virt_to_page
#define virt_to_page(x) (&mem_map[MAP_NR(x)])
#endif
#define snd_request_region request_region
#ifndef rwlock_init
#define rwlock_init(x) do { *(x) = RW_LOCK_UNLOCKED; } while(0)
#endif
#define snd_kill_fasync(fp, sig, band) kill_fasync(fp, sig, band)
#if defined(__i386__) || defined(__ppc__)
/*
 * Here a dirty hack for 2.4 kernels.. See kernel/memory.c.
 */
#define HACK_PCI_ALLOC_CONSISTENT
void *snd_pci_hack_alloc_consistent(struct pci_dev *hwdev, size_t size,
				    dma_addr_t *dma_handle);
#undef pci_alloc_consistent
#define pci_alloc_consistent snd_pci_hack_alloc_consistent
#endif /* i386 or ppc */
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

#ifndef PCI_D0
#define PCI_D0     0
#define PCI_D1     1
#define PCI_D2     2
#define PCI_D3hot  3
#define PCI_D3cold 4
#define pci_choose_state(pci,state)     ((state) ? PCI_D3hot : PCI_D0)
#endif


#include <sound/asound.h>
#include <sound/asoundef.h>

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

struct delayed_work {
	struct work_struct work;
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
#define DECLARE_WORK(n, f, d)                           \
        struct work_struct n = __WORK_INITIALIZER(n, f, d)
int snd_compat_schedule_work(struct work_struct *work);
#define schedule_work(w) snd_compat_schedule_work(w)

/* Name change */
typedef struct timeval snd_timestamp_t;
#ifndef TARGET_OS2
typedef enum sndrv_card_type snd_card_type;
#endif
typedef union sndrv_pcm_sync_id snd_pcm_sync_id_t;
#ifndef TARGET_OS2
typedef enum sndrv_pcm_xrun snd_pcm_xrun_t;
#endif
#ifdef TARGET_OS2
//typedef struct snd_pcm_volume snd_pcm_volume_t;
#else
typedef enum sndrv_timer_global snd_timer_global_t;
#endif

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

#if defined(__alpha__) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 14)
#undef writeb
#define writeb(v, a) do { __writeb((v),(a)); mb(); } while(0)
#undef writew
#define writew(v, a) do {__writew((v),(a)); mb(); } while(0)
#undef writel
#define writel(v, a) do {__writel((v),(a)); mb(); } while(0)
#undef writeq
#define writeq(v, a) do {__writeq((v),(a)); mb(); } while(0)
#endif

/* do we have virt_to_bus? */
#if defined(CONFIG_SPARC64)
#undef HAVE_VIRT_TO_BUS
#else
#define HAVE_VIRT_TO_BUS  1
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
#define CONFIG_SYSFS_DEPRECATED
#undef interrupt

#define fops_get(x) (x)
#define fops_put(x) do { ; } while (0)
#define __builtin_return_address(a)	0
#define SetPageReserved(a)		a
#define ClearPageReserved(a)		a
#define set_current_state(a)
#define try_inc_mod_count(x) 	        ++(*(unsigned long *)x)
#define try_module_get(x) try_inc_mod_count(x)
static inline void module_put(struct module *module)
{
    if (module)
        do {} while(0);
}

#define rwlock_init(x) *(x) = RW_LOCK_UNLOCKED;

#define suser()	1

#define snd_kill_fasync(fp, sig, band) kill_fasync(*(fp), sig, band)


//what's this??
#define capable(a)	1
#define CAP_SYS_ADMIN   0

#define BITS_PER_LONG	32

#ifndef bitmap_member
#define bitmap_member(name,bits) \
	unsigned long name[((bits)+BITS_PER_LONG-1)/BITS_PER_LONG]
#endif

#ifndef snd_card_set_dev
#define snd_card_set_dev(card,devptr) ((card)->dev = (devptr))
#endif

/* for easier backward-porting */
#if defined(CONFIG_GAMEPORT) || defined(CONFIG_GAMEPORT_MODULE)
#ifndef gameport_set_dev_parent
#define gameport_set_dev_parent(gp,xdev) ((gp)->dev.parent = (xdev))
#define gameport_set_port_data(gp,r) ((gp)->port_data = (r))
#define gameport_get_port_data(gp) (gp)->port_data
#endif
#endif

/* misc.c */
struct resource;
void release_and_free_resource(struct resource *res);

#include "typedefs.h"
#define container_of(ptr, type, member) \
( (type *)( (char *)ptr - offsetof(type,member) ) )
_WCRTLINK extern int     snprintf( char *__buf, size_t __bufsize,
                                   const char *__fmt, ... );
#define offsetof(__typ,__id) ((size_t)&(((__typ*)0)->__id))
#define list_for_each_entry(itemptr, headptr, struct_listmember_name, container_type) \
    for (itemptr=(container_type *) \
         (((char *)((headptr)->next))-offsetof(container_type, struct_listmember_name)); \
         &(itemptr->struct_listmember_name)!=(headptr); \
         itemptr=(container_type *) \
         (((char *)(itemptr->struct_listmember_name.next))-offsetof(container_type, struct_listmember_name)))

#define list_for_each_entry_safe(itemptr, n, headptr, struct_listmember_name, container_type) \
    for (itemptr=(container_type *) \
         (((char *)((headptr)->next))-offsetof(container_type, struct_listmember_name)), \
         n=(container_type *) \
         (((char *)(itemptr->struct_listmember_name.next))-offsetof(container_type, struct_listmember_name)); \
         &(itemptr->struct_listmember_name)!=(headptr); \
         itemptr=n, \
         n=(container_type *) \
         (((char *)(n->struct_listmember_name.next))-offsetof(container_type, struct_listmember_name)))
#define flush_scheduled_work()

/* schedule_timeout_[un]interruptible() wrappers */
#include <linux/sched.h>
#define schedule_timeout_interruptible(x) \
	set_current_state(TASK_INTERRUPTIBLE); \
	schedule_timeout(x);

#define schedule_timeout_uninterruptible(x) \
	set_current_state(TASK_UNINTERRUPTIBLE); \
	schedule_timeout(x);


/* pm_message_t type */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
#include <linux/pm.h>
#ifndef PMSG_FREEZE
typedef u32 pm_message_t;
#define PMSG_FREEZE	3
#define PMSG_SUSPEND	3
#define PMSG_ON		0
#endif
#endif

#ifndef imajor
#define imajor(x) major((x)->i_rdev)
#endif

#ifndef __nocast
#define __nocast
#endif

#ifndef CONFIG_HAVE_KZALLOC
void *snd_compat_kzalloc(size_t n, unsigned int __nocast gfp_flags);
#define kzalloc(s,f) snd_compat_kzalloc(s,f)
#endif

static inline struct proc_dir_entry *PDE(const struct inode *inode)
{
	return (struct proc_dir_entry *) inode->u.generic_ip;
}

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
#define MODULE_ALIAS_CHARDEV_MAJOR(x)
typedef unsigned gfp_t;

#ifndef __devexit_p
#define __devexit_p(x) x
#endif

#define printk_ratelimit()      1
#define __builtin_expect(x, expected_value) (x)
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define smp_mb__after_clear_bit()	
int snd_compat_devfs_mk_dir(const char *dir, ...);
#define devfs_mk_dir snd_compat_devfs_mk_dir
int snd_compat_devfs_mk_cdev(dev_t dev, umode_t mode, const char *fmt, ...);
#define devfs_mk_cdev snd_compat_devfs_mk_cdev
void snd_compat_devfs_remove(const char *fmt, ...);
#define devfs_remove snd_compat_devfs_remove
#define SINGLE_DEPTH_NESTING                    1

struct iovec {
        char    *iov_base;      /* Base address. */
#ifdef __32BIT__
        size_t   iov_len;       /* Length. */
#else
        long     iov_len;       /* Length. */
#endif
};
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#ifndef IRQF_SHARED
#include <linux/signal.h>
#define IRQF_SHARED			SA_SHIRQ
#define IRQF_DISABLED			SA_INTERRUPT
#define IRQF_SAMPLE_RANDOM		SA_SAMPLE_RANDOM
#define IRQF_PERCPU			SA_PERCPU
#ifdef SA_PROBEIRQ
#define IRQF_PROBE_SHARED		SA_PROBEIRQ
#else
#define IRQF_PROBE_SHARED		0 /* dummy */
#endif
#endif /* IRQ_SHARED */

#ifndef MODULE_FIRMWARE
#define MODULE_FIRMWARE(x)
#endif

typedef irqreturn_t (*snd_irq_handler_t)(int, void *);
#define irq_handler_t snd_irq_handler_t
#undef irq_handler_t
#define irq_handler_t snd_irq_handler_t
int request_irq(unsigned int, irq_handler_t handler,
		    unsigned long, const char *, void *);

#ifndef MODULE_ALIAS
#define MODULE_ALIAS(x)
#define MODULE_ALIAS_CHARDEV_MAJOR(x)
#endif

#ifndef cpu_relax
#define cpu_relax()
#endif

#ifndef roundup
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#endif

/*
 * lockdep macros
 */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 17)
#define lockdep_set_class(lock, key)		do { (void)(key); } while (0)
#define down_read_nested(sem, subclass)		down_read(sem)
#define down_write_nested(sem, subclass)	down_write(sem)
#define down_read_non_owner(sem)		down_read(sem)
#define up_read_non_owner(sem)			up_read(sem)
#define spin_lock_nested(lock, x)		spin_lock(lock)
#define spin_lock_irqsave_nested(lock, f, x)	spin_lock_irqsave(lock, f)
#endif

/* msleep */
#ifndef CONFIG_HAVE_MSLEEP
void snd_compat_msleep(unsigned int msecs);
#define msleep snd_compat_msleep
#endif

#define do_posix_clock_monotonic_gettime getnstimeofday

static inline unsigned char snd_pci_revision(struct pci_dev *pci)
{
	unsigned char rev;
	pci_read_config_byte(pci, PCI_REVISION_ID, &rev);
	return rev;
}

#ifndef ALIGN
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#endif

/* pci_intx() wrapper */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)
#ifdef CONFIG_PCI
#undef pci_intx
#define pci_intx(pci,x)
#endif
#endif

/* MSI */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 8)
static inline int snd_pci_enable_msi(struct pci_dev *dev) { return -1; }
#undef pci_enable_msi
#define pci_enable_msi(dev) snd_pci_enable_msi(dev)
#undef pci_disable_msi
#define pci_disable_msi(dev)
#endif

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

#define assert(a)
#define CONFIG_SND_YMFPCI_FIRMWARE_IN_KERNEL
#define CONFIG_SND_MAESTRO3_FIRMWARE_IN_KERNEL

#ifndef fastcall
#define fastcall
#endif

/* dump_stack hack */
#ifndef CONFIG_HAVE_DUMP_STACK
#undef dump_stack
#define dump_stack()
#endif

#define pci_dev_present(x) snd_pci_dev_present(x)

/* definitions for functions in pci_iomap_compat.c */
#ifndef fastcall
#define fastcall
#endif

unsigned int fastcall ioread8(void __iomem *addr);
unsigned int fastcall ioread16(void __iomem *addr);
unsigned int fastcall ioread32(void __iomem *addr);
void fastcall iowrite8(u8 val, void __iomem *addr);
void fastcall iowrite16(u16 val, void __iomem *addr);
void fastcall iowrite32(u32 val, void __iomem *addr);
void __iomem *ioport_map(unsigned long port, unsigned int nr);
void ioport_unmap(void __iomem *addr);
void __iomem *pci_iomap(struct pci_dev *dev, int bar, unsigned long maxlen);
void pci_iounmap(struct pci_dev *dev, void __iomem * addr);

#endif //__ALSA_CONFIG_H__
