#ifndef _LINUX_DEVICE_H
#define _LINUX_DEVICE_H

#include <linux/types.h>
#include <linux/kobject.h>
#include <linux/pm.h>
#include <linux/sysfs.h>
#include <linux/lockdep.h>
#include <linux/overflow.h>

struct device;
struct device_private;
struct device_driver;
struct driver_private;
struct class;
struct subsys_private;
struct bus_type;
struct device_node;

struct bus_attribute {
	struct attribute	attr;
	ssize_t (*show)(struct bus_type *bus, char *buf);
	ssize_t (*store)(struct bus_type *bus, const char *buf, size_t count);
};

/*
 * The type of device, "struct device" is embedded in. A class
 * or bus can contain devices of different types
 * like "partitions" and "disks", "mouse" and "event".
 * This identifies the device type and carries type-specific
 * information, equivalent to the kobj_type of a kobject.
 * If "name" is specified, the uevent will contain it in
 * the DEVTYPE variable.
 */
struct device_type {
	const char *name;
	const struct attribute_group **groups;
	int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
/*	char *(*devnode)(struct device *dev, umode_t *mode,
			 kuid_t *uid, kgid_t *gid);*/
	void (*release)(struct device *dev);

	const struct dev_pm_ops *pm;
};

struct device_dma_parameters {
	/*
	 * a low level driver may set these to teach IOMMU code about
	 * sg limitations.
	 */
	unsigned int max_segment_size;
	unsigned int min_align_mask;
	unsigned long segment_boundary_mask;
};

typedef struct device {
    struct pci_dev *pci;  /* for PCI and PCI-SG types */
  struct device   * parent;
  struct device_private *p;
  struct bus_type * bus;    /* type of bus device is on */
  struct kobject kobj;
  const char		*init_name; /* initial name of the device */
  const struct device_type *type;
#if 0
  char  bus_id[BUS_ID_SIZE];  /* position on parent bus */
#endif
  dev_t			devt;	/* dev_t, creates the sysfs "dev" */
  void  (*release)(struct device * dev);
    unsigned int flags; /* GFP_XXX for continous and ISA types */
  struct semaphore		mutex;	/* mutex to synchronize calls to
					 * its driver.
					 */
#ifdef CONFIG_SBUS
    struct sbus_dev *sbus;  /* for SBUS type */
#endif
  void *private_data;
  void *platform_data;
  struct dev_pm_info	power;
	struct device_dma_parameters *dma_parms;
  struct list_head	dma_pools;	/* dma pools (if dma'ble) */
  struct device_driver *driver;
  void		*driver_data;	/* Driver data, set and get with
					   dev_set_drvdata/dev_get_drvdata */
  struct pm_dev *pm_dev;
  char  bus_id[20];
  struct class		*class;
  spinlock_t		devres_lock;
  struct list_head	devres_head;
  const struct attribute_group **groups;	/* optional groups */
	struct dma_map_ops *dma_ops;
	u64		*dma_mask;	/* dma mask (if dma'able device) */
	u64		coherent_dma_mask; /* Like dma_mask, but for
					     alloc_coherent mappings as
					     not all hardware supports
					     64 bit addresses for consistent
					     allocations such descriptors. */
  struct device_node	*of_node; /* associated device tree node */
} device;

static inline struct device *kobj_to_dev(struct kobject *kobj)
{
	return container_of(kobj, struct device, kobj);
}

static inline const char *dev_name(const struct device *dev)
{
#if 0
	/* Use the init name until the kobject becomes available */
	if (dev->init_name)
		return dev->init_name;

	return kobject_name(&dev->kobj);
#else
	return "uniaud32";
#endif
}

struct bus_type {
	const char		*name;
	const char		*dev_name;
	int (*match)(struct device *dev, struct device_driver *drv);
	int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
	int (*probe)(struct device *dev);
	int (*remove)(struct device *dev);
	struct subsys_private *p;
	struct lock_class_key lock_key;
};

struct device_driver {
	const char		*name;
	struct bus_type		*bus;

	struct module		*owner;
	const char		*mod_name;	/* used for built-in modules */

	bool suppress_bind_attrs;	/* disables bind/unbind via sysfs */

	const struct of_device_id	*of_match_table;

	int (*probe) (struct device *dev);
	int (*remove) (struct device *dev);
	void (*shutdown) (struct device *dev);
	int (*suspend) (struct device *dev, u32 state);
	int (*resume) (struct device *dev);
	const struct attribute_group **groups;

	const struct dev_pm_ops *pm;

	struct driver_private *p;
};

struct device_attribute {
	struct attribute	attr;
	ssize_t (*show)(struct device *dev, struct device_attribute *attr,
			char *buf);
	ssize_t (*store)(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count);
};

extern int __must_check driver_register(struct device_driver *drv);
extern void driver_unregister(struct device_driver *drv);
extern struct device_driver *driver_find(const char *name,
					 struct bus_type *bus);

#define MODULE_ALIAS_CHARDEV_MAJOR(x)

#define dev_dbg_ratelimited dev_dbg
#define dev_emerg dev_dbg
#define dev_crit dev_dbg
#define dev_alert dev_dbg
#define dev_err dev_dbg
#define dev_warn dev_dbg
#define dev_notice dev_dbg
#define dev_info dev_dbg

#define dev_err_ratelimited dev_err

#define DEVICE_ATTR_RO(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_RO(_name)

int dev_dbg(const struct device *dev, const char *fmt, ...);



/*
 * get_device - atomically increment the reference count for the device.
 *
 */
extern struct device *get_device(struct device *dev);
extern void put_device(struct device *dev);

static inline int device_add(struct device *dev) { return 0; }
static inline void device_del(struct device *dev) { }
extern void device_initialize(struct device *dev);
extern int dev_set_name(struct device *dev, const char *name, ...);


static inline int device_is_registered(struct device *dev)
{
	return dev->kobj.state_in_sysfs;
}

static inline void device_enable_async_suspend(struct device *dev)
{
	if (!dev->power.is_prepared)
		dev->power.async_suspend = 1;
}

extern int __must_check bus_register(struct bus_type *bus);

static inline void bus_unregister(struct bus_type *bus) {}
int bus_for_each_dev(struct bus_type *bus, struct device *start, void *data,
		     int (*fn)(struct device *dev, void *data));

static inline void device_lock(struct device *dev)
{
	mutex_lock(&dev->mutex);
}

static inline void device_unlock(struct device *dev)
{
	mutex_unlock(&dev->mutex);
}

extern int  device_attach(struct device *dev);
//static inline int device_attach(struct device *dev) {return 0;}
extern void devres_add(struct device *dev, void *res);
extern int driver_attach(struct device_driver *drv);

/* device resource management */
typedef void (*dr_release_t)(struct device *dev, void *res);
typedef int (*dr_match_t)(struct device *dev, void *res, void *match_data);

#define	NUMA_NO_NODE	(-1)
void *__devres_alloc_node(dr_release_t release, size_t size, gfp_t gfp,
			  int nid, const char *name);
#define devres_alloc(release, size, gfp) \
	__devres_alloc_node(release, size, gfp, NUMA_NO_NODE, #release)

/**
 * struct class - device classes
 * @name:	Name of the class.
 * @owner:	The module owner.
 * @class_attrs: Default attributes of this class.
 * @dev_attrs:	Default attributes of the devices belong to the class.
 * @dev_bin_attrs: Default binary attributes of the devices belong to the class.
 * @dev_kobj:	The kobject that represents this class and links it into the hierarchy.
 * @dev_uevent:	Called when a device is added, removed from this class, or a
 *		few other things that generate uevents to add the environment
 *		variables.
 * @devnode:	Callback to provide the devtmpfs.
 * @class_release: Called to release this class.
 * @dev_release: Called to release the device.
 * @suspend:	Used to put the device to sleep mode, usually to a low power
 *		state.
 * @resume:	Used to bring the device from the sleep mode.
 * @ns_type:	Callbacks so sysfs can detemine namespaces.
 * @namespace:	Namespace of the device belongs to this class.
 * @pm:		The default device power management operations of this class.
 * @p:		The private data of the driver core, no one other than the
 *		driver core can touch this.
 *
 * A class is a higher-level view of a device that abstracts out low-level
 * implementation details. Drivers may see a SCSI disk or an ATA disk, but,
 * at the class level, they are all simply disks. Classes allow user space
 * to work with devices based on what they do, rather than how they are
 * connected or how they work.
 */
struct class {
	const char		*name;
	struct module		*owner;

	struct class_attribute		*class_attrs;
	struct device_attribute		*dev_attrs;
	struct bin_attribute		*dev_bin_attrs;
	struct kobject			*dev_kobj;

	int (*dev_uevent)(struct device *dev, struct kobj_uevent_env *env);
	char *(*devnode)(struct device *dev, mode_t *mode);

	void (*class_release)(struct class *class);
	void (*dev_release)(struct device *dev);

	int (*suspend)(struct device *dev, pm_message_t state);
	int (*resume)(struct device *dev);

	const struct kobj_ns_type_operations *ns_type;
	const void *(*namespace)(struct device *dev);

	const struct dev_pm_ops *pm;

	struct subsys_private *p;
};

extern int __must_check device_bind_driver(struct device *dev);
int bus_for_each_drv(struct bus_type *bus, struct device_driver *start,
		     void *data, int (*fn)(struct device_driver *, void *));
extern void devres_free(void *res);
extern void *devres_find(struct device *dev, dr_release_t release,
			 dr_match_t match, void *match_data);

/* debugging and troubleshooting/diagnostic helpers. */
extern const char *dev_driver_string(const struct device *dev);
void *devm_kmalloc(struct device *dev, size_t size, gfp_t gfp);

static inline void *devm_kzalloc(struct device *dev, size_t size, gfp_t gfp)
{
	return devm_kmalloc(dev, size, gfp | __GFP_ZERO);
}

static inline void *devm_kmalloc_array(struct device *dev,
				       size_t n, size_t size, gfp_t flags)
{
//	size_t bytes;

//	if (unlikely(check_mul_overflow(n, size, &bytes)))
//		return NULL;

	return devm_kmalloc(dev, n * size, flags);
}
static inline void *devm_kcalloc(struct device *dev,
				 size_t n, size_t size, gfp_t flags)
{
	return devm_kmalloc_array(dev, n, size, flags | __GFP_ZERO);
}

/* allows to add/remove a custom action to devres stack */
int devm_add_action(struct device *dev, void (*action)(void *), void *data);
void devm_remove_action(struct device *dev, void (*action)(void *), void *data);

static inline int dev_to_node(struct device *dev)
{
	return NUMA_NO_NODE;
}

static inline void *dev_get_drvdata(const struct device *dev)
{
	return dev->driver_data;
}

static inline void dev_set_drvdata(struct device *dev, void *data)
{
	dev->driver_data = data;
}

/* Generic device matching functions that all busses can use to match with */
int device_match_name(struct device *dev, const void *name);
int device_match_of_node(struct device *dev, const void *np);

#endif /* _LINUX_DEVICE_H */

