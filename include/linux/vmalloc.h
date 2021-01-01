#ifndef __LINUX_VMALLOC_H
#define __LINUX_VMALLOC_H

//#include <linux/sched.h>
//#include <linux/mm.h>

#include <asm/page.h>


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
#endif

