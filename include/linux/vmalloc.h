#ifndef __LINUX_VMALLOC_H
#define __LINUX_VMALLOC_H

//#include <linux/sched.h>
//#include <linux/mm.h>

//#include <asm/pgtable.h>

/* bits in vm_struct->flags */
#define VM_IOREMAP	0x00000001	/* ioremap() and friends */
#define VM_ALLOC	0x00000002	/* vmalloc() */
#define VM_MAP		0x00000004	/* vmap()ed pages */
#define VM_USERMAP	0x00000008	/* suitable for remap_vmalloc_range */
#define VM_VPAGES	0x00000010	/* buffer for pages was vmalloc'ed */
/* bits [20..32] reserved for arch specific ioremap internals */

struct vm_struct {
	unsigned long flags;
	void * addr;
	unsigned long size;
	struct vm_struct * next;
};

struct vm_struct * get_vm_area(unsigned long size);
void *vmalloc(unsigned long size);
void  vfree(void *ptr);
long vread(char *buf, char *addr, unsigned long count);
void vmfree_area_pages(unsigned long address, unsigned long size);
int vmalloc_area_pages(unsigned long address, unsigned long size);

extern struct vm_struct * vmlist;
#endif

