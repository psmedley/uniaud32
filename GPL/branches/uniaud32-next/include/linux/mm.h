#ifndef _LINUX_MM_H
#define _LINUX_MM_H

#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <asm/page.h>
#include <asm/atomic.h>
#include <linux/overflow.h>

/*
 * GFP bitmasks..
 */
#define __GFP_WAIT	0x01
#define __GFP_LOW	0x02
#define __GFP_MED	0x04
#define __GFP_HIGH	0x08
#define __GFP_IO	0x10
#define __GFP_SWAP	0x20

#ifdef TARGET_OS2
#define __GFP_DMAHIGHMEM  0x100
#define GFP_DMAHIGHMEM    __GFP_DMAHIGHMEM
#endif

#define GFP_BUFFER	(__GFP_LOW | __GFP_WAIT)
#define GFP_ATOMIC	(__GFP_HIGH)
#define GFP_USER	(__GFP_LOW | __GFP_WAIT | __GFP_IO)
#define GFP_HIGHUSER	(GFP_USER | __GFP_HIGHMEM)
#define GFP_KERNEL	(__GFP_MED | __GFP_WAIT | __GFP_IO)
#define GFP_NFS		(__GFP_HIGH | __GFP_WAIT | __GFP_IO)
#define GFP_KSWAPD	(__GFP_IO | __GFP_SWAP)

/* Flag - indicates that the buffer will be suitable for DMA.  Ignored on some
   platforms, used as appropriate on others */

#define GFP_DMA		__GFP_DMA

/* Flag - indicates that the buffer can be taken from high memory which is not
   directly addressable by the kernel */

#define GFP_HIGHMEM	__GFP_HIGHMEM

/*
 * This struct defines a memory VMM memory area. There is one of these
 * per VM-area/task.  A VM area is any part of the process virtual memory
 * space that has a special rule for the page-fault handlers (ie a shared
 * library, the executable area etc).
 */
struct vm_area_struct {
	struct mm_struct * vm_mm;	/* VM area parameters */
	unsigned long vm_start;
	unsigned long vm_end;

	/* linked list of VM areas per task, sorted by address */
	struct vm_area_struct *vm_next;

	pgprot_t vm_page_prot;
	unsigned short vm_flags;

	/* AVL tree of VM areas per task, sorted by address */
	short vm_avl_height;
	struct vm_area_struct * vm_avl_left;
	struct vm_area_struct * vm_avl_right;

	/* For areas with inode, the list inode->i_mmap, for shm areas,
	 * the list of attaches, otherwise unused.
	 */
	struct vm_area_struct *vm_next_share;
	struct vm_area_struct **vm_pprev_share;

	struct vm_operations_struct * vm_ops;
	unsigned long vm_pgoff;		/* offset in PAGE_SIZE units, *not* PAGE_CACHE_SIZE */
	struct file * vm_file;
	void * vm_private_data;		/* was vm_pte (shared mem) */
};

/*
 * vm_flags..
 */
#define VM_READ		0x0001	/* currently active flags */
#define VM_WRITE	0x0002
#define VM_EXEC		0x0004
#define VM_SHARED	0x0008

#define VM_MAYREAD	0x0010	/* limits for mprotect() etc */
#define VM_MAYWRITE	0x0020
#define VM_MAYEXEC	0x0040
#define VM_MAYSHARE	0x0080

#define VM_GROWSDOWN	0x0100	/* general info on the segment */
#define VM_GROWSUP	0x0200
#define VM_SHM		0x0400	/* shared memory area, don't swap out */
#define VM_DENYWRITE	0x0800	/* ETXTBSY on write attempts.. */

#define VM_EXECUTABLE	0x1000
#define VM_LOCKED	0x2000
#define VM_IO           0x4000  /* Memory mapped I/O or similar */

#define VM_STACK_FLAGS	0x0177

/* Page flag bit values */
#define PG_locked		 0
#define PG_error		 1
#define PG_referenced		 2
#define PG_uptodate		 3
#define PG_decr_after		 5
#define PG_DMA			 7
#define PG_slab			 8
#define PG_swap_cache		 9
#define PG_skip			10
#define PG_swap_entry		11
#define PG_highmem		12
				/* bits 21-30 unused */
#define PG_reserved		31

typedef struct page {
	unsigned long index;
	atomic_t count;
	unsigned long flags;	/* atomic flags, some possibly updated asynchronously */
	unsigned long virtual; /* nonzero if kmapped */
	struct kmem_cache *slab_cache;	/* SL[AU]B: Pointer to slab */
	struct page *first_page;	/* Compound tail pages */
} mem_map_t;

extern mem_map_t * mem_map;

#define free_page(addr) free_pages((addr),0)
extern int free_pages(unsigned long addr, unsigned long order);

#define virt_to_bus virt_to_phys
extern unsigned long virt_to_phys(void * address);

//extern mem_map_t *virt_to_page(int x);
#define virt_to_page(x) (&mem_map[MAP_NR(x)])

extern void * phys_to_virt(unsigned long address);

#define __get_free_page(gfp_mask) __get_free_pages((gfp_mask),0)
#define __get_dma_pages(gfp_mask, order) __get_free_pages((gfp_mask) | GFP_DMA,(order))

extern void *__get_free_pages(int gfp_mask, unsigned long order);
extern struct page * alloc_pages(int gfp_mask, unsigned long order);

extern int remap_page_range(unsigned long from, unsigned long to, unsigned long size, pgprot_t prot);

/*
 * Error return values for the *_nopage functions
 */
#define NOPAGE_SIGBUS	(NULL)
#define NOPAGE_OOM	-1

/*
 * These are the virtual MM functions - opening of an area, closing and
 * unmapping it (needed to keep files on disk up-to-date etc), pointer
 * to the functions called when a no-page or a wp-page exception occurs. 
 */
struct vm_operations_struct {
	void (*open)(struct vm_area_struct * area);
	void (*close)(struct vm_area_struct * area);
	void (*unmap)(struct vm_area_struct *area, unsigned long, size_t);
	void (*protect)(struct vm_area_struct *area, unsigned long, size_t, unsigned int newprot);
	int (*sync)(struct vm_area_struct *area, unsigned long, size_t, unsigned int flags);
	void (*advise)(struct vm_area_struct *area, unsigned long, size_t, unsigned int advise);
	struct page * (*nopage)(struct vm_area_struct * area, unsigned long address, int write_access);
	struct page * (*wppage)(struct vm_area_struct * area, unsigned long address, struct page * page);
	int (*swapout)(struct page *, struct file *);
};

#define SetPageReserved(a)		a
#define ClearPageReserved(a)		a
struct page *vmalloc_to_page(void *addr);

#endif
