
#ifndef _I386_PAGE_H
#define _I386_PAGE_H

/* PAGE_SHIFT determines the page size */
#define PAGE_SHIFT	12
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

//#include <linux/config.h>

/*
 *	On older X86 processors its not a win to use MMX here it seems.
 *	Maybe the K6-III ?
 */
 
#define clear_page(page)	memset((void *)(page), 0, PAGE_SIZE)
#define copy_page(to,from)	memcpy((void *)(to), (void *)(from), PAGE_SIZE)

/*
 * These are used to make use of C type-checking..
 */
#if CONFIG_X86_PAE
typedef struct { unsigned long long pte; } pte_t;
typedef struct { unsigned long long pmd; } pmd_t;
typedef struct { unsigned long long pgd; } pgd_t;
#else
typedef struct { unsigned long pte; } pte_t;
typedef struct { unsigned long pmd; } pmd_t;
typedef struct { unsigned long pgd; } pgd_t;
#endif

typedef struct { unsigned long long pgprot; } pgprot_t;

#define pte_val(x)	((x).pte)
#define pmd_val(x)	((x).pmd)
#define pgd_val(x)	((x).pgd)
#define pgprot_val(x)	((x).pgprot)

pgprot_t __pgprot(int x);

#endif /* !__ASSEMBLY__ */

/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr)	(((addr)+PAGE_SIZE-1)&PAGE_MASK)

/*
 * This handles the memory map.. We could make this a config
 * option, but too many people screw it up, and too few need
 * it.
 *
 * A __PAGE_OFFSET of 0xC0000000 means that the kernel has
 * a virtual address space of one gigabyte, which limits the
 * amount of physical memory you can use to about 950MB. 
 *
 * If you want more physical memory than this then see the CONFIG_BIGMEM
 * option in the kernel configuration.
 */

#define __PAGE_OFFSET		(0xC0000000)

#define PAGE_OFFSET		((unsigned long)__PAGE_OFFSET)
#define __pa(x)			((unsigned long)(x)-PAGE_OFFSET)
#define __va(x)			((void *)((unsigned long)(x)+PAGE_OFFSET))
#define MAP_NR(addr)		(__pa(addr) >> PAGE_SHIFT)
#define PHYSMAP_NR(addr)	((unsigned long)(addr) >> PAGE_SHIFT)
#ifndef virt_to_page
#define virt_to_page(x) (&mem_map[MAP_NR(x)])
#endif

#endif /* __KERNEL__ */

#endif /* _I386_PAGE_H */
