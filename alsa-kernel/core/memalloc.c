/*
 *  Copyright (c) by Jaroslav Kysela <perex@suse.cz>
 *                   Takashi Iwai <tiwai@suse.de>
 *
 *  Generic memory allocators
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

#include <sound/driver.h>
#include <sound/core.h>
#include <sound/info.h>
#include <sound/memalloc.h>


MODULE_AUTHOR("Takashi Iwai <tiwai@suse.de>, Jaroslav Kysela <perex@suse.cz>");
MODULE_DESCRIPTION("Memory allocator for ALSA system.");
MODULE_LICENSE("GPL");


#ifndef SNDRV_CARDS
#define SNDRV_CARDS	8
#endif
static int enable[SNDRV_CARDS] = {1,1,1,1,1,1,1,1};
MODULE_PARM(enable, "1-" __MODULE_STRING(SNDRV_CARDS) "i");
MODULE_PARM_DESC(enable, "Enable cards to allocate buffers.");


void *snd_malloc_sgbuf_pages(struct device *device,
                             size_t size, struct snd_dma_buffer *dmab,
			     size_t *res_size);
int snd_free_sgbuf_pages(struct snd_dma_buffer *dmab);

/*
 */

static DECLARE_MUTEX(list_mutex);
static LIST_HEAD(mem_list_head);

/* buffer preservation list */
struct snd_mem_list {
    struct snd_dma_buffer buffer;
    unsigned int id;
    struct list_head list;
};

#include <linux/proc_fs.h>
static inline struct proc_dir_entry *create_proc_read_entry(const char *name,
        mode_t mode, struct proc_dir_entry *base,
        read_proc_t *read_proc, void * data)
{
        struct proc_dir_entry *res=create_proc_entry(name,mode,base);
        if (res) {
                res->read_proc=read_proc;
                res->data=data;
        }
        return res;
}

/* id for pre-allocated buffers */
#define SNDRV_DMA_DEVICE_UNUSED (unsigned int)-1

#ifdef CONFIG_PCI
#if defined(__i386__) || defined(__ppc__) || defined(__x86_64__)
#define HACK_PCI_ALLOC_CONSISTENT

/*
 * A hack to allocate large buffers via pci_alloc_consistent()
 *
 * since pci_alloc_consistent always tries GFP_DMA when the requested
 * pci memory region is below 32bit, it happens quite often that even
 * 2 order of pages cannot be allocated.
 *
 * so in the following, we allocate at first without dma_mask, so that
 * allocation will be done without GFP_DMA.  if the area doesn't match
 * with the requested region, then realloate with the original dma_mask
 * again.
 */

static void *snd_pci_hack_alloc_consistent(struct pci_dev *hwdev, size_t size,
				    dma_addr_t *dma_handle)
{
	void *ret;
	u64 dma_mask, cdma_mask;
	unsigned long mask;

	if (hwdev == NULL)
		return pci_alloc_consistent(hwdev, size, dma_handle);
	dma_mask = hwdev->dma_mask;
	cdma_mask = hwdev->consistent_dma_mask;
	mask = (unsigned long)dma_mask && (unsigned long)cdma_mask;
	hwdev->dma_mask = 0xffffffff; /* do without masking */
	hwdev->consistent_dma_mask = 0xffffffff; /* do without masking */
	ret = pci_alloc_consistent(hwdev, size, dma_handle);
	hwdev->dma_mask = dma_mask; /* restore */
	hwdev->consistent_dma_mask = cdma_mask; /* restore */
	if (ret) {
		/* obtained address is out of range? */
		if (((unsigned long)*dma_handle + size - 1) & ~mask) {
			/* reallocate with the proper mask */
			pci_free_consistent(hwdev, size, ret, *dma_handle);
			ret = pci_alloc_consistent(hwdev, size, dma_handle);
		}
	} else {
		/* wish to success now with the proper mask... */
		if (mask != 0xffffffffUL)
			ret = pci_alloc_consistent(hwdev, size, dma_handle);
	}
	return ret;
}

/* redefine pci_alloc_consistent for some architectures */
#undef pci_alloc_consistent
#define pci_alloc_consistent snd_pci_hack_alloc_consistent

#endif /* arch */
/* for 2.2/2.4 kernels */
#define dma_alloc_coherent(dev,size,addr,flags) pci_alloc_consistent((struct pci_dev *)(dev),size,addr)
#define dma_free_coherent(dev,size,ptr,addr) pci_free_consistent((struct pci_dev *)(dev),size,ptr,addr)

#endif /* CONFIG_PCI */

/* allocate the coherent DMA pages */
static void *snd_malloc_dev_pages(struct device *dev, size_t size, dma_addr_t *dma)
{
	int pg;
	void *res;
	unsigned int gfp_flags;

	snd_assert(size > 0, return NULL);
	snd_assert(dma != NULL, return NULL);
	pg = get_order(size);
	gfp_flags = GFP_KERNEL;
	if (pg > 0)
		gfp_flags |= __GFP_NOWARN;
	res = dma_alloc_coherent(dev, PAGE_SIZE << pg, dma, gfp_flags);
	if (res != NULL) {
		inc_snd_pages(pg);
	}

	return res;
}

/* free the coherent DMA pages */
static void snd_free_dev_pages(struct device *dev, size_t size, void *ptr,
			       dma_addr_t dma)
{
	int pg;

	if (ptr == NULL)
		return;
	pg = get_order(size);
	dec_snd_pages(pg);
	dma_free_coherent(dev, PAGE_SIZE << pg, ptr, dma);
}

#if ! defined(__arm__)
#define NEED_RESERVE_PAGES
#endif
/**
 * snd_dma_alloc_pages - allocate the buffer area according to the given type
 * @dev: the buffer device info
 * @size: the buffer size to allocate
 * @dmab: buffer allocation record to store the allocated data
 *
 * Calls the memory-allocator function for the corresponding
 * buffer type.
 *
 * Returns zero if the buffer with the given size is allocated successfuly,
 * other a negative value at error.
 */
int snd_dma_alloc_pages(int type, struct device *device, size_t size,
			struct snd_dma_buffer *dmab)
{
#ifdef DEBUG
    dprintf(("snd_dma_alloc_pages with size = %d",size));
#endif
    snd_assert(size > 0, return -ENXIO);
    snd_assert(dmab != NULL, return -ENXIO);

    dmab->dev.type = type;
    dmab->dev.dev = device;
    dmab->bytes = 0;
    switch (type) {
    case SNDRV_DMA_TYPE_CONTINUOUS:
        dmab->area = snd_malloc_pages(size, (unsigned long)device);
        dmab->addr = 0;
        break;
#ifdef CONFIG_SBUS
    case SNDRV_DMA_TYPE_SBUS:
        dmab->area = snd_malloc_sbus_pages(device, size, &dmab->addr);
		break;
#endif
    case SNDRV_DMA_TYPE_DEV:
        dmab->area = snd_malloc_dev_pages(device, size, &dmab->addr);
        break;
    case SNDRV_DMA_TYPE_DEV_SG:
        snd_malloc_sgbuf_pages(device, size, dmab, NULL);
        break;
    default:
        printk(KERN_ERR "snd-malloc: invalid device type %d\n", type);
        dmab->area = NULL;
        dmab->addr = 0;
        return -ENXIO;
    }
    if (! dmab->area)
        return -ENOMEM;
    dmab->bytes = size;
    return 0;
}

/**
 * snd_dma_alloc_pages_fallback - allocate the buffer area according to the given type with fallback
 * @type: the DMA buffer type
 * @device: the device pointer
 * @size: the buffer size to allocate
 * @dmab: buffer allocation record to store the allocated data
 *
 * Calls the memory-allocator function for the corresponding
 * buffer type.  When no space is left, this function reduces the size and
 * tries to allocate again.  The size actually allocated is stored in
 * res_size argument.
 * 
 * Returns zero if the buffer with the given size is allocated successfuly,
 * other a negative value at error.
 */
int snd_dma_alloc_pages_fallback(int type, struct device *device, size_t size,
				 struct snd_dma_buffer *dmab)
{
	int err;

	snd_assert(size > 0, return -ENXIO);
	snd_assert(dmab != NULL, return -ENXIO);

	while ((err = snd_dma_alloc_pages(type, device, size, dmab)) < 0) {
		if (err != -ENOMEM)
			return err;
		size >>= 1;
		if (size <= PAGE_SIZE)
			return -ENOMEM;
	}
	if (! dmab->area)
		return -ENOMEM;
	return 0;
}

/**
 * snd_dma_free_pages - release the allocated buffer
 * @dmab: the buffer allocation record to release
 *
 * Releases the allocated buffer via snd_dma_alloc_pages().
 */
void snd_dma_free_pages(struct snd_dma_buffer *dmab)
{
	switch (dmab->dev.type) {
	case SNDRV_DMA_TYPE_CONTINUOUS:
		snd_free_pages(dmab->area, dmab->bytes);
		break;
#ifdef CONFIG_SBUS
	case SNDRV_DMA_TYPE_SBUS:
		snd_free_sbus_pages(dmab->dev.dev, dmab->bytes, dmab->area, dmab->addr);
		break;
#endif
	case SNDRV_DMA_TYPE_DEV:
		snd_free_dev_pages(dmab->dev.dev, dmab->bytes, dmab->area, dmab->addr);
		break;
	case SNDRV_DMA_TYPE_DEV_SG:
		snd_free_sgbuf_pages(dmab);
		break;
	default:
		printk(KERN_ERR "snd-malloc: invalid device type %d\n", dmab->dev.type);
	}
}

/**
 * snd_dma_get_reserved - get the reserved buffer for the given device
 * @dmab: the buffer allocation record to store
 * @id: the buffer id
 *
 * Looks for the reserved-buffer list and re-uses if the same buffer
 * is found in the list.  When the buffer is found, it's removed from the free list.
 *
 * Returns the size of buffer if the buffer is found, or zero if not found.
 */
size_t snd_dma_get_reserved_buf(struct snd_dma_buffer *dmab, unsigned int id)
{
	struct list_head *p;
	struct snd_mem_list *mem;

	snd_assert(dmab, return 0);

	down(&list_mutex);
	list_for_each(p, &mem_list_head) {
		mem = list_entry(p, struct snd_mem_list, list);
		if (mem->id == id &&
		    ! memcmp(&mem->buffer.dev, &dmab->dev, sizeof(dmab->dev))) {
			list_del(p);
			*dmab = mem->buffer;
			kfree(mem);
			up(&list_mutex);
			return dmab->bytes;
		}
	}
	up(&list_mutex);
	return 0;
}

/**
 * snd_dma_reserve_buf - reserve the buffer
 * @dmab: the buffer to reserve
 * @id: the buffer id
 *
 * Reserves the given buffer as a reserved buffer.
 * 
 * Returns zero if successful, or a negative code at error.
 */
int snd_dma_reserve_buf(struct snd_dma_buffer *dmab, unsigned int id)
{
	struct snd_mem_list *mem;

	snd_assert(dmab, return -EINVAL);
	mem = kmalloc(sizeof(*mem), GFP_KERNEL);
	if (! mem)
		return -ENOMEM;
	down(&list_mutex);
	mem->buffer = *dmab;
	mem->id = id;
	list_add_tail(&mem->list, &mem_list_head);
	up(&list_mutex);
	return 0;
}

/*
 * purge all reserved buffers
 */
static void free_all_reserved_pages(void)
{
	struct list_head *p;
	struct snd_mem_list *mem;

	down(&list_mutex);
	while (! list_empty(&mem_list_head)) {
		p = mem_list_head.next;
		mem = list_entry(p, struct snd_mem_list, list);
		list_del(p);
		snd_dma_free_pages(&mem->buffer);
		kfree(mem);
	}
	up(&list_mutex);
}


/*
 *
 *  Generic memory allocators
 *
 */

/* Pure 2^n version of get_order */
static inline int get_order(unsigned int size)
{
        int order;

        size = (size-1) >> (PAGE_SHIFT-1);
        order = -1;
        do {
                size >>= 1;
                order++;
        } while (size);
        return order;
}

static long snd_allocated_pages; /* holding the number of allocated pages */

static inline void inc_snd_pages(int order)
{
        snd_allocated_pages += 1 << order;
}

static inline void dec_snd_pages(int order)
{
        snd_allocated_pages -= 1 << order;
}


/**
 * snd_malloc_pages - allocate pages with the given size
 * @size: the size to allocate in bytes
 * @gfp_flags: the allocation conditions, GFP_XXX
 *
 * Allocates the physically contiguous pages with the given size.
 *
 * Returns the pointer of the buffer, or NULL if no enoguh memory.
 */
void *snd_malloc_pages(unsigned long size, unsigned int dma_flags)
{
    int pg;
    void *res;

    snd_assert(size > 0, return NULL);
    snd_assert(dma_flags != 0, return NULL);
    pg = get_order(size);
    if ((res = (void *) __get_free_pages(dma_flags, pg)) != NULL) {
        inc_snd_pages(pg);
    }
    return res;
}

/**
 * snd_free_pages - release the pages
 * @ptr: the buffer pointer to release
 * @size: the allocated buffer size
 *
 * Releases the buffer allocated via snd_malloc_pages().
 */
void snd_free_pages(void *ptr, unsigned long size)
{
    int pg;
    struct page *page, *last_page;

    if (ptr == NULL)
        return;
    pg = get_order(size);
    dec_snd_pages(pg);
    free_pages((unsigned long) ptr, pg);
}

#if defined(CONFIG_ISA) && ! defined(CONFIG_PCI)

/**
 * snd_malloc_isa_pages - allocate pages for ISA bus with the given size
 * @size: the size to allocate in bytes
 * @dma_addr: the pointer to store the physical address of the buffer
 *
 * Allocates the physically contiguous pages with the given size for
 * ISA bus.
 *
 * Returns the pointer of the buffer, or NULL if no enoguh memory.
 */
void *snd_malloc_isa_pages(size_t size, dma_addr_t *dma_addr)
{
	void *dma_area;
	dma_area = snd_malloc_pages(size, GFP_ATOMIC|GFP_DMA);
	*dma_addr = dma_area ? isa_virt_to_bus(dma_area) : 0UL;
	return dma_area;
}

/**
 * snd_malloc_isa_pages_fallback - allocate pages with the given size with fallback for ISA bus
 * @size: the requested size to allocate in bytes
 * @dma_addr: the pointer to store the physical address of the buffer
 * @res_size: the pointer to store the size of buffer actually allocated
 *
 * Allocates the physically contiguous pages with the given request
 * size for PCI bus.  When no space is left, this function reduces the size and
 * tries to allocate again.  The size actually allocated is stored in
 * res_size argument.
 *
 * Returns the pointer of the buffer, or NULL if no enoguh memory.
 */
void *snd_malloc_isa_pages_fallback(size_t size,
				    dma_addr_t *dma_addr,
				    size_t *res_size)
{
	void *dma_area;
	dma_area = snd_malloc_pages_fallback(size, GFP_ATOMIC|GFP_DMA, res_size);
	*dma_addr = dma_area ? isa_virt_to_bus(dma_area) : 0UL;
	return dma_area;
}

#endif /* CONFIG_ISA && !CONFIG_PCI */

#ifdef CONFIG_PCI

/**
 * snd_malloc_pci_pages - allocate pages for PCI bus with the given size
 * @pci: the pci device pointer
 * @size: the size to allocate in bytes
 * @dma_addr: the pointer to store the physical address of the buffer
 *
 * Allocates the physically contiguous pages with the given size for
 * PCI bus.
 *
 * Returns the pointer of the buffer, or NULL if no enoguh memory.
 */
void *snd_malloc_pci_pages(struct pci_dev *pci,
			   size_t size,
			   dma_addr_t *dmaaddr)
{
	int pg;
	void *res;
#ifdef DEBUG
        dprintf(("snd_malloc_pci_pages. size = %d",size));
#endif
	snd_assert(size > 0, return NULL);
	snd_assert(dmaaddr != NULL, return NULL);
	for (pg = 0; PAGE_SIZE * (1 << pg) < size; pg++);
	res = pci_alloc_consistent(pci, PAGE_SIZE * (1 << pg), dmaaddr);
        if (res != NULL) {
            inc_snd_pages(pg);
	}
	return res;
}

/**
 * snd_malloc_pci_pages_fallback - allocate pages with the given size with fallback for PCI bus
 * @pci: pci device pointer
 * @size: the requested size to allocate in bytes
 * @dma_addr: the pointer to store the physical address of the buffer
 * @res_size: the pointer to store the size of buffer actually allocated
 *
 * Allocates the physically contiguous pages with the given request
 * size for PCI bus.  When no space is left, this function reduces the size and
 * tries to allocate again.  The size actually allocated is stored in
 * res_size argument.
 *
 * Returns the pointer of the buffer, or NULL if no enoguh memory.
 */
void *snd_malloc_pci_pages_fallback(struct pci_dev *pci, size_t size,
				    dma_addr_t *dmaaddr,
				    size_t *res_size)
{
	void *res;

	snd_assert(res_size != NULL, return NULL);
	do {
		if ((res = snd_malloc_pci_pages(pci, size, dmaaddr)) != NULL) {
			*res_size = size;
			return res;
		}
		size >>= 1;
	} while (size >= PAGE_SIZE);
	return NULL;
}

/**
 * snd_free_pci_pages - release the pages
 * @pci: pci device pointer
 * @size: the allocated buffer size
 * @ptr: the buffer pointer to release
 * @dma_addr: the physical address of the buffer
 *
 * Releases the buffer allocated via snd_malloc_pci_pages().
 */
void snd_free_pci_pages(struct pci_dev *pci, size_t size, void *ptr, dma_addr_t dmaaddr)
{
	int pg;
	mem_map_t *page, *last_page;

	if (ptr == NULL)
		return;
        for (pg = 0; PAGE_SIZE * (1 << pg) < size; pg++);
        dec_snd_pages(pg);
	pci_free_consistent(pci, PAGE_SIZE * (1 << pg), ptr, dmaaddr);
}

#if defined(__i386__)
/*
 * on ix86, we allocate a page with GFP_KERNEL to assure the
 * allocation.  the code is almost same with kernel/i386/pci-dma.c but
 * it allocates only a single page and checks the validity of the
 * page address with the given pci dma mask.
 */

/**
 * snd_malloc_pci_page - allocate a page in the valid pci dma mask
 * @pci: pci device pointer
 * @addrp: the pointer to store the physical address of the buffer
 *
 * Allocates a single page for the given PCI device and returns
 * the virtual address and stores the physical address on addrp.
 *
 * This function cannot be called from interrupt handlers or
 * within spinlocks.
 */
void *snd_malloc_pci_page(struct pci_dev *pci, dma_addr_t *addrp)
{
	void *ptr;
	dma_addr_t addr;
	unsigned long mask;

	mask = pci ? (unsigned long)pci->consistent_dma_mask : 0x00ffffffUL;
	ptr = (void *)__get_free_page(GFP_KERNEL);
	if (ptr) {
		addr = virt_to_phys(ptr);
		if (((unsigned long)addr + PAGE_SIZE - 1) & ~mask) {
			/* try to reallocate with the GFP_DMA */
			free_page((unsigned long)ptr);
			/* use GFP_ATOMIC for the DMA zone to avoid stall */
			ptr = (void *)__get_free_page(GFP_ATOMIC | GFP_DMA);
			if (ptr) /* ok, the address must be within lower 16MB... */
				addr = virt_to_phys(ptr);
			else
				addr = 0;
		}
	} else
		addr = 0;
	if (ptr) {
		memset(ptr, 0, PAGE_SIZE);
	}
	*addrp = addr;
	return ptr;
}
#else

/* on other architectures, call snd_malloc_pci_pages() helper function
 * which uses pci_alloc_consistent().
 */
void *snd_malloc_pci_page(struct pci_dev *pci, dma_addr_t *addrp)
{
        return snd_malloc_pci_pages(pci, PAGE_SIZE, addrp);
}

#endif

#endif /* CONFIG_PCI */

#ifdef CONFIG_SBUS

/**
 * snd_malloc_sbus_pages - allocate pages for SBUS with the given size
 * @sdev: sbus device pointer
 * @size: the size to allocate in bytes
 * @dma_addr: the pointer to store the physical address of the buffer
 *
 * Allocates the physically contiguous pages with the given size for
 * SBUS.
 *
 * Returns the pointer of the buffer, or NULL if no enoguh memory.
 */
void *snd_malloc_sbus_pages(struct sbus_dev *sdev,
			    size_t size,
			    dma_addr_t *dma_addr)
{
	int pg;
	void *res;

	snd_assert(size > 0, return NULL);
        snd_assert(dma_addr != NULL, return NULL);
        pg = get_order(size);
	res = sbus_alloc_consistent(sdev, PAGE_SIZE * (1 << pg), dma_addr);
        if (res != NULL)
            inc_snd_pages(pg);
	return res;
}

/**
 * snd_malloc_pci_pages_fallback - allocate pages with the given size with fallback for SBUS
 * @sdev: sbus device pointer
 * @size: the requested size to allocate in bytes
 * @dma_addr: the pointer to store the physical address of the buffer
 * @res_size: the pointer to store the size of buffer actually allocated
 *
 * Allocates the physically contiguous pages with the given request
 * size for SBUS.  When no space is left, this function reduces the size and
 * tries to allocate again.  The size actually allocated is stored in
 * res_size argument.
 *
 * Returns the pointer of the buffer, or NULL if no enoguh memory.
 */
/**
 * snd_free_sbus_pages - release the pages
 * @sdev: sbus device pointer
 * @size: the allocated buffer size
 * @ptr: the buffer pointer to release
 * @dma_addr: the physical address of the buffer
 *
 * Releases the buffer allocated via snd_malloc_pci_pages().
 */
void snd_free_sbus_pages(struct sbus_dev *sdev,
			 size_t size,
			 void *ptr,
			 dma_addr_t dma_addr)
{
	int pg;

	if (ptr == NULL)
            return;
        pg = get_order(size);
        dec_snd_pages(pg);
	sbus_free_consistent(sdev, PAGE_SIZE * (1 << pg), ptr, dma_addr);
}

#endif /* CONFIG_SBUS */

/*
 * allocation of buffers for pre-defined devices
 */

#ifdef CONFIG_PCI
/* FIXME: for pci only - other bus? */
struct prealloc_dev {
	unsigned short vendor;
	unsigned short device;
	unsigned long dma_mask;
	unsigned int size;
	unsigned int buffers;
};

#define HAMMERFALL_BUFFER_SIZE    (16*1024*4*(26+1))

static struct prealloc_dev prealloc_devices[] __initdata = {
	{
		/* hammerfall */
		/*.vendor = */0x10ee,
		/*.device = */0x3fc4,
		/*.dma_mask = */0xffffffff,
		/*.size = */HAMMERFALL_BUFFER_SIZE,
		/*.buffers = */2
	},
	{
		/* HDSP */
		/*.vendor = */0x10ee,
		/*.device = */0x3fc5,
		/*.dma_mask = */0xffffffff,
		/*.size = */HAMMERFALL_BUFFER_SIZE,
		/*.buffers = */2
	},
	{0 }, /* terminator */
};

static void __init preallocate_cards(void)
{
	struct pci_dev *pci = NULL;
	int card;

	card = 0;

	while ((pci = pci_find_device(PCI_ANY_ID, PCI_ANY_ID, pci)) != NULL) {
		struct prealloc_dev *dev;
		unsigned int i;
		if (card >= SNDRV_CARDS)
			break;
		for (dev = prealloc_devices; dev->vendor; dev++) {
			if (dev->vendor == pci->vendor && dev->device == pci->device)
				break;
		}
		if (! dev->vendor)
			continue;
		if (! enable[card++]) {
			printk(KERN_DEBUG "snd-page-alloc: skipping card %d, device %04x:%04x\n", card, pci->vendor, pci->device);
			continue;
		}
			
		pci_set_dma_mask(pci, dev->dma_mask);
		for (i = 0; i < dev->buffers; i++) {
			struct snd_dma_buffer dmab;
			memset(&dmab, 0, sizeof(dmab));
			if (snd_dma_alloc_pages(SNDRV_DMA_TYPE_DEV, snd_dma_pci_data(pci),
						dev->size, &dmab) < 0)
				printk(KERN_WARNING "snd-page-alloc: cannot allocate buffer pages (size = %d)\n", dev->size);
			else
				snd_dma_reserve_buf(&dmab, snd_dma_pci_buf_id(pci));
		}
	}
}
#else
#define preallocate_cards()	/* NOP */
#endif

#ifdef CONFIG_PROC_FS
/*
 * proc file interface
 */
static int snd_mem_proc_read(char *page, char **start, off_t off,
			     int count, int *eof, void *data)
{
	int len = 0;
	long pages = snd_allocated_pages >> (PAGE_SHIFT-12);
	struct list_head *p;
	struct snd_mem_list *mem;
	int devno;
	static char *types[] = { "UNKNOWN", "CONT", "DEV", "DEV-SG", "SBUS" };
#if 0
	down(&list_mutex);
	len += snprintf(page + len, count - len,
			"pages  : %li bytes (%li pages per %likB)\n",
			pages * PAGE_SIZE, pages, PAGE_SIZE / 1024);
	devno = 0;
	list_for_each(p, &mem_list_head) {
		mem = list_entry(p, struct snd_mem_list, list);
                devno++;

		len += snprintf(page + len, count - len,
				"buffer %d : ID %08x : type %s\n",
				devno, mem->id, types[mem->buffer.dev.type]);
		len += snprintf(page + len, count - len,
				"  addr = 0x%lx, size = %d bytes\n",
                                (unsigned long)mem->buffer.addr, (int)mem->buffer.bytes);

	}
        up(&list_mutex);
#endif
	return len;
}
#endif /* CONFIG_PROC_FS */

/*
 * module entry
 */

static int __init snd_mem_init(void)
{
#ifdef CONFIG_PROC_FS
	create_proc_read_entry("driver/snd-page-alloc", 0, NULL, snd_mem_proc_read, NULL);
#endif
	preallocate_cards();
	return 0;
}

static void __exit snd_mem_exit(void)
{
	remove_proc_entry("driver/snd-page-alloc", NULL);
	free_all_reserved_pages();
	if (snd_allocated_pages > 0)
		printk(KERN_ERR "snd-malloc: Memory leak?  pages not freed = %li\n", snd_allocated_pages);
}

module_init(snd_mem_init)
module_exit(snd_mem_exit)


#ifndef MODULE

/* format is: snd-page-alloc=enable */

static int __init snd_mem_setup(char *str)
{
	static unsigned __initdata nr_dev = 0;

	if (nr_dev >= SNDRV_CARDS)
		return 0;
	(void)(get_option(&str,&enable[nr_dev]) == 2);
	nr_dev++;
	return 1;
}

__setup("snd-page-alloc=", snd_mem_setup);

#endif

/*
 * exports
 */
EXPORT_SYMBOL(snd_dma_alloc_pages);
EXPORT_SYMBOL(snd_dma_free_pages);
EXPORT_SYMBOL(snd_dma_get_reserved);
EXPORT_SYMBOL(snd_dma_free_reserved);
EXPORT_SYMBOL(snd_dma_set_reserved);

EXPORT_SYMBOL(snd_malloc_pages);
EXPORT_SYMBOL(snd_malloc_pages_fallback);
EXPORT_SYMBOL(snd_free_pages);
#if defined(CONFIG_ISA) && ! defined(CONFIG_PCI)
EXPORT_SYMBOL(snd_malloc_isa_pages);
EXPORT_SYMBOL(snd_malloc_isa_pages_fallback);
#endif
#ifdef CONFIG_PCI
EXPORT_SYMBOL(snd_malloc_pci_pages);
EXPORT_SYMBOL(snd_malloc_pci_pages_fallback);
EXPORT_SYMBOL(snd_malloc_pci_page);
EXPORT_SYMBOL(snd_free_pci_pages);
EXPORT_SYMBOL(snd_malloc_sgbuf_pages);
EXPORT_SYMBOL(snd_free_sgbuf_pages);
#endif
#ifdef CONFIG_SBUS
EXPORT_SYMBOL(snd_malloc_sbus_pages);
EXPORT_SYMBOL(snd_malloc_sbus_pages_fallback);
EXPORT_SYMBOL(snd_free_sbus_pages);
#endif

