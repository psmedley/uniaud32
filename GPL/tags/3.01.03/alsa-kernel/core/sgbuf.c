// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Scatter-Gather buffer
 *
 *  Copyright (c) by Takashi Iwai <tiwai@suse.de>
 */

#ifdef TARGET_OS2
#include <sound/core.h>
void snd_free_dev_pages(struct device *dev, size_t size, void *ptr, dma_addr_t dma);
void *snd_malloc_dev_pages(struct device *dev, size_t size, dma_addr_t *dma);
#include <sound/config.h>
#endif
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/export.h>
#include <sound/memalloc.h>
#include "memalloc_local.h"
 
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

#ifdef CONFIG_SND_DMA_SGBUF

/* table entries are align to 32 */
#define SGBUF_TBL_ALIGN		32
#define sgbuf_align_table(tbl)	ALIGN((tbl), SGBUF_TBL_ALIGN)

#ifndef TARGET_OS2
int snd_free_sgbuf_pages(struct snd_dma_buffer *dmab)
{
	struct snd_sg_buf *sgbuf = dmab->private_data;
	struct snd_dma_buffer tmpb;
	int i;

	if (! sgbuf)
		return -EINVAL;

	vunmap(dmab->area);
	dmab->area = NULL;

	tmpb.dev.type = SNDRV_DMA_TYPE_DEV;
	if (dmab->dev.type == SNDRV_DMA_TYPE_DEV_UC_SG)
		tmpb.dev.type = SNDRV_DMA_TYPE_DEV_UC;
	tmpb.dev.dev = sgbuf->dev;
	for (i = 0; i < sgbuf->pages; i++) {
		if (!(sgbuf->table[i].addr & ~PAGE_MASK))
			continue; /* continuous pages */
		tmpb.area = sgbuf->table[i].buf;
		tmpb.addr = sgbuf->table[i].addr & PAGE_MASK;
		tmpb.bytes = (sgbuf->table[i].addr & ~PAGE_MASK) << PAGE_SHIFT;
		snd_dma_free_pages(&tmpb);
	}

	kfree(sgbuf->table);
	kfree(sgbuf->page_table);
	kfree(sgbuf);
	dmab->private_data = NULL;
	
	return 0;
}
#else
/*
 * shrink to the given pages.
 * free the unused pages
 */
static void sgbuf_shrink(struct snd_sg_buf *sgbuf, int pages)
{
        if (! sgbuf->table)
                return;
        while (sgbuf->pages > pages) {
                sgbuf->pages--;
                snd_free_dev_pages(sgbuf->dev, PAGE_SIZE,
                                   sgbuf->table[sgbuf->pages].buf,
                                   sgbuf->table[sgbuf->pages].addr);
        }
}

/* base this fn on the one in Uniaud 1.1.4 */
int snd_free_sgbuf_pages(struct snd_dma_buffer *dmab)
{
	struct snd_sg_buf *sgbuf = dmab->private_data;

        sgbuf_shrink(sgbuf, 0);
	if (sgbuf->table)
		kfree(sgbuf->table);
	sgbuf->table = NULL;
	if (sgbuf->page_table)
		kfree(sgbuf->page_table);
	kfree(sgbuf);
	dmab->private_data = NULL;
	
	return 0;
}
#endif

#define MAX_ALLOC_PAGES		32

void *snd_malloc_sgbuf_pages(struct device *device,
			     size_t size, struct snd_dma_buffer *dmab,
			     size_t *res_size)
{
	struct snd_sg_buf *sgbuf;
	unsigned int pages;
//	unsigned int i, pages, chunk, maxpages;
//	struct snd_dma_buffer tmpb;
#ifdef TARGET_OS2
	void *ptr;
	dma_addr_t addr;
#endif
	struct snd_sg_page *table;
	struct page **pgtable;
	int type = SNDRV_DMA_TYPE_DEV;
#ifndef TARGET_OS2
	pgprot_t prot = PAGE_KERNEL;
#endif
	dprintf(("snd_malloc_sgbuf_pages. size %x", size));

	dmab->area = NULL;
	dmab->addr = 0;
	dmab->private_data = sgbuf = kzalloc(sizeof(*sgbuf), GFP_KERNEL);
	if (! sgbuf)
		return NULL;
	if (dmab->dev.type == SNDRV_DMA_TYPE_DEV_UC_SG) {
		type = SNDRV_DMA_TYPE_DEV_UC;
#ifdef pgprot_noncached
		prot = pgprot_noncached(PAGE_KERNEL);
#endif
	}
	sgbuf->dev = device;
	pages = snd_sgbuf_aligned_pages(size);
	sgbuf->tblsize = sgbuf_align_table(pages);
	table = kcalloc(sgbuf->tblsize, sizeof(*table), GFP_KERNEL);
	if (!table)
		goto _failed;
	sgbuf->table = table;
	pgtable = kcalloc(sgbuf->tblsize, sizeof(*pgtable), GFP_KERNEL);
	if (!pgtable)
		goto _failed;
	sgbuf->page_table = pgtable;

#ifndef TARGET_OS2
	/* allocate pages */
	maxpages = MAX_ALLOC_PAGES;
	while (pages > 0) {
		chunk = pages;
		/* don't be too eager to take a huge chunk */
		if (chunk > maxpages)
			chunk = maxpages;
		chunk <<= PAGE_SHIFT;
		if (snd_dma_alloc_pages_fallback(type, device,
						 chunk, &tmpb) < 0) {
			if (!sgbuf->pages)
				goto _failed;
			if (!res_size)
				goto _failed;
			size = sgbuf->pages * PAGE_SIZE;
			break;
		}
		chunk = tmpb.bytes >> PAGE_SHIFT;
		for (i = 0; i < chunk; i++) {
			table->buf = tmpb.area;
			table->addr = tmpb.addr;
			if (!i)
				table->addr |= chunk; /* mark head */
			table++;
			*pgtable++ = virt_to_page(tmpb.area);
			tmpb.area += PAGE_SIZE;
			tmpb.addr += PAGE_SIZE;
		}
		sgbuf->pages += chunk;
		pages -= chunk;
		if (chunk < maxpages)
			maxpages = chunk;
	}

	sgbuf->size = size;
	dmab->area = vmap(sgbuf->page_table, sgbuf->pages, VM_MAP, prot);
	if (! dmab->area)
		goto _failed;
	if (res_size)
		*res_size = sgbuf->size;
#else
            ptr = snd_malloc_dev_pages(sgbuf->dev, size, &addr);
            if (! ptr)
                goto _failed;

            /* allocate each page */
            while (sgbuf->pages < pages) {
                mem_map_t *page;

                sgbuf->table[sgbuf->pages].buf = (char*)ptr + PAGE_SIZE*sgbuf->pages;
                sgbuf->table[sgbuf->pages].addr = addr + PAGE_SIZE*sgbuf->pages;
                page = (mem_map_t *)virt_to_page((int)sgbuf->table[sgbuf->pages].buf);
                sgbuf->page_table[sgbuf->pages] = page;
                SetPageReserved(page);
                sgbuf->pages++;
            }

            memset(ptr,0,size);

            dmab->area = ptr;
            sgbuf->size = size;
#endif
	return dmab->area;

 _failed:
	snd_free_sgbuf_pages(dmab); /* free the table */
	return NULL;
}
#endif

/*
 * compute the max chunk size with continuous pages on sg-buffer
 */
unsigned int snd_sgbuf_get_chunk_size(struct snd_dma_buffer *dmab,
				      unsigned int ofs, unsigned int size)
{
	struct snd_sg_buf *sg = dmab->private_data;
	unsigned int start, end, pg;

	if (!sg)
		return size;

	start = ofs >> PAGE_SHIFT;
	end = (ofs + size - 1) >> PAGE_SHIFT;
	/* check page continuity */
	pg = sg->table[start].addr >> PAGE_SHIFT;
	for (;;) {
		start++;
		if (start > end)
			break;
		pg++;
		if ((sg->table[start].addr >> PAGE_SHIFT) != pg)
			return (start << PAGE_SHIFT) - ofs;
	}
	/* ok, all on continuous pages */
	return size;
}
EXPORT_SYMBOL(snd_sgbuf_get_chunk_size);
