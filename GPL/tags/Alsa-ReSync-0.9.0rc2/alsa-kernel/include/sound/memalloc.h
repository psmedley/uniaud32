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

#ifndef __SOUND_MEMALLOC_H
#define __SOUND_MEMALLOC_H

#include <linux/pci.h>
#ifdef CONFIG_SBUS
#include <asm/sbus.h>
#endif

#if 0
typedef struct device {
    struct pci_dev *pci;	/* for PCI and PCI-SG types */
    unsigned int flags;	/* GFP_XXX for continous and ISA types */
#ifdef CONFIG_SBUS
    struct sbus_dev *sbus;	/* for SBUS type */
#endif
} device;
#endif
/*
 * buffer device info
 */
struct snd_dma_device {
    int type;                       /* SNDRV_DMA_TYPE_XXX */
    struct device *dev;
};

#ifndef snd_dma_pci_data
#define snd_dma_pci_data(pci)   (&(pci)->dev)
#define snd_dma_isa_data()      NULL
#define snd_dma_sbus_data(sbus) ((struct device *)(sbus))
#define snd_dma_continuous_data(x)      ((struct device *)(unsigned long)(x))
#endif

/*
 * buffer types
 */
#define SNDRV_DMA_TYPE_UNKNOWN		0	/* not defined */
#define SNDRV_DMA_TYPE_CONTINUOUS	1	/* continuous no-DMA memory */
#define SNDRV_DMA_TYPE_DEV		2	/* generic device continuous */
#define SNDRV_DMA_TYPE_DEV_SG		3	/* generic device SG-buffer */
#define SNDRV_DMA_TYPE_SBUS		4	/* SBUS continuous */
#define SNDRV_DMA_TYPE_PCI_SG		5	/* PCI SG-buffer */

#define __GFP_NOWARN    0

/*
 * info for buffer allocation
 */
struct snd_dma_buffer {
    struct snd_dma_device dev;      /* device type */
    unsigned char *area;	/* virtual pointer */
    dma_addr_t addr;	/* physical address */
    size_t bytes;		/* buffer size in bytes */
    void *private_data;	/* private for allocator; don't touch */
};

/* allocate/release a buffer */
int snd_dma_alloc_pages(int type, struct device *dev, size_t size,
                        struct snd_dma_buffer *dmab);
int snd_dma_alloc_pages_fallback(int type, struct device *dev, size_t size,
                                 struct snd_dma_buffer *dmab);
void snd_dma_free_pages(struct snd_dma_buffer *dmab);

/* buffer-preservation managements */
#define snd_dma_pci_buf_id(pci) (((unsigned int)(pci)->vendor << 16) | (pci)->device)
size_t snd_dma_get_reserved_buf(struct snd_dma_buffer *dmab, unsigned int id);
int snd_dma_reserve_buf(struct snd_dma_buffer *dmab, unsigned int id);

/*
 * Generic memory allocators
 */

/*
 * continuous pages
 */
void *snd_malloc_pages(unsigned long size, unsigned int gfp_flags);
void snd_free_pages(void *ptr, unsigned long  size);

#ifdef CONFIG_PCI
/*
 * PCI continuous pages
 */
void *snd_malloc_pci_pages(struct pci_dev *pci, size_t size, dma_addr_t *dma_addr);
void *snd_malloc_pci_pages_fallback(struct pci_dev *pci, size_t size, dma_addr_t *dma_addr, size_t *res_size);
void snd_free_pci_pages(struct pci_dev *pci, size_t size, void *ptr, dma_addr_t dma_addr);
/* one page allocation */
void *snd_malloc_pci_page(struct pci_dev *pci, dma_addr_t *dma_addr);
#define snd_free_pci_page(pci,ptr,addr) snd_free_pci_pages(pci,PAGE_SIZE,ptr,addr)
#endif

#ifdef CONFIG_SBUS
/*
 * SBUS continuous pages
 */
void *snd_malloc_sbus_pages(struct sbus_dev *sdev, size_t size, dma_addr_t *dma_addr);
void *snd_malloc_sbus_pages_fallback(struct sbus_dev *sdev, size_t size, dma_addr_t *dma_addr, size_t *res_size);
void snd_free_sbus_pages(struct sbus_dev *sdev, size_t size, void *ptr, dma_addr_t dma_addr);
#endif

#ifdef CONFIG_ISA
/*
 * ISA continuous pages
 */
void *snd_malloc_isa_pages(size_t size, dma_addr_t *dma_addr);
void *snd_malloc_isa_pages_fallback(size_t size, dma_addr_t *dma_addr, size_t *res_size);
void snd_free_isa_pages(size_t size, void *ptr, dma_addr_t addr);
#ifdef CONFIG_PCI
#define snd_malloc_isa_pages(size, dma_addr) snd_malloc_pci_pages(NULL, size, dma_addr)
#define snd_malloc_isa_pages_fallback(size, dma_addr, res_size) snd_malloc_pci_pages_fallback(NULL, size, dma_addr, res_size)
#define snd_free_isa_pages(size, ptr, dma_addr) snd_free_pci_pages(NULL, size, ptr, dma_addr)
#else /* !CONFIG_PCI */
#define snd_free_isa_pages(size, ptr, dma_addr) snd_free_pages(ptr, size)
#endif /* CONFIG_PCI */
#endif /* CONFIG_ISA */

#ifdef CONFIG_PCI
/*
 * Scatter-Gather PCI pages
 */
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
	struct pci_dev *pci;
};

/*
 * return the pages matching with the given byte size
 */
static inline unsigned int snd_sgbuf_aligned_pages(size_t size)
{
	return (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
}

/*
 * return the physical address at the corresponding offset
 */
static inline dma_addr_t snd_sgbuf_get_addr(struct snd_sg_buf *sgbuf, size_t offset)
{
	return sgbuf->table[offset >> PAGE_SHIFT].addr + offset % PAGE_SIZE;
}
#endif /* CONFIG_PCI */

#endif /* __SOUND_MEMALLOC_H */
