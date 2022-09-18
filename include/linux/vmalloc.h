#ifndef __LINUX_VMALLOC_H
#define __LINUX_VMALLOC_H

//#include <linux/sched.h>
//#include <linux/mm.h>
#include <linux/overflow.h>

#include <asm/page.h>

#define VM_MAP			0x00000004	/* vmap()ed pages */

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
extern void *vzalloc(unsigned long size);
extern void *__vmalloc(unsigned long size, gfp_t gfp_mask);
void *__vmalloc_node(unsigned long size, unsigned long align, gfp_t gfp_mask,
		int node, const void *caller);
#endif

