// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *                   Takashi Iwai <tiwai@suse.de>
 * 
 *  Generic memory allocators
 */

#ifdef TARGET_OS2
#include <sound/config.h> //for CONFIG_HAS_DMA
#endif
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/genalloc.h>
#include <linux/highmem.h>
#include <linux/vmalloc.h>
#ifdef CONFIG_X86
#include <asm/set_memory.h>
#endif
#include <sound/memalloc.h>
#include "memalloc_local.h"

static const struct snd_malloc_ops *snd_dma_get_ops(struct snd_dma_buffer *dmab);

/*
 *
 *  Bus-specific memory allocators
 *
 */

#ifdef CONFIG_HAS_DMA
/* allocate the coherent DMA pages */
#ifndef TARGET_OS2
static void snd_malloc_dev_pages(struct snd_dma_buffer *dmab, size_t size)
{
	gfp_t gfp_flags;

	gfp_flags = GFP_KERNEL
		| __GFP_COMP	/* compound page lets parts be mapped */
		| __GFP_NORETRY /* don't trigger OOM-killer */
		| __GFP_NOWARN; /* no stack trace print - this call is non-critical */
	dmab->area = dma_alloc_coherent(dmab->dev.dev, size, &dmab->addr,
					gfp_flags);
#ifdef CONFIG_X86
	if (dmab->area && dmab->dev.type == SNDRV_DMA_TYPE_DEV_WC)
		set_memory_wc((unsigned long)dmab->area,
			      PAGE_ALIGN(size) >> PAGE_SHIFT);
#endif
}
#else
void *snd_malloc_dev_pages(struct device *dev, size_t size, dma_addr_t *dma)
{
	int pg;
	gfp_t gfp_flags;

	if (WARN_ON(!dma))
		return NULL;
	pg = get_order(size);
	gfp_flags = GFP_KERNEL
		| __GFP_COMP	/* compound page lets parts be mapped */
		| __GFP_NORETRY /* don't trigger OOM-killer */
		| __GFP_NOWARN; /* no stack trace print - this call is non-critical */
	return dma_alloc_coherent(dev, PAGE_SIZE << pg, dma, gfp_flags);
}
#endif

/* free the coherent DMA pages */
#ifndef TARGET_OS2
static void snd_free_dev_pages(struct snd_dma_buffer *dmab)
{
#ifdef CONFIG_X86
	if (dmab->dev.type == SNDRV_DMA_TYPE_DEV_WC)
		set_memory_wb((unsigned long)dmab->area,
			      PAGE_ALIGN(dmab->bytes) >> PAGE_SHIFT);
#endif
	dma_free_coherent(dmab->dev.dev, dmab->bytes, dmab->area, dmab->addr);
}
#else
//use version from linux 4.19.x on OS/2
void snd_free_dev_pages(struct device *dev, size_t size, void *ptr,
			       dma_addr_t dma)
{
	int pg;

	if (ptr == NULL)
		return;
	pg = get_order(size);
	dma_free_coherent(dev, PAGE_SIZE << pg, ptr, dma);
}

#endif

#ifdef CONFIG_GENERIC_ALLOCATOR
/**
 * snd_malloc_dev_iram - allocate memory from on-chip internal ram
 * @dmab: buffer allocation record to store the allocated data
 * @size: number of bytes to allocate from the iram
 *
 * This function requires iram phandle provided via of_node
 */
static void snd_malloc_dev_iram(struct snd_dma_buffer *dmab, size_t size)
{
	struct device *dev = dmab->dev.dev;
	struct gen_pool *pool = NULL;

	dmab->area = NULL;
	dmab->addr = 0;

	if (dev->of_node)
		pool = of_gen_pool_get(dev->of_node, "iram", 0);

	if (!pool)
		return;

	/* Assign the pool into private_data field */
	dmab->private_data = pool;

	dmab->area = gen_pool_dma_alloc_align(pool, size, &dmab->addr,
					PAGE_SIZE);
}

/**
 * snd_free_dev_iram - free allocated specific memory from on-chip internal ram
 * @dmab: buffer allocation record to store the allocated data
 */
static void snd_free_dev_iram(struct snd_dma_buffer *dmab)
{
	struct gen_pool *pool = dmab->private_data;

	if (pool && dmab->area)
		gen_pool_free(pool, (unsigned long)dmab->area, dmab->bytes);
}
#endif /* CONFIG_GENERIC_ALLOCATOR */
#endif /* CONFIG_HAS_DMA */

/*
 *
 *  ALSA generic memory management
 *
 */

static inline gfp_t snd_mem_get_gfp_flags(const struct device *dev,
					  gfp_t default_gfp)
{
	if (!dev)
		return default_gfp;
	else
		return (__force gfp_t)(unsigned long)dev;
}

/**
 * snd_dma_alloc_dir_pages - allocate the buffer area according to the given
 *	type and direction
 * @type: the DMA buffer type
 * @device: the device pointer
 * @dir: DMA direction
 * @size: the buffer size to allocate
 * @dmab: buffer allocation record to store the allocated data
 *
 * Calls the memory-allocator function for the corresponding
 * buffer type.
 *
 * Return: Zero if the buffer with the given size is allocated successfully,
 * otherwise a negative value on error.
 */
int snd_dma_alloc_dir_pages(int type, struct device *device,
			    enum dma_data_direction dir, size_t size,
			    struct snd_dma_buffer *dmab)
{
	gfp_t gfp;
    dprintf(("snd_dma_alloc_pages with size = %d",size));
	if (WARN_ON(!size))
		return -ENXIO;
	if (WARN_ON(!dmab))
		return -ENXIO;

	size = PAGE_ALIGN(size);
	dmab->dev.type = type;
	dmab->dev.dev = device;
	dmab->dev.dir = dir;
	dmab->bytes = 0;
	dmab->area = NULL;
	dmab->addr = 0;
	dmab->private_data = NULL;
	switch (type) {
	case SNDRV_DMA_TYPE_CONTINUOUS:
		gfp = snd_mem_get_gfp_flags(device, GFP_KERNEL);
		dmab->area = alloc_pages_exact(size, gfp);
		break;
#ifndef TARGET_OS2 //as of 5.5.19, no drivers we are targetting need this
	case SNDRV_DMA_TYPE_VMALLOC:
		gfp = snd_mem_get_gfp_flags(device, GFP_KERNEL | __GFP_HIGHMEM);
		dmab->area = __vmalloc(size, gfp);
		break;
#endif
#ifdef CONFIG_HAS_DMA
#ifdef CONFIG_GENERIC_ALLOCATOR
	case SNDRV_DMA_TYPE_DEV_IRAM:
		snd_malloc_dev_iram(dmab, size);
		if (dmab->area)
			break;
		/* Internal memory might have limited size and no enough space,
		 * so if we fail to malloc, try to fetch memory traditionally.
		 */
		dmab->dev.type = SNDRV_DMA_TYPE_DEV;
		fallthrough;
#endif /* CONFIG_GENERIC_ALLOCATOR */
	case SNDRV_DMA_TYPE_DEV:
	case SNDRV_DMA_TYPE_DEV_WC:
#ifndef TARGET_OS2
		snd_malloc_dev_pages(dmab, size);
#else
		dmab->area = snd_malloc_dev_pages(device, size, &dmab->addr);
#endif
		break;
#endif
#ifdef CONFIG_SND_DMA_SGBUF
	case SNDRV_DMA_TYPE_DEV_SG:
	case SNDRV_DMA_TYPE_DEV_WC_SG:
		snd_malloc_sgbuf_pages(device, size, dmab, NULL);
		break;
#endif
	default:
		pr_err("snd-malloc: invalid device type %d\n", type);
		return -ENXIO;
	}
	if (! dmab->area)
		return -ENOMEM;
	dmab->bytes = size;
	return 0;
}
EXPORT_SYMBOL(snd_dma_alloc_dir_pages);

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
 * Return: Zero if the buffer with the given size is allocated successfully,
 * otherwise a negative value on error.
 */
int snd_dma_alloc_pages_fallback(int type, struct device *device, size_t size,
				 struct snd_dma_buffer *dmab)
{
	int err;

	while ((err = snd_dma_alloc_pages(type, device, size, dmab)) < 0) {
		if (err != -ENOMEM)
			return err;
		if (size <= PAGE_SIZE)
			return -ENOMEM;
		size >>= 1;
		size = PAGE_SIZE << get_order(size);
	}
	if (! dmab->area)
		return -ENOMEM;
	return 0;
}
EXPORT_SYMBOL(snd_dma_alloc_pages_fallback);


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
		free_pages_exact(dmab->area, dmab->bytes);
		break;
	case SNDRV_DMA_TYPE_VMALLOC:
		vfree(dmab->area);
		break;
#ifdef CONFIG_HAS_DMA
#ifdef CONFIG_GENERIC_ALLOCATOR
	case SNDRV_DMA_TYPE_DEV_IRAM:
		snd_free_dev_iram(dmab);
		break;
#endif /* CONFIG_GENERIC_ALLOCATOR */
	case SNDRV_DMA_TYPE_DEV:
	case SNDRV_DMA_TYPE_DEV_WC:
#ifndef TARGET_OS2
		snd_free_dev_pages(dmab);
#else
		snd_free_dev_pages(dmab->dev.dev, dmab->bytes, dmab->area, dmab->addr);
#endif
		break;
#endif
#ifdef CONFIG_SND_DMA_SGBUF
	case SNDRV_DMA_TYPE_DEV_SG:
	case SNDRV_DMA_TYPE_DEV_WC_SG:
		snd_free_sgbuf_pages(dmab);
		break;
#endif
	default:
		pr_err("snd-malloc: invalid device type %d\n", dmab->dev.type);
	}
}
EXPORT_SYMBOL(snd_dma_free_pages);

#ifdef TARGET_OS2
struct snd_sg_page {
	void *buf;
	dma_addr_t addr;
};

struct snd_sg_buf {
	int size;	/* allocated byte size */
	int pages;	/* allocated pages */
	int tblsize;	/* allocated table size */
	struct snd_sg_page *table;	/* address table */
	struct page **page_table;	/* page table (for vmap/vunmap) */
	struct device *dev;
};

/*
 * return the physical address at the corresponding offset
 */
dma_addr_t snd_sgbuf_get_addr(struct snd_dma_buffer *dmab,
					   size_t offset)
{
	struct snd_sg_buf *sgbuf = dmab->private_data;
	dma_addr_t addr;

	if (!sgbuf)
		return dmab->addr + offset;
	addr = sgbuf->table[offset >> PAGE_SHIFT].addr;
	addr &= ~((dma_addr_t)PAGE_SIZE - 1);
	return addr + offset % PAGE_SIZE;
}
#endif

/* called by devres */
static void __snd_release_pages(struct device *dev, void *res)
{
	snd_dma_free_pages(res);
}

/**
 * snd_devm_alloc_dir_pages - allocate the buffer and manage with devres
 * @dev: the device pointer
 * @type: the DMA buffer type
 * @dir: DMA direction
 * @size: the buffer size to allocate
 *
 * Allocate buffer pages depending on the given type and manage using devres.
 * The pages will be released automatically at the device removal.
 *
 * Unlike snd_dma_alloc_pages(), this function requires the real device pointer,
 * hence it can't work with SNDRV_DMA_TYPE_CONTINUOUS or
 * SNDRV_DMA_TYPE_VMALLOC type.
 *
 * The function returns the snd_dma_buffer object at success, or NULL if failed.
 */
struct snd_dma_buffer *
snd_devm_alloc_dir_pages(struct device *dev, int type,
			 enum dma_data_direction dir, size_t size)
{
	struct snd_dma_buffer *dmab;
	int err;

	if (WARN_ON(type == SNDRV_DMA_TYPE_CONTINUOUS ||
		    type == SNDRV_DMA_TYPE_VMALLOC))
		return NULL;

	dmab = devres_alloc(__snd_release_pages, sizeof(*dmab), GFP_KERNEL);
	if (!dmab)
		return NULL;

	err = snd_dma_alloc_dir_pages(type, dev, dir, size, dmab);
	if (err < 0) {
		devres_free(dmab);
		return NULL;
	}

	devres_add(dev, dmab);
	return dmab;
}
EXPORT_SYMBOL_GPL(snd_devm_alloc_dir_pages);

