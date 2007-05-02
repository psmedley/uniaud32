/*
 *  Misc and compatibility things
 *  Copyright (c) by Jaroslav Kysela <perex@suse.cz>
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
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <sound/driver.h>
#include <linux/firmware.h>
int snd_task_name(struct task_struct *task, char *name, size_t size)
{
	unsigned int idx;

	snd_assert(task != NULL && name != NULL && size >= 2, return -EINVAL);
	for (idx = 0; idx < sizeof(task->comm) && idx + 1 < size; idx++)
		name[idx] = task->comm[idx];
	name[idx] = '\0';
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)

int try_inc_mod_count(struct module *module)
{
	__MOD_INC_USE_COUNT(module);
	return 1;
}

struct resource snd_compat_mem_region;

struct resource *snd_compat_request_region(unsigned long start, unsigned long size, const char *name)
{
	struct resource *resource;

	resource = snd_kcalloc(sizeof(struct resource), GFP_KERNEL);
	if (resource == NULL)
		return NULL;
	if (check_region(start, size)) {
		kfree(resource);
		return NULL;
	}
	snd_wrapper_request_region(start, size, name);
	resource->name = name;
	resource->start = start;
	resource->end = start + size - 1;
	resource->flags = IORESOURCE_IO;
	return resource;
}

int snd_compat_release_resource(struct resource *resource)
{
	snd_runtime_check(resource != NULL, return -EINVAL);
	if (resource == &snd_compat_mem_region)
		return 0;
	release_region(resource->start, (resource->end - resource->start) + 1);
	kfree(resource);
	return 0;
}

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0) && defined(CONFIG_PCI)

/*
 *  Registration of PCI drivers and handling of hot-pluggable devices.
 */

static LIST_HEAD(pci_drivers);

struct pci_driver_mapping {
	struct pci_dev *dev;
	struct pci_driver *drv;
	unsigned long dma_mask;
	void *driver_data;
};

#define PCI_MAX_MAPPINGS 64
static struct pci_driver_mapping drvmap [PCI_MAX_MAPPINGS] = { { NULL, } , };


static struct pci_driver_mapping *get_pci_driver_mapping(struct pci_dev *dev)
{
	int i;
	
	for (i = 0; i < PCI_MAX_MAPPINGS; i++)
		if (drvmap[i].dev == dev)
			return &drvmap[i];
	return NULL;
}

void * snd_pci_compat_get_driver_data (struct pci_dev *dev)
{
	struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
	if (map)
		return map->driver_data;
	return NULL;
}


void snd_pci_compat_set_driver_data (struct pci_dev *dev, void *driver_data)
{
	struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
	if (map)
		map->driver_data = driver_data;
}


unsigned long snd_pci_compat_get_dma_mask (struct pci_dev *dev)
{
	struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
	if (map)
		return map->dma_mask;
	return 0;
}


void snd_pci_compat_set_dma_mask (struct pci_dev *dev, unsigned long mask)
{
	struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
	if (map)
		map->dma_mask = mask;
}


const struct pci_device_id * snd_pci_compat_match_device(const struct pci_device_id *ids, struct pci_dev *dev)
{
	u16 subsystem_vendor, subsystem_device;

	pci_read_config_word(dev, PCI_SUBSYSTEM_VENDOR_ID, &subsystem_vendor);
	pci_read_config_word(dev, PCI_SUBSYSTEM_ID, &subsystem_device);

	while (ids->vendor || ids->subvendor || ids->class_mask) {
		if ((ids->vendor == PCI_ANY_ID || ids->vendor == dev->vendor) &&
		    (ids->device == PCI_ANY_ID || ids->device == dev->device) &&
		    (ids->subvendor == PCI_ANY_ID || ids->subvendor == subsystem_vendor) &&
		    (ids->subdevice == PCI_ANY_ID || ids->subdevice == subsystem_device) &&
		    !((ids->class ^ dev->class) & ids->class_mask))
			return ids;
		ids++;
	}
	return NULL;
}

static int snd_pci_announce_device(struct pci_driver *drv, struct pci_dev *dev)
{
        int i;
	const struct pci_device_id *id;

	if (drv->id_table) {
		id = snd_pci_compat_match_device(drv->id_table, dev);
		if (!id)
			return 0;
	} else {
		id = NULL;
	}
        for (i = 0; i < PCI_MAX_MAPPINGS; i++) {
	        if (drvmap[i].dev == NULL) {
	                drvmap[i].dev = dev;
	                drvmap[i].drv = drv;
			drvmap[i].dma_mask = ~0UL;
	                break;
	        }
        }
        if (i >= PCI_MAX_MAPPINGS)
		return 0;
	if (drv->probe(dev, id) < 0) {
                 drvmap[i].dev = NULL;
	         return 0;
	}
	return 1;
}

int snd_pci_compat_register_driver(struct pci_driver *drv)
{
	struct pci_dev *dev;
	int count = 0;

	list_add_tail(&drv->node, &pci_drivers);
	pci_for_each_dev(dev) {
		struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
		if (! map)
			count += snd_pci_announce_device(drv, dev);
	}
	return count;
}

void snd_pci_compat_unregister_driver(struct pci_driver *drv)
{
	struct pci_dev *dev;

	list_del(&drv->node);
	pci_for_each_dev(dev) {
		struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
		if (map && map->drv == drv) {
			if (drv->remove)
				drv->remove(dev);
			map->dev = NULL;
			map->drv = NULL;
	        }
	}
}

unsigned long snd_pci_compat_get_size (struct pci_dev *dev, int n_base)
{
	u32 l, sz;
	int reg = PCI_BASE_ADDRESS_0 + (n_base << 2);

	pci_read_config_dword (dev, reg, &l);
	if (l == 0xffffffff)
		return 0;

	pci_write_config_dword (dev, reg, ~0);
	pci_read_config_dword (dev, reg, &sz);
	pci_write_config_dword (dev, reg, l);

	if (!sz || sz == 0xffffffff)
		return 0;
	if ((l & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY) {
		sz = ~(sz & PCI_BASE_ADDRESS_MEM_MASK);
	} else {
		sz = ~(sz & PCI_BASE_ADDRESS_IO_MASK) & 0xffff;
	}
	
	return sz;
}

int snd_pci_compat_get_flags (struct pci_dev *dev, int n_base)
{
	unsigned long foo = dev->base_address[n_base] & PCI_BASE_ADDRESS_SPACE;
	int flags = 0;
	
	if (foo == 0)
		flags |= IORESOURCE_MEM;
	if (foo == 1)
		flags |= IORESOURCE_IO;
	
	return flags;
}

/*
 *  Set power management state of a device.  For transitions from state D3
 *  it isn't as straightforward as one could assume since many devices forget
 *  their configuration space during wakeup.  Returns old power state.
 */
int snd_pci_compat_set_power_state(struct pci_dev *dev, int new_state)
{
	u32 base[5], romaddr;
	u16 pci_command, pwr_command;
	u8  pci_latency, pci_cacheline;
	int i, old_state;
	int pm = snd_pci_compat_find_capability(dev, PCI_CAP_ID_PM);

	if (!pm)
		return 0;
	pci_read_config_word(dev, pm + PCI_PM_CTRL, &pwr_command);
	old_state = pwr_command & PCI_PM_CTRL_STATE_MASK;
	if (old_state == new_state)
		return old_state;
	if (old_state == 3) {
		pci_read_config_word(dev, PCI_COMMAND, &pci_command);
		pci_write_config_word(dev, PCI_COMMAND, pci_command & ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY));
		for (i = 0; i < 5; i++)
			pci_read_config_dword(dev, PCI_BASE_ADDRESS_0 + i*4, &base[i]);
		pci_read_config_dword(dev, PCI_ROM_ADDRESS, &romaddr);
		pci_read_config_byte(dev, PCI_LATENCY_TIMER, &pci_latency);
		pci_read_config_byte(dev, PCI_CACHE_LINE_SIZE, &pci_cacheline);
		pci_write_config_word(dev, pm + PCI_PM_CTRL, new_state);
		for (i = 0; i < 5; i++)
			pci_write_config_dword(dev, PCI_BASE_ADDRESS_0 + i*4, base[i]);
		pci_write_config_dword(dev, PCI_ROM_ADDRESS, romaddr);
		pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
		pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, pci_cacheline);
		pci_write_config_byte(dev, PCI_LATENCY_TIMER, pci_latency);
		pci_write_config_word(dev, PCI_COMMAND, pci_command);
	} else
		pci_write_config_word(dev, pm + PCI_PM_CTRL, (pwr_command & ~PCI_PM_CTRL_STATE_MASK) | new_state);
	return old_state;
}

/*
 *  Initialize device before it's used by a driver. Ask low-level code
 *  to enable I/O and memory. Wake up the device if it was suspended.
 *  Beware, this function can fail.
 */
int snd_pci_compat_enable_device(struct pci_dev *dev)
{
	u16 pci_command;

	pci_read_config_word(dev, PCI_COMMAND, &pci_command);
	pci_write_config_word(dev, PCI_COMMAND, pci_command | (PCI_COMMAND_IO | PCI_COMMAND_MEMORY));
	snd_pci_compat_set_power_state(dev, 0);
	return 0;
}

int snd_pci_compat_find_capability(struct pci_dev *dev, int cap)
{
	u16 status;
	u8 pos, id;
	int ttl = 48;

	pci_read_config_word(dev, PCI_STATUS, &status);
	if (!(status & PCI_STATUS_CAP_LIST))
		return 0;
	pci_read_config_byte(dev, PCI_CAPABILITY_LIST, &pos);
	while (ttl-- && pos >= 0x40) {
		pos &= ~3;
		pci_read_config_byte(dev, pos + PCI_CAP_LIST_ID, &id);
		if (id == 0xff)
			break;
		if (id == cap)
			return pos;
		pci_read_config_byte(dev, pos + PCI_CAP_LIST_NEXT, &pos);
	}
	return 0;
}

static void *snd_pci_compat_alloc_consistent1(unsigned long dma_mask,
					      unsigned long size,
					      int hop)
{
	void *res;

	if (++hop > 10)
		return NULL;
	res = snd_malloc_pages(size, GFP_KERNEL | (dma_mask <= 0x00ffffff ? GFP_DMA : 0));
	if (res == NULL)
		return NULL;
	if ((virt_to_bus(res) & ~dma_mask) ||
	    ((virt_to_bus(res) + size - 1) & ~dma_mask)) {
		void *res1 = snd_pci_compat_alloc_consistent1(dma_mask, size, hop);
		snd_free_pages(res, size);
		return res1;
	}
	return res;
}

void *snd_pci_compat_alloc_consistent(struct pci_dev *dev,
				      long size,
				      dma_addr_t *dmaaddr)
{
	unsigned long dma_mask = snd_pci_compat_get_dma_mask(dev);
	void *res = snd_pci_compat_alloc_consistent1(dma_mask, size, 0);
	if (res != NULL)
		*dmaaddr = (dma_addr_t)virt_to_bus(res);
	return res;
}

void snd_pci_compat_free_consistent(struct pci_dev *dev, long size, void *ptr, dma_addr_t dmaaddr)
{
	snd_runtime_check(bus_to_virt(dmaaddr) == ptr, return);
	snd_free_pages(ptr, size);
}

int snd_pci_compat_dma_supported(struct pci_dev *dev, dma_addr_t mask)
{
	return 1;
}

#endif /* kernel version < 2.3.0 && CONFIG_PCI */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0) && defined(CONFIG_APM)

#include <linux/apm_bios.h>

static spinlock_t pm_devs_lock = SPIN_LOCK_UNLOCKED;
static LIST_HEAD(pm_devs);

#ifdef CONFIG_PCI
static struct pm_dev *pci_compat_pm_dev;
static int pci_compat_pm_callback(struct pm_dev *pdev, pm_request_t rqst, void *data)
{
	struct pci_dev *dev;
	switch (rqst) {
	case PM_SUSPEND:
		pci_for_each_dev(dev) {
			struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
			if (map->drv && map->drv->suspend)
				map->drv->suspend(dev);
		}
		break;
	case PM_RESUME:
		pci_for_each_dev(dev) {
			struct pci_driver_mapping *map = get_pci_driver_mapping(dev);
			if (map->drv && map->drv->resume)
				map->drv->resume(dev);
		}
		break;
	}	
	return 0;
}
#endif

static int snd_apm_callback(apm_event_t ev)
{
	struct list_head *entry;
	pm_request_t rqst;
	void *data;
	int status;
	
	switch (ev) {
	case APM_SYS_SUSPEND:
	case APM_USER_SUSPEND:
	case APM_CRITICAL_SUSPEND:
		rqst = PM_SUSPEND;
		data = (void *)3;
		break;
	case APM_NORMAL_RESUME:
	case APM_CRITICAL_RESUME:
	case APM_STANDBY_RESUME:		/* ??? */
		rqst = PM_RESUME;
		data = (void *)0;
		break;
	default:
		return 0;
	}
	for (entry = pm_devs.next; entry != &pm_devs; entry = entry->next) {
		struct pm_dev *dev = list_entry(entry, struct pm_dev, entry);
		if ((status = pm_send(dev, rqst, data)))
			return status;
	}
	return 0;
}

int __init pm_init(void)
{
	if (apm_register_callback(snd_apm_callback))
		snd_printk("apm_register_callback failure!\n");
#ifdef CONFIG_PCI
	pci_compat_pm_dev = pm_register(PM_PCI_DEV, 0, pci_compat_pm_callback);
#endif
	return 0;
}

void __exit pm_done(void)
{
#ifdef CONFIG_PCI
	if (pci_compat_pm_dev)
		pm_unregister(pci_compat_pm_dev);
#endif
	apm_unregister_callback(snd_apm_callback);
}

struct pm_dev *pm_register(pm_dev_t type,
			   unsigned long id,
			   pm_callback callback)
{
	struct pm_dev *dev = kmalloc(sizeof(struct pm_dev), GFP_KERNEL);

	if (dev) {
		unsigned long flags;
		
		memset(dev, 0, sizeof(*dev));
		dev->type = type;
		dev->id = id;
		dev->callback = callback;
		
		spin_lock_irqsave(&pm_devs_lock, flags);
		list_add(&dev->entry, &pm_devs);
		spin_unlock_irqrestore(&pm_devs_lock, flags);
	}
	return dev;
}

void pm_unregister(struct pm_dev *dev)
{
	if (dev) {
		unsigned long flags;
		
		spin_lock_irqsave(&pm_devs_lock, flags);
		list_del(&dev->entry);
		spin_unlock_irqrestore(&pm_devs_lock, flags);

		kfree(dev);
	}
}

int pm_send(struct pm_dev *dev, pm_request_t rqst, void *data)
{
	int status = 0;
	int prev_state, next_state;
	
	switch (rqst) {
	case PM_SUSPEND:
	case PM_RESUME:
		prev_state = dev->state;
		next_state = (int) data;
		if (prev_state != next_state) {
			if (dev->callback)
				status = (*dev->callback)(dev, rqst, data);
			if (!status) {
				dev->state = next_state;
				dev->prev_state = prev_state;
			}
		} else {
			dev->prev_state = prev_state;
		}
		break;
	default:
		if (dev->callback)
			status = (*dev->callback)(dev, rqst, data);
		break;
	}
	return status;
}

#endif /* kernel version < 2.3.0 && CONFIG_APM */

static void run_workqueue(struct workqueue_struct *wq)
{
	unsigned long flags;

	spin_lock_irqsave(&wq->lock, flags);
	while (!list_empty(&wq->worklist)) {
		struct work_struct *work = list_entry(wq->worklist.next,
						      struct work_struct, entry);
		void (*f) (void *) = work->func;
		void *data = work->data;

		list_del_init(wq->worklist.next);
		spin_unlock_irqrestore(&wq->lock, flags);
		clear_bit(0, &work->pending);
		f(data);
		spin_lock_irqsave(&wq->lock, flags);
		wake_up(&wq->work_done);
	}
	spin_unlock_irqrestore(&wq->lock, flags);
}

#if 0
void flush_workqueue(struct workqueue_struct *wq)
{
	if (0 /* wq->task == current */) {
		run_workqueue(wq);
	} else {
		wait_queue_t wait;

		init_waitqueue_entry(&wait, current);
		set_current_state(TASK_UNINTERRUPTIBLE);
		spin_lock_irq(&wq->lock);
		add_wait_queue(&wq->work_done, &wait);
		while (!list_empty(&wq->worklist)) {
			spin_unlock_irq(&wq->lock);
			schedule();
			spin_lock_irq(&wq->lock);
		}
		remove_wait_queue(&wq->work_done, &wait);
		spin_unlock_irq(&wq->lock);
	}
}
#endif 

void destroy_workqueue(struct workqueue_struct *wq)
{
#if 0
    flush_workqueue(wq);
	kill_proc(wq->task_pid, SIGKILL, 1);
	if (wq->task_pid >= 0)
            wait_for_completion(&wq->thread_exited);
#endif
	kfree(wq);
}

static int xworker_thread(void *data)
{
	struct workqueue_struct *wq = data;

	strcpy(current->comm, wq->name); /* FIXME: different names? */

	do {
            run_workqueue(wq);
#if 0
            wait_event_interruptible(wq->more_work, !list_empty(&wq->worklist));
#endif
	} while (!signal_pending(current));
#if 0
        complete_and_exit(&wq->thread_exited, 0);
#endif
}

struct workqueue_struct *create_workqueue(const char *name)
{
	struct workqueue_struct *wq;
	
	//BUG_ON(strlen(name) > 10);
	
	wq = kmalloc(sizeof(*wq), GFP_KERNEL);
	if (!wq)
		return NULL;
	memset(wq, 0, sizeof(*wq));
	
	spin_lock_init(&wq->lock);
	INIT_LIST_HEAD(&wq->worklist);
	init_waitqueue_head(&wq->more_work);
        init_waitqueue_head(&wq->work_done);
        wq->name = name;
#if 0
	init_completion(&wq->thread_exited);
	wq->task_pid = kernel_thread(xworker_thread, wq, 0);
	if (wq->task_pid < 0) {
		printk(KERN_ERR "snd: failed to start thread %s\n", name);
		destroy_workqueue(wq);
		wq = NULL;
        }
#endif
	return wq;
}

static void __x_queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	unsigned long flags;

	spin_lock_irqsave(&wq->lock, flags);
	work->wq_data = wq;
	list_add_tail(&work->entry, &wq->worklist);
	wake_up(&wq->more_work);
	spin_unlock_irqrestore(&wq->lock, flags);
}

int queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	if (!test_and_set_bit(0, &work->pending)) {
		__x_queue_work(wq, work);
		return 1;
	}
	return 0;
}

static void delayed_work_timer_fn(unsigned long __data)
{
	struct work_struct *work = (struct work_struct *)__data;
	struct workqueue_struct *wq = work->wq_data;
	
	__x_queue_work(wq, work);
}

int queue_delayed_work(struct workqueue_struct *wq, struct work_struct *work, unsigned long delay)
{
	struct timer_list *timer = &work->timer;

	if (!test_and_set_bit(0, &work->pending)) {
		work->wq_data = work;
		timer->expires = jiffies + delay;
		timer->data = (unsigned long)work;
		timer->function = delayed_work_timer_fn;
		add_timer(timer);
		return 1;
	}
	return 0;
}

void release_and_free_resource(struct resource *res)
{
        if (res) {
                release_resource(res);
                kfree(res);
        }
}

int mod_firmware_load(const char *fn, char **fp)
{
    return 0;
}

static int snd_try_load_firmware(const char *path, const char *name,
				 struct firmware *firmware)
{
	char filename[30 + FIRMWARE_NAME_MAX];

	sprintf(filename, "%s/%s", path, name);
	firmware->size = mod_firmware_load(filename, (char **)&firmware->data);
	if (firmware->size)
		printk(KERN_INFO "Loaded '%s'.", filename);
	return firmware->size;
}

int request_firmware(const struct firmware **fw, const char *name,
		     struct device *device)
{
	struct firmware *firmware;

	*fw = NULL;
	firmware = kmalloc(sizeof *firmware, GFP_KERNEL);
	if (!firmware)
		return -ENOMEM;
	if (!snd_try_load_firmware("/lib/firmware", name, firmware) &&
	    !snd_try_load_firmware("/lib/hotplug/firmware", name, firmware) &&
	    !snd_try_load_firmware("/usr/lib/hotplug/firmware", name, firmware)) {
		kfree(firmware);
		return -EIO;
	}
	*fw = firmware;
	return 0;
}

void release_firmware(const struct firmware *fw)
{
	if (fw) {
		vfree(fw->data);
		kfree(fw);
	}
}

/**
 * snd_pci_quirk_lookup - look up a PCI SSID quirk list
 * @pci: pci_dev handle
 * @list: quirk list, terminated by a null entry
 *
 * Look through the given quirk list and finds a matching entry
 * with the same PCI SSID.  When subdevice is 0, all subdevice
 * values may match.
 *
 * Returns the matched entry pointer, or NULL if nothing matched.
 */
const struct snd_pci_quirk *
snd_pci_quirk_lookup(struct pci_dev *pci, const struct snd_pci_quirk *list)
{
	const struct snd_pci_quirk *q;

	for (q = list; q->subvendor; q++)
		if (q->subvendor == pci->subsystem_vendor &&
		    (!q->subdevice || q->subdevice == pci->subsystem_device))
			return q;
	return NULL;
}

