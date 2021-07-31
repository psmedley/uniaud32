/*
 * driver.c - centralized device driver management
 *
 * Copyright (c) 2002-3 Patrick Mochel
 * Copyright (c) 2002-3 Open Source Development Labs
 * Copyright (c) 2007 Greg Kroah-Hartman <gregkh@suse.de>
 * Copyright (c) 2007 Novell Inc.
 *
 * This file is released under the GPLv2
 *
 */

// current based on linux 3.0.95 code

#include <linux/device.h>
#include <linux/printk.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/klist.h>
#include <linux/notifier.h>
#include <linux/pm_runtime.h>
#include "base.h"

#define devres_log(dev, node, op)	do {} while (0)

struct devres_node {
	struct list_head		entry;
	dr_release_t			release;
#ifdef CONFIG_DEBUG_DEVRES
	const char			*name;
	size_t				size;
#endif
};

struct devres {
	struct devres_node		node;
	/* -- 3 pointers */
	unsigned long long		data[1];	/* guarantee ull alignment */
};


/**
 * dev_set_name - set a device name
 * @dev: device
 * @fmt: format string for the device's name
 */
int dev_set_name(struct device *dev, const char *fmt, ...)
{
	va_list vargs;
	int err;

	va_start(vargs, fmt);
	err = kobject_set_name_vargs(&dev->kobj, fmt, vargs);
	va_end(vargs);
	return err;
}

static void add_dr(struct device *dev, struct devres_node *node)
{
	devres_log(dev, node, "ADD");
	BUG_ON(!list_empty(&node->entry));
	list_add_tail(&node->entry, &dev->devres_head);
}

/**
 * devres_add - Register device resource
 * @dev: Device to add resource to
 * @res: Resource to register
 *
 * Register devres @res to @dev.  @res should have been allocated
 * using devres_alloc().  On driver detach, the associated release
 * function will be invoked and devres will be freed automatically.
 */
void devres_add(struct device *dev, void *res)
{
	struct devres *dr = container_of(res, struct devres, data);
	unsigned long flags;

	spin_lock_irqsave(&dev->devres_lock, flags);
	add_dr(dev, &dr->node);
	spin_unlock_irqrestore(&dev->devres_lock, flags);
}

static struct device *next_device(struct klist_iter *i)
{
	struct klist_node *n = klist_next(i);
	struct device *dev = NULL;
	struct device_private *dev_prv;

	if (n) {
		dev_prv = to_device_private_driver(n);
		dev = dev_prv->device;
	}
	return dev;
}

#define to_bus_attr(_attr) container_of(_attr, struct bus_attribute, attr)

static struct bus_type *bus_get(struct bus_type *bus)
{
	if (bus) {
		kset_get(&bus->p->subsys);
		return bus;
	}
	return NULL;
}

static void bus_put(struct bus_type *bus)
{
	if (bus)
		kset_put(&bus->p->subsys);
}

static void driver_release(struct kobject *kobj)
{
	struct driver_private *drv_priv = to_driver(kobj);

	pr_debug("driver: '%s': %s\n", kobject_name(kobj), __func__);
	kfree(drv_priv);
}

static struct kobj_type driver_ktype = {
//	.sysfs_ops	= &driver_sysfs_ops,
	.release	= driver_release,
};

/*
 * sysfs bindings for buses
 */
static ssize_t bus_attr_show(struct kobject *kobj, struct attribute *attr,
			     char *buf)
{
	struct bus_attribute *bus_attr = to_bus_attr(attr);
	struct subsys_private *subsys_priv = to_subsys_private(kobj);
	ssize_t ret = 0;

	if (bus_attr->show)
		ret = bus_attr->show(subsys_priv->bus, buf);
	return ret;
}

static ssize_t bus_attr_store(struct kobject *kobj, struct attribute *attr,
			      const char *buf, size_t count)
{
	struct bus_attribute *bus_attr = to_bus_attr(attr);
	struct subsys_private *subsys_priv = to_subsys_private(kobj);
	ssize_t ret = 0;

	if (bus_attr->store)
		ret = bus_attr->store(subsys_priv->bus, buf, count);
	return ret;
}

static const struct sysfs_ops bus_sysfs_ops = {
	.show	= bus_attr_show,
	.store	= bus_attr_store,
};

static struct kobj_type bus_ktype = {
	.sysfs_ops	= &bus_sysfs_ops,
};


static void klist_devices_get(struct klist_node *n)
{
	struct device_private *dev_prv = to_device_private_bus(n);
	struct device *dev = dev_prv->device;

	get_device(dev);
}

static void klist_devices_put(struct klist_node *n)
{
	struct device_private *dev_prv = to_device_private_bus(n);
	struct device *dev = dev_prv->device;

	put_device(dev);
}

static struct kset *bus_kset;

/**
 * bus_add_driver - Add a driver to the bus.
 * @drv: driver.
 */
int bus_add_driver(struct device_driver *drv)
{
	struct bus_type *bus;
	struct driver_private *priv;
	int error = 0;

	bus = bus_get(drv->bus);
	if (!bus)
		return -EINVAL;

	pr_debug("bus: '%s': add driver %s\n", bus->name, drv->name);

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		error = -ENOMEM;
		goto out_put_bus;
	}
	klist_init(&priv->klist_devices, NULL, NULL);
	priv->driver = drv;
	drv->p = priv;

	priv->kobj.kset = bus->p->drivers_kset; // traps?
	error = kobject_init_and_add(&priv->kobj, &driver_ktype, NULL,
				     "%s", drv->name);
	if (error)
		goto out_unregister;

	if (drv->bus->p->drivers_autoprobe) {
		error = driver_attach(drv);
		if (error)
			goto out_unregister;
	}
	klist_add_tail(&priv->knode_bus, &bus->p->klist_drivers);

	module_add_driver(drv->owner, drv);

#if 0 //not required ?
	error = driver_create_file(drv, &driver_attr_uevent);
	if (error) {
		printk(KERN_ERR "%s: uevent attr (%s) failed\n",
			__func__, drv->name);
	}
	error = driver_add_attrs(bus, drv);
	if (error) {
		/* How the hell do we get out of this pickle? Give up */
		printk(KERN_ERR "%s: driver_add_attrs(%s) failed\n",
			__func__, drv->name);
	}

	if (!drv->suppress_bind_attrs) {
		error = add_bind_files(drv);
		if (error) {
			/* Ditto */
			printk(KERN_ERR "%s: add_bind_files(%s) failed\n",
				__func__, drv->name);
		}
	}
#endif
//	kobject_uevent(&priv->kobj, KOBJ_ADD);
	return 0;

out_unregister:
	kobject_put(&priv->kobj);
	kfree(drv->p);
	drv->p = NULL;
out_put_bus:
	bus_put(bus);
	return error;
}

/**
 * bus_remove_driver - delete driver from bus's knowledge.
 * @drv: driver.
 *
 * Detach the driver from the devices it controls, and remove
 * it from its bus's list of drivers. Finally, we drop the reference
 * to the bus we took in bus_add_driver().
 */
void bus_remove_driver(struct device_driver *drv)
{
	if (!drv->bus)
		return;

//	if (!drv->suppress_bind_attrs)
//		remove_bind_files(drv);
//	driver_remove_attrs(drv->bus, drv);
//	driver_remove_file(drv, &driver_attr_uevent);
	klist_remove(&drv->p->knode_bus);
	pr_debug("bus: '%s': remove driver %s\n", drv->bus->name, drv->name);
	driver_detach(drv);
	module_remove_driver(drv);

	kobject_put(&drv->p->kobj);
	bus_put(drv->bus);
}

/**
 * bus_for_each_dev - device iterator.
 * @bus: bus type.
 * @start: device to start iterating from.
 * @data: data for the callback.
 * @fn: function to be called for each device.
 *
 * Iterate over @bus's list of devices, and call @fn for each,
 * passing it @data. If @start is not NULL, we use that device to
 * begin iterating from.
 *
 * We check the return of @fn each time. If it returns anything
 * other than 0, we break out and return that value.
 *
 * NOTE: The device that returns a non-zero value is not retained
 * in any way, nor is its refcount incremented. If the caller needs
 * to retain this data, it should do so, and increment the reference
 * count in the supplied callback.
 */
int bus_for_each_dev(struct bus_type *bus, struct device *start,
		     void *data, int (*fn)(struct device *, void *))
{
	struct klist_iter i;
	struct device *dev;
	int error = 0;

	if (!bus || !bus->p)
		return -EINVAL;

	klist_iter_init_node(&bus->p->klist_devices, &i,
			     (start ? &start->p->knode_bus : NULL));
	while ((dev = next_device(&i)) && !error)
		error = fn(dev, data);
	klist_iter_exit(&i);
	return error;
}
EXPORT_SYMBOL_GPL(bus_for_each_dev);

/**
 * driver_find - locate driver on a bus by its name.
 * @name: name of the driver.
 * @bus: bus to scan for the driver.
 *
 * Call kset_find_obj() to iterate over list of drivers on
 * a bus to find driver by name. Return driver if found.
 *
 * Note that kset_find_obj increments driver's reference count.
 */
struct device_driver *driver_find(const char *name, struct bus_type *bus)
{
	struct kobject *k = kset_find_obj(bus->p->drivers_kset, name); 
	struct driver_private *priv;

	if (k) {
		priv = to_driver(k);
		return priv->driver;
	}
	return NULL;
}

/**
 * driver_register - register driver with bus
 * @drv: driver to register
 *
 * We pass off most of the work to the bus_add_driver() call,
 * since most of the things we have to do deal with the bus
 * structures.
 */
int driver_register(struct device_driver *drv)
{
	int ret;
	struct device_driver *other;

	BUG_ON(!drv->bus->p);
#ifndef TARGET_OS2
	if ((drv->bus->probe && drv->probe) ||
	    (drv->bus->remove && drv->remove) ||
	    (drv->bus->shutdown && drv->shutdown))
		printk(KERN_WARNING "Driver '%s' needs updating - please use "
			"bus_type methods\n", drv->name);
#endif 
	other = driver_find(drv->name, drv->bus); 
	if (other) {
		printk(KERN_ERR "Error: Driver '%s' is already registered, "
			"aborting...\n", drv->name);
		return -EBUSY;
	}

	ret = bus_add_driver(drv); 
	if (ret)
		return ret;
#if 0
	ret = driver_add_groups(drv, drv->groups);
	if (ret) {
		bus_remove_driver(drv);
		return ret;
	}
	kobject_uevent(&drv->p->kobj, KOBJ_ADD);
#endif
	return ret;
}

/**
 * driver_unregister - remove driver from system.
 * @drv: driver.
 *
 * Again, we pass off most of the work to the bus-level call.
 */
void driver_unregister(struct device_driver *drv)
{
	if (!drv || !drv->p) {
		WARN(1, "Unexpected driver unregister!\n");
		return;
	}
//	driver_remove_groups(drv, drv->groups);
	bus_remove_driver(drv);
}
EXPORT_SYMBOL_GPL(driver_unregister);


/**
 * driver_find_device - device iterator for locating a particular device.
 * @drv: The device's driver
 * @start: Device to begin with
 * @data: Data to pass to match function
 * @match: Callback function to check device
 *
 * This is similar to the driver_for_each_device() function above, but
 * it returns a reference to a device that is 'found' for later use, as
 * determined by the @match callback.
 *
 * The callback should return 0 if the device doesn't match and non-zero
 * if it does.  If the callback returns non-zero, this function will
 * return to the caller and not iterate over any more devices.
 */
struct device *driver_find_device(struct device_driver *drv,
				  struct device *start, void *data,
				  int (*match)(struct device *dev, void *data))
{
	struct klist_iter i;
	struct device *dev;

	if (!drv)
		return NULL;

	klist_iter_init_node(&drv->p->klist_devices, &i,
			     (start ? &start->p->knode_driver : NULL));
	while ((dev = next_device(&i)))
		if (match(dev, data) && get_device(dev))
			break;
	klist_iter_exit(&i);
	return dev;
}
EXPORT_SYMBOL_GPL(driver_find_device);


/**
 * bus_register - register a bus with the system.
 * @bus: bus.
 *
 * Once we have that, we registered the bus with the kobject
 * infrastructure, then register the children subsystems it has:
 * the devices and drivers that belong to the bus.
 */
int bus_register(struct bus_type *bus)
{
	int retval;
	struct subsys_private *priv;

	priv = kzalloc(sizeof(struct subsys_private), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->bus = bus;
	bus->p = priv;

	BLOCKING_INIT_NOTIFIER_HEAD(&priv->bus_notifier);

	retval = kobject_set_name(&priv->subsys.kobj, "%s", bus->name);
	if (retval)
		goto out;

	priv->subsys.kobj.kset = bus_kset;
	priv->subsys.kobj.ktype = &bus_ktype;
	priv->drivers_autoprobe = 1;

	retval = kset_register(&priv->subsys);
	if (retval)
		goto out;

#if 0
	retval = bus_create_file(bus, &bus_attr_uevent);
	if (retval)
		goto bus_uevent_fail;
#endif
	priv->devices_kset = kset_create_and_add("devices", NULL,
						 &priv->subsys.kobj);
	if (!priv->devices_kset) {
		retval = -ENOMEM;
		goto bus_devices_fail;
	}

	priv->drivers_kset = kset_create_and_add("drivers", NULL,
						 &priv->subsys.kobj);
	if (!priv->drivers_kset) {
		retval = -ENOMEM;
		goto bus_drivers_fail;
	}

	klist_init(&priv->klist_devices, klist_devices_get, klist_devices_put);
	klist_init(&priv->klist_drivers, NULL, NULL);

#if 0
	retval = add_probe_files(bus);
	if (retval)
		goto bus_probe_files_fail;

	retval = bus_add_attrs(bus);
	if (retval)
		goto bus_attrs_fail;
#endif
	pr_debug("bus: '%s': registered\n", bus->name);
	return 0;

//bus_attrs_fail:
//	remove_probe_files(bus);
//bus_probe_files_fail:
//	kset_unregister(bus->p->drivers_kset);
bus_drivers_fail:
	kset_unregister(bus->p->devices_kset);
bus_devices_fail:
//	bus_remove_file(bus, &bus_attr_uevent);
//bus_uevent_fail:
	kset_unregister(&bus->p->subsys);
out:
	kfree(bus->p);
	bus->p = NULL;
	return retval;
}
EXPORT_SYMBOL_GPL(bus_register);

static int __driver_attach(struct device *dev, void *data)
{
	struct device_driver *drv = data;

	/*
	 * Lock device and try to bind to it. We drop the error
	 * here and always return 0, because we need to keep trying
	 * to bind to devices and some drivers will return an error
	 * simply if it didn't support the device.
	 *
	 * driver_probe_device() will spit a warning if there
	 * is an error.
	 */

	if (!driver_match_device(drv, dev))
		return 0;

	if (dev->parent)	/* Needed for USB */
		device_lock(dev->parent);
	device_lock(dev);
	if (!dev->driver)
		driver_probe_device(drv, dev);
	device_unlock(dev);
	if (dev->parent)
		device_unlock(dev->parent);

	return 0;
}

/**
 * driver_attach - try to bind driver to devices.
 * @drv: driver.
 *
 * Walk the list of devices that the bus has on it and try to
 * match the driver with each one.  If driver_probe_device()
 * returns 0 and the @dev->driver is set, we've found a
 * compatible pair.
 */
int driver_attach(struct device_driver *drv)
{
	return bus_for_each_dev(drv->bus, NULL, drv, __driver_attach);
}

static atomic_t probe_count = ATOMIC_INIT(0);
static DECLARE_WAIT_QUEUE_HEAD(probe_waitqueue);

static int really_probe(struct device *dev, struct device_driver *drv)
{
	int ret = 0;
	atomic_inc(&probe_count);
	pr_debug("bus: '%s': %s: probing driver %s with device %s\n",
		 drv->bus->name, __func__, drv->name, dev_name(dev));

	WARN_ON(!list_empty(&dev->devres_head));

	dev->driver = drv;

#if 0
	if (driver_sysfs_add(dev)) {
		printk(KERN_ERR "%s: driver_sysfs_add(%s) failed\n",
			__func__, dev_name(dev));
		goto probe_failed;
	}
#endif

	if (dev->bus->probe) {
		ret = dev->bus->probe(dev);
//		if (ret)
//			goto probe_failed;
	} else if (drv->probe) {
		ret = drv->probe(dev);
//		if (ret)
//			goto probe_failed;
	}
#ifdef TARGET_OS2xxx
	driver_bound(dev);
	ret = 1;
	pr_debug("bus: '%s': %s: bound device %s to driver %s\n",
		 drv->bus->name, __func__, dev_name(dev), drv->name);
	goto done;

probe_failed:
	devres_release_all(dev);
//	driver_sysfs_remove(dev);
	dev->driver = NULL;

	if (ret != -ENODEV && ret != -ENXIO) {
		/* driver matched but the probe failed */
		printk(KERN_WARNING
		       "%s: probe of %s failed with error %d\n",
		       drv->name, dev_name(dev), ret);
	}
	/*
	 * Ignore errors returned by ->probe so that the next driver can try
	 * its luck.
	 */
	ret = 0;
done:
	atomic_dec(&probe_count);
	wake_up(&probe_waitqueue);
#endif
	return ret;
}

/*
 * __device_release_driver() must be called with @dev lock held.
 * When called for a USB interface, @dev->parent lock must be held as well.
 */
static void __device_release_driver(struct device *dev)
{
	struct device_driver *drv;

	drv = dev->driver;
	if (drv) {
		pm_runtime_get_sync(dev);

#if 0
		driver_sysfs_remove(dev);

		if (dev->bus)
			blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
						     BUS_NOTIFY_UNBIND_DRIVER,
						     dev);
#endif
		pm_runtime_put_sync(dev);

		if (dev->bus && dev->bus->remove)
			dev->bus->remove(dev);
		else if (drv->remove)
			drv->remove(dev);
		devres_release_all(dev);
		dev->driver = NULL;
		klist_remove(&dev->p->knode_driver);
#if 0
		if (dev->bus)
			blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
						     BUS_NOTIFY_UNBOUND_DRIVER,
						     dev);
#endif
	}
}

/**
 * driver_detach - detach driver from all devices it controls.
 * @drv: driver.
 */
void driver_detach(struct device_driver *drv)
{
	struct device_private *dev_prv;
	struct device *dev;

	for (;;) {
		spin_lock(&drv->p->klist_devices.k_lock);
		if (list_empty(&drv->p->klist_devices.k_list)) {
			spin_unlock(&drv->p->klist_devices.k_lock);
			break;
		}
		dev_prv = list_entry(drv->p->klist_devices.k_list.prev,
				     struct device_private,
				     knode_driver.n_node);
		dev = dev_prv->device;
		get_device(dev);
		spin_unlock(&drv->p->klist_devices.k_lock);

		if (dev->parent)	/* Needed for USB */
			device_lock(dev->parent);
		device_lock(dev);
		if (dev->driver == drv)
			__device_release_driver(dev);
		device_unlock(dev);
		if (dev->parent)
			device_unlock(dev->parent);
		put_device(dev);
	}
}

/**
 * driver_probe_device - attempt to bind device & driver together
 * @drv: driver to bind a device to
 * @dev: device to try to bind to the driver
 *
 * This function returns -ENODEV if the device is not registered,
 * 1 if the device is bound successfully and 0 otherwise.
 *
 * This function must be called with @dev lock held.  When called for a
 * USB interface, @dev->parent lock must be held as well.
 */
int driver_probe_device(struct device_driver *drv, struct device *dev)
{
	int ret = 0;
#if 0
	if (!device_is_registered(dev))
		return -ENODEV;
#endif
	pr_debug("bus: '%s': %s: matched device %s with driver %s\n",
		 drv->bus->name, __func__, dev_name(dev), drv->name);
	pm_runtime_get_noresume(dev);
	pm_runtime_barrier(dev);

	ret = really_probe(dev, drv);
	pm_runtime_put_sync(dev);
	return ret;
}

static int __device_attach(struct device_driver *drv, void *data)
{
	struct device *dev = data;

	if (!driver_match_device(drv, dev))
		return 0;

	return driver_probe_device(drv, dev);
}

/**
 * device_attach - try to attach device to a driver.
 * @dev: device.
 *
 * Walk the list of drivers that the bus has and call
 * driver_probe_device() for each pair. If a compatible
 * pair is found, break out and return.
 *
 * Returns 1 if the device was bound to a driver;
 * 0 if no matching driver was found;
 * -ENODEV if the device is not registered.
 *
 * When called for a USB interface, @dev->parent lock must be held.
 */
int device_attach(struct device *dev)
{
	int ret = 0;
	device_lock(dev);
	if (dev->driver) {
		if (klist_node_attached(&dev->p->knode_driver)) {
			ret = 1;
			goto out_unlock;
		}
		ret = device_bind_driver(dev);
		if (ret == 0)
			ret = 1;
		else {
			dev->driver = NULL;
			ret = 0;
		}
	} else {
		pm_runtime_get_noresume(dev);
		ret = bus_for_each_drv(dev->bus, NULL, dev, __device_attach);
		pm_runtime_put_sync(dev);
	}
out_unlock:
	device_unlock(dev);
	return ret;
}
EXPORT_SYMBOL_GPL(device_attach);

static struct device_driver *next_driver(struct klist_iter *i)
{
	struct klist_node *n = klist_next(i);
	struct driver_private *drv_priv;

	if (n) {
		drv_priv = container_of(n, struct driver_private, knode_bus);
		return drv_priv->driver;
	}
	return NULL;
}

/**
 * bus_for_each_drv - driver iterator
 * @bus: bus we're dealing with.
 * @start: driver to start iterating on.
 * @data: data to pass to the callback.
 * @fn: function to call for each driver.
 *
 * This is nearly identical to the device iterator above.
 * We iterate over each driver that belongs to @bus, and call
 * @fn for each. If @fn returns anything but 0, we break out
 * and return it. If @start is not NULL, we use it as the head
 * of the list.
 *
 * NOTE: we don't return the driver that returns a non-zero
 * value, nor do we leave the reference count incremented for that
 * driver. If the caller needs to know that info, it must set it
 * in the callback. It must also be sure to increment the refcount
 * so it doesn't disappear before returning to the caller.
 */
int bus_for_each_drv(struct bus_type *bus, struct device_driver *start,
		     void *data, int (*fn)(struct device_driver *, void *))
{
	struct klist_iter i;
	struct device_driver *drv;
	int error = 0;

	if (!bus)
		return -EINVAL;

	klist_iter_init_node(&bus->p->klist_drivers, &i,
			     start ? &start->p->knode_bus : NULL);
	while ((drv = next_driver(&i)) && !error)
		error = fn(drv, data);
	klist_iter_exit(&i);
	return error;
}
EXPORT_SYMBOL_GPL(bus_for_each_drv);

static void driver_bound(struct device *dev)
{
	if (klist_node_attached(&dev->p->knode_driver)) {
		printk(KERN_WARNING "%s: device %s already bound\n",
			__func__, kobject_name(&dev->kobj));
		return;
	}

	pr_debug("driver: '%s': %s: bound to device '%s'\n", dev_name(dev),
		 __func__, dev->driver->name);

	klist_add_tail(&dev->p->knode_driver, &dev->driver->p->klist_devices);

#if 0
	if (dev->bus)
		blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
					     BUS_NOTIFY_BOUND_DRIVER, dev);
#endif
}

/**
 * device_bind_driver - bind a driver to one device.
 * @dev: device.
 *
 * Allow manual attachment of a driver to a device.
 * Caller must have already set @dev->driver.
 *
 * Note that this does not modify the bus reference count
 * nor take the bus's rwsem. Please verify those are accounted
 * for before calling this. (It is ok to call with no other effort
 * from a driver's probe() method.)
 *
 * This function must be called with the device lock held.
 */
int device_bind_driver(struct device *dev)
{
	int ret=0;

//	ret = driver_sysfs_add(dev);
//	if (!ret)
		driver_bound(dev);
	return ret;
}
EXPORT_SYMBOL_GPL(device_bind_driver);
