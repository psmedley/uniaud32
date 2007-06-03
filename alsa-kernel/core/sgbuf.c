/*
 * Scatter-Gather buffer
 *
 *  Copyright (c) by Takashi Iwai <tiwai@suse.de>
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

#ifdef TARGET_OS2
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/info.h>
#endif /* TARGET_OS2 */
#include <sound/memalloc.h>

/* table entries are align to 32 */
#define SGBUF_TBL_ALIGN		32
#define sgbuf_align_table(tbl)	((((tbl) + SGBUF_TBL_ALIGN - 1) / SGBUF_TBL_ALIGN) * SGBUF_TBL_ALIGN)

/* set up the page table from the given vmalloc'ed buffer pointer.
 * return a negative error if the page is out of the pci address mask.
 */
static int store_page_tables(struct snd_sg_buf *sgbuf, void *vmaddr, unsigned int pages)
{
	unsigned int i;

	sgbuf->pages = 0;
        DebugInt3();
	for (i = 0; i < pages; i++) {
		struct page *page;
		void *ptr;
		dma_addr_t addr;
                ptr = (void*)virt_to_phys((void*)((char*)vmaddr + (i << PAGE_SHIFT)));
//		ptr = get_vmalloc_addr(vmaddr + (i << PAGE_SHIFT));
		addr = virt_to_bus(ptr);
		page = virt_to_page((int)ptr);
#ifdef DEBUG
                dprintf(("virt mem: %x, phys: %x, addr: %x, page: %x",(char*)vmaddr, (char*)ptr, addr, page));
#endif
		sgbuf->table[i].buf = ptr;
		sgbuf->table[i].addr = addr;
		sgbuf->page_table[i] = page;
		SetPageReserved(page);
		sgbuf->pages++;
	}
	return 0;
}

/*
 * shrink to the given pages.
 * free the unused pages
 */
static void sgbuf_shrink(struct snd_sg_buf *sgbuf, int pages)
{
        snd_assert(sgbuf, return);
        if (! sgbuf->table)
                return;
        while (sgbuf->pages > pages) {
                sgbuf->pages--;
                snd_free_pci_pages(sgbuf->pci, PAGE_SIZE,
                                   sgbuf->table[sgbuf->pages].buf,
                                   sgbuf->table[sgbuf->pages].addr);
        }
}

/**
 * snd_malloc_sgbuf_pages - allocate the pages for the PCI SG buffer
 * @pci: the pci device pointer
 * @size: the requested buffer size in bytes
 * @dmab: the buffer record to store
 *
 * Initializes the SG-buffer table and allocates the buffer pages
 * for the given size.
 * The pages are mapped to the virtually continuous memory.
 *
 * This function is usually called from the middle-level functions such as
 * snd_pcm_lib_malloc_pages().
 *
 * Returns the mapped virtual address of the buffer if allocation was
 * successful, or NULL at error.
 */
void *snd_malloc_sgbuf_pages(struct pci_dev *pci,
                             size_t size,
                             struct snd_dma_buffer *dmab,
                             size_t *res_size)
{
	struct snd_sg_buf *sgbuf;
        unsigned int pages;
        void *ptr;
        dma_addr_t addr;

#ifdef DEBUG
        dprintf(("snd_malloc_sgbuf_pages. size %x",size));
#endif

	dmab->area = NULL;
	dmab->addr = 0;
	dmab->private_data = sgbuf = kmalloc(sizeof(*sgbuf), GFP_KERNEL);
        if (! sgbuf)
        {
#ifdef DEBUG
            dprintf(("snd_malloc_sgbuf_pages failed: error allocating sgbuf"));
#endif
            return NULL;
        }
	memset(sgbuf, 0, sizeof(*sgbuf));
	sgbuf->pci = pci;
	pages = snd_sgbuf_aligned_pages(size);
	sgbuf->tblsize = sgbuf_align_table(pages);
	sgbuf->table = kmalloc(sizeof(*sgbuf->table) * sgbuf->tblsize, GFP_KERNEL);
        if (! sgbuf->table)
        {
#ifdef DEBUG
            dprintf(("snd_malloc_sgbuf_pages failed: error allocating sgbuf->table"));
#endif
            return NULL;
        }
	memset(sgbuf->table, 0, sizeof(*sgbuf->table) * sgbuf->tblsize);
	sgbuf->page_table = kmalloc(sizeof(*sgbuf->page_table) * sgbuf->tblsize, GFP_KERNEL);
	if (! sgbuf->page_table)
        {
#ifdef DEBUG
            dprintf(("snd_malloc_sgbuf_pages failed: error allocating sgbuf->page_table"));
#endif
            return NULL;
        }
        memset(sgbuf->page_table, 0, sizeof(*sgbuf->page_table) * sgbuf->tblsize);

#ifdef DEBUG
            dprintf(("allocating %d pages",pages));
#endif
            ptr = snd_malloc_pci_pages(sgbuf->pci, size, &addr);
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
            return dmab->area;

        _failed:
#ifdef DEBUG
            dprintf(("snd_malloc_sgbuf_pages failed"));
#endif

	snd_free_sgbuf_pages(dmab); /* free the table */
	return NULL;
}


/**
 * snd_free_sgbuf_pages - free the sg buffer
 * @dmab: buffer record
 *
 * Releases the pages and the SG-buffer table.
 *
 * This function is called usually from the middle-level function
 * such as snd_pcm_lib_free_pages().
 *
 * Returns zero if successful, or a negative error code on failure.
 */
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


