/*
 * based on drivers/base/core.c - core driver model code (device registration, etc)
 *
 * Copyright (c) 2002-3 Patrick Mochel
 * Copyright (c) 2002-3 Open Source Development Labs
 * Copyright (c) 2006 Greg Kroah-Hartman <gregkh@suse.de>
 * Copyright (c) 2006 Novell, Inc.
 *
 * This file is released under the GPLv2
 *
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kdev_t.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/pm_runtime.h>
#include <linux/sysfs.h>
#include <linux/kref.h>
#include <linux/kobject.h>
#include <linux/lockdep.h>

struct device_private {
/*	struct klist klist_children;
	struct klist_node knode_parent;
	struct klist_node knode_driver;
	struct klist_node knode_bus;
	struct list_head deferred_probe;*/
	struct device *device;
};
/**
 * device_release - free device structure.
 * @kobj: device's kobject.
 *
 * This is called once the reference count for the object
 * reaches 0. We forward the call to the device's release
 * method, which should handle actually freeing the structure.
 */
static void device_release(struct kobject *kobj)
{
	struct device *dev = kobj_to_dev(kobj);
	struct device_private *p = dev->p;

#if 0
	/*
	 * Some platform devices are driven without driver attached
	 * and managed resources may have been acquired.  Make sure
	 * all resources are released.
	 *
	 * Drivers still can add resources into device after device
	 * is deleted but alive, so release devres here to avoid
	 * possible memory leak.
	 */
	devres_release_all(dev);

	if (dev->release)
		dev->release(dev);
	else if (dev->type && dev->type->release)
		dev->type->release(dev);
	else if (dev->class && dev->class->dev_release)
		dev->class->dev_release(dev);
	else
		WARN(1, KERN_ERR "Device '%s' does not have a release() "
			"function, it is broken and must be fixed.\n",
			dev_name(dev));
#endif
	kfree(p);
}

static const void *device_namespace(struct kobject *kobj)
{
	struct device *dev = kobj_to_dev(kobj);
	const void *ns = NULL;

//	if (dev->class && dev->class->ns_type)
//		ns = dev->class->namespace(dev);

	return ns;
}

static struct kobj_type device_ktype = {
	.release	= device_release,
//	.sysfs_ops	= &dev_sysfs_ops,
	.namespace	= device_namespace,
};

/* /sys/devices/ */
struct kset *devices_kset;

/**
 * device_initialize - init device structure.
 * @dev: device.
 *
 * This prepares the device for use by other layers by initializing
 * its fields.
 * It is the first half of device_register(), if called by
 * that function, though it can also be called separately, so one
 * may use @dev's fields. In particular, get_device()/put_device()
 * may be used for reference counting of @dev after calling this
 * function.
 *
 * All fields in @dev must be initialized by the caller to 0, except
 * for those explicitly set to some other value.  The simplest
 * approach is to use kzalloc() to allocate the structure containing
 * @dev.
 *
 * NOTE: Use put_device() to give up your reference instead of freeing
 * @dev directly once you have called this function.
 */
void device_initialize(struct device *dev)
{
	dev->kobj.kset = devices_kset;
	kobject_init(&dev->kobj, &device_ktype);
	INIT_LIST_HEAD(&dev->dma_pools);
	mutex_init(&dev->mutex);
	lockdep_set_novalidate_class(&dev->mutex);
	spin_lock_init(&dev->devres_lock);
	INIT_LIST_HEAD(&dev->devres_head);
//	device_pm_init(dev);
//	set_dev_node(dev, -1);
}
EXPORT_SYMBOL_GPL(device_initialize);

/**
 * get_device - increment reference count for device.
 * @dev: device.
 *
 * This simply forwards the call to kobject_get(), though
 * we do take care to provide for the case that we get a NULL
 * pointer passed in.
 */
struct device *get_device(struct device *dev)
{
	return dev ? kobj_to_dev(kobject_get(&dev->kobj)) : NULL;
}
EXPORT_SYMBOL_GPL(get_device);

/**
 * put_device - decrement reference count.
 * @dev: device in question.
 */
void put_device(struct device *dev)
{
	/* might_sleep(); */
	if (dev)
		kobject_put(&dev->kobj);
}
EXPORT_SYMBOL_GPL(put_device);

