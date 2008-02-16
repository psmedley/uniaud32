/*
 *  Configuration header file for compilation of the ALSA driver
 */

#ifndef __ALSA_CONFIG_H__
#define __ALSA_CONFIG_H__

#define inline __inline
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
#include <linux/byteorder/little_endian.h>
#include <asm/ioctl.h>
#include <asm/hardirq.h>
#include <asm/processor.h>
#include <asm/siginfo.h>
#include <dbgos2.h>

extern int this_module[64];
#define THIS_MODULE (void *)&this_module[0]
#define MODULE_GENERIC_TABLE(gtype,name)
#define MODULE_DEVICE_TABLE(type,name)
#define CONFIG_PROC_FS
#define CONFIG_PM
#define PCI_NEW_SUSPEND
#define CONFIG_PCI
#define CONFIG_SND_SEQUENCER
//#define CONFIG_SND_OSSEMUL
#define SNDRV_LITTLE_ENDIAN
#define EXPORT_SYMBOL(a)
#define CONFIG_SOUND
#define ATTRIBUTE_UNUSED

#undef interrupt

/*
 * Power management requests
 */
enum
{
	PM_SUSPEND, /* enter D1-D3 */
	PM_RESUME,  /* enter D0 */

	/* enable wake-on */
	PM_SET_WAKEUP,

	/* bus resource management */
	PM_GET_RESOURCES,
	PM_SET_RESOURCES,

	/* base station management */
	PM_EJECT,
	PM_LOCK,
};

typedef int pm_request_t;

/*
 * Device types
 */
enum
{
	PM_UNKNOWN_DEV = 0, /* generic */
	PM_SYS_DEV,	    /* system device (fan, KB controller, ...) */
	PM_PCI_DEV,	    /* PCI device */
	PM_USB_DEV,	    /* USB device */
	PM_SCSI_DEV,	    /* SCSI device */
	PM_ISA_DEV,	    /* ISA device */
};

typedef int pm_dev_t;

/*
 * System device hardware ID (PnP) values
 */
enum
{
	PM_SYS_UNKNOWN = 0x00000000, /* generic */
	PM_SYS_KBC =	 0x41d00303, /* keyboard controller */
	PM_SYS_COM =	 0x41d00500, /* serial port */
	PM_SYS_IRDA =	 0x41d00510, /* IRDA controller */
	PM_SYS_FDC =	 0x41d00700, /* floppy controller */
	PM_SYS_VGA =	 0x41d00900, /* VGA controller */
	PM_SYS_PCMCIA =	 0x41d00e00, /* PCMCIA controller */
};

/*
 * Request handler callback
 */
struct pm_dev;

typedef int (*pm_callback)(struct pm_dev *dev, pm_request_t rqst, void *data);

/*
 * Dynamic device information
 */
struct pm_dev
{
	pm_dev_t	 type;
	unsigned long	 id;
	pm_callback	 callback;
	void		*data;

	unsigned long	 flags;
	int		 state;
	int		 prev_state;

	struct list_head entry;
};

int pm_init(void);
void pm_done(void);

#define CONFIG_PM

#define PM_IS_ACTIVE() 1

/*
 * Register a device with power management
 */
struct pm_dev *pm_register(pm_dev_t type,
			   unsigned long id,
			   pm_callback callback);

/*
 * Unregister a device with power management
 */
void pm_unregister(struct pm_dev *dev);

/*
 * Send a request to a single device
 */
int pm_send(struct pm_dev *dev, pm_request_t rqst, void *data);

#define fops_get(x) (x)
#define fops_put(x) do { ; } while (0)

#define __builtin_return_address(a)	0
#define SetPageReserved(a)		a
#define ClearPageReserved(a)		a
#define set_current_state(a)
#define try_inc_mod_count(x) 	        ++(*(unsigned long *)x)
#define try_module_get(x) try_inc_mod_count(x)

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

/* for easier backward-porting */
#if defined(CONFIG_GAMEPORT) || defined(CONFIG_GAMEPORT_MODULE)
#ifndef gameport_set_dev_parent
#define gameport_set_dev_parent(gp,xdev) ((gp)->dev.parent = (xdev))
#define gameport_set_port_data(gp,r) ((gp)->port_data = (r))
#define gameport_get_port_data(gp) (gp)->port_data
#endif
#endif

struct snd_minor {
	struct list_head list;		/* list of all minors per card */
	int number;			/* minor number */
	int device;			/* device number */
	const char *comment;		/* for /proc/asound/devices */
	struct file_operations *f_ops;	/* file operations */
	char name[1];			/* device name (keep at the end of
								structure) */
};

/* PCI quirk list helper */
struct snd_pci_quirk {
        unsigned short subvendor;       /* PCI subvendor ID */
        unsigned short subdevice;       /* PCI subdevice ID */
        int value;                      /* value */
#ifdef CONFIG_SND_DEBUG_DETECT
        const char *name;               /* name of the device (optional) */
#endif
};

#define _SND_PCI_QUIRK_ID(vend,dev) \
        .subvendor = (vend), .subdevice = (dev)
#define SND_PCI_QUIRK_ID(vend,dev) {_SND_PCI_QUIRK_ID(vend, dev)}
#ifdef CONFIG_SND_DEBUG_DETECT
#define SND_PCI_QUIRK(vend,dev,xname,val) \
        {_SND_PCI_QUIRK_ID(vend, dev), .value = (val), .name = (xname)}
#else
#define SND_PCI_QUIRK(vend,dev,xname,val) \
        {_SND_PCI_QUIRK_ID(vend, dev), .value = (val)}
#endif

const struct snd_pci_quirk *
snd_pci_quirk_lookup(struct pci_dev *pci, const struct snd_pci_quirk *list);

/* misc.c */
struct resource;
void release_and_free_resource(struct resource *res);

#include "typedefs.h"

#endif //__ALSA_CONFIG_H__
