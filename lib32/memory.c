/* $Id: memory.cpp,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
/*
 * OS/2 implementation of Linux memory kernel services
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 * (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
 */

#define INCL_NOPMAPI
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include <devhelp.h>
#include <ossidc.h>
#include <string.h>
#include <dbgos2.h>
#include <stacktoflat.h>
#include <limits.h>
#include <kee.h>
#include "malloc.h"
#define _I386_PAGE_H
typedef struct { unsigned long pgprot; } pgprot_t;
#define MAP_NR(addr)		(__pa(addr) >> PAGE_SHIFT)
#define PAGE_SHIFT	12
#define __PAGE_OFFSET		(0xC0000000)

#define PAGE_OFFSET		((unsigned long)__PAGE_OFFSET)
#define __pa(x)			((unsigned long)(x)-PAGE_OFFSET)

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/printk.h>

#pragma off (unreferenced)

#define PAGE_SIZE 4096
#define min(a,b)  (((a) < (b)) ? (a) : (b))

int free_pages(unsigned long addr, unsigned long order);
int __compat_get_order(unsigned long size);
#ifdef DEBUGHEAP
void near *__kmalloc(int size, int flags, const char *filename, int lineno);
void __kfree(const void near *ptr, const char *filename, int lineno);
#else
void near *__kmalloc(int size, int flags);
void __kfree(const void near *ptr);
#endif

#ifdef DEBUGHEAP
#define _kmalloc(a, b) __kmalloc(a, b, __FILE__, __LINE__)
#define _kfree(a)      __kfree(a, __FILE__, __LINE__)
#else
#define _kmalloc(a, b) __kmalloc(a, b)
#define _kfree(a)      __kfree(a)
#endif

typedef struct _BaseAddr {
    ULONG                  base;	// VMAlloc addr
    ULONG                  retaddr;	// aligned addr returned to caller
    ULONG                  size;	// VMAlloc size
    struct _BaseAddr NEAR *next;
} BaseAddr;

static BaseAddr NEAR *pBaseAddrHead = NULL;

//******************************************************************************
//Very simple linked list for storing original addresses returned by VMAlloc
//if returned address is different due to alignment requirements
//Performance is not an issue as the alloc & free functions aren't called often.
//(e.g. ALS4000 driver calls it 4 times (2 alloc during boot, 2 during shutdown)
//******************************************************************************
void AddBaseAddress(ULONG baseaddr, ULONG retaddr, ULONG size)
{
    BaseAddr NEAR *pBase;

    pBase = (BaseAddr NEAR *)_kmalloc(sizeof(BaseAddr), 0);
    if(pBase == NULL) {
        DebugInt3();
        return;
    }
    DevCli();
    pBase->base    = baseaddr;
    pBase->retaddr = retaddr;
    pBase->size    = size;
    pBase->next    = pBaseAddrHead;
    pBaseAddrHead  = pBase;
    DevSti();
}
//******************************************************************************
//******************************************************************************
ULONG GetBaseAddressAndFree(ULONG addr, ULONG *pSize)
{
    BaseAddr NEAR *pCur, NEAR *pTemp;

    if(pBaseAddrHead == NULL) return addr;

    DevCli();
    pCur = pBaseAddrHead;

    // If address is in list, remove list item and free entry
    // Caller must VMFree returned address or else
    if(pCur->retaddr == addr)
    {
        addr = pCur->base;
        if(pSize) *pSize = pCur->size;
        pBaseAddrHead = pCur->next;
        _kfree(pCur);
    }
    else
    while(pCur->next) {
        if(pCur->next->retaddr == addr) {
            pTemp = pCur->next;
            addr = pTemp->base;
            if(pSize) *pSize = pTemp->size;
            pCur->next = pTemp->next;

            _kfree(pTemp);
            break;
        }
        pCur = pCur->next;
    }
    DevSti();
    return addr;
}
//******************************************************************************
//******************************************************************************
ULONG GetBaseAddressNoFree(ULONG addr, ULONG *pSize)
{
    BaseAddr NEAR *pCur, NEAR *pTemp;

    if(pBaseAddrHead == NULL) return addr;

    DevCli();
    pCur = pBaseAddrHead;

    if(pCur->retaddr == addr)
    {
        addr = pCur->base;
        if(pSize) *pSize = pCur->size;
        pBaseAddrHead = pCur->next;
//        _kfree(pCur);
    }
    else
    while(pCur->next) {
        if(pCur->next->retaddr == addr) {
            pTemp = pCur->next;
            addr = pTemp->base;
            if(pSize) *pSize = pTemp->size;
            pCur->next = pTemp->next;
//            _kfree(pTemp);
            break;
        }
        pCur = pCur->next;
    }
    DevSti();
    return addr;
}
//******************************************************************************
//NOTE: Assumes memory is continuous!!
//******************************************************************************
unsigned long virt_to_phys(void * address)
{
    KEEVMPageList pagelist;
    ULONG         nrpages;

	if(KernLinToPageList(address, PAGE_SIZE, &pagelist, &nrpages)) {
		DebugInt3();
		return 0;
	}
	return pagelist.addr;
}
//******************************************************************************
//******************************************************************************
void * phys_to_virt(unsigned long address)
{
    APIRET rc = 0;
    ULONG addr = 0;

    SHORT sel;
    rc = KernVMAlloc(PAGE_SIZE, VMDHA_PHYS, (PVOID*)&addr, (PVOID*)&address, &sel);
    if (rc != 0) {
        DebugInt3();
        return NULL;
    }
    return (void *)addr;
}
//******************************************************************************
extern int fStrategyInit;
//******************************************************************************
APIRET VMAlloc(ULONG size, ULONG flags, LINEAR *pAddr)
{
    APIRET rc;
    ULONG addr;
    SHORT sel;

    if(fStrategyInit && !(flags & VMDHA_16M)) {
        flags |= VMDHA_USEHIGHMEM;
    }

__again:

    rc = KernVMAlloc(size, flags, (PVOID*)&addr, (PVOID*)-1, &sel);
    if (rc == 0) {
        *pAddr = (LINEAR)addr;
        if (flags & VMDHA_USEHIGHMEM)
            dprintf1((("allocated %X in HIGH memory\n"), size));
        else dprintf1((("allocated %X in LOW memory\n"), size));
    }
    if ((rc == 87) && (flags & VMDHA_USEHIGHMEM))
    {
        // EARLYMEMINIT workaround
        flags = flags & (~VMDHA_USEHIGHMEM);
        goto __again;
    }
    return rc;
}
//******************************************************************************
//******************************************************************************
APIRET VMFree(LINEAR addr)
{
    APIRET rc;
    rc = KernVMFree((PVOID)addr);
    if(rc) {
        DebugInt3();
    }

    return rc;
}
//******************************************************************************
ULONG ulget_free_pagesMemUsed = 0;

//******************************************************************************
//******************************************************************************
void *__get_free_dma_pages(unsigned long size, unsigned long flags)
{
    ULONG  addr, physaddr, physaddr2, diff, tempaddr;
    APIRET rc;

    if(VMAlloc(size, flags, (LINEAR *)&addr)) {
		DebugInt3();
		return 0;
	}
    physaddr = virt_to_phys((void *)addr);
    if(physaddr) {
        ULONG startpage = (physaddr >> 16);
        ULONG endpage   = (physaddr + ((size < 0x10000) ? size : 63*1024)) >> 16;

        if(startpage != endpage) {
            // not in same 32K page, try once more
            rc = VMAlloc(size, flags, (LINEAR *)&tempaddr);
            VMFree((LINEAR)addr);
            if(rc) {
                DebugInt3();
                return 0;
            }
            addr = tempaddr;

            physaddr = virt_to_phys((void *)addr);
            if(physaddr) {
                ULONG startpage = (physaddr >> 16);
                ULONG endpage   = (physaddr + ((size < 0x10000) ? size : 63*1024)) >> 16;

                if(startpage != endpage) {
                    //oops, this didn't work, fail
                    VMFree((LINEAR)addr);
                    dprintf(("get_free_dma_pages failed %x size:%x st:%x end:%x, trying wasteful method instead", physaddr, size, startpage, endpage));
                    return 0;
                }
            }
        }
    }
    else {
        DebugInt3();
        VMFree((LINEAR)addr);
        addr = 0;
    }

    if(addr) {
        //only done to save size of memory block
        AddBaseAddress(addr, addr, size);
        ulget_free_pagesMemUsed += size;
        dprintf(("get_free_dma_pages: size=%x adr=%x (phys %x) total alloc size=%x",
			size, (ULONG)addr, virt_to_phys((void *)addr), ulget_free_pagesMemUsed));
    }

    return (void *)addr;
}
//******************************************************************************
//******************************************************************************
void *__get_free_pages(int gfp_mask, unsigned long order)
{
    ULONG addr;
    ULONG flags = VMDHA_FIXED|VMDHA_CONTIG;
    ULONG size, allocsize;

    order = (1 << order); //TODO: Is this correct???
    size = order * PAGE_SIZE;

    if(gfp_mask & (GFP_DMA|GFP_DMAHIGHMEM))
    {//below 16 mb for legacy DMA?
        if(gfp_mask & GFP_DMA)
            flags |= VMDHA_16M;

        //these buffers must be aligned at 64kb boundary

        //first try a less wasteful approach
        void *pBlock;

        pBlock = __get_free_dma_pages(size, flags);
        if(pBlock) {
            return pBlock;
        }
        //else allocate extra memory to make sure we can satisfy
        //the alignment requirement
        if(size < 0x10000) {
            allocsize = size * 2;
        }
        else {
            allocsize = size + 0x10000;
        }
    }
    else allocsize = size;

    if(VMAlloc(allocsize, flags, (LINEAR *)&addr)) {
		DebugInt3();
		return 0;
	}
	//dprintf(("__get_free_pages %d returned %x", order*PAGE_SIZE, addr));
    if(gfp_mask & (GFP_DMA|GFP_DMAHIGHMEM))
    {//must be aligned at 64kb boundary
        ULONG physaddr = virt_to_phys((void *)addr);
        ULONG physaddr2;

        if(physaddr) {
            ULONG startpage = (physaddr >> 16);
            ULONG endpage   = (physaddr + ((size < 0x10000) ? size : 63*1024)) >> 16;

            if (startpage != endpage) {
		// Not in same 32K page
                physaddr2 = (startpage+1) << 16;

                AddBaseAddress(addr, addr + (physaddr2 - physaddr), allocsize);
                addr += (physaddr2 - physaddr);
            }
        }
        else {
            DebugInt3();
            free_pages(addr, order);
            addr = 0;
        }
    }
    else {
        //only done to save size of memory block
        AddBaseAddress(addr, addr, allocsize);
    }
    if(addr) {
        //dprintf(("get_free_pages %d (%d) -> %x (phys %x)", allocsize, size, (ULONG)addr, virt_to_phys((void *)addr)));
        ulget_free_pagesMemUsed += allocsize;
        //dprintf(("get_free_pages: total alloc size %d", ulget_free_pagesMemUsed));
    }
    return (void *)addr;
}
//******************************************************************************
//******************************************************************************
int free_pages(unsigned long addr, unsigned long order)
{
    ULONG rc, size = 0;

    //check if it really is the base of the allocation (see above)
    addr = GetBaseAddressAndFree(addr, (ULONG NEAR *)&size);

    if(VMFree((LINEAR)addr)) {
        DebugInt3();
    }
    else {
        //dprintf(("free_pages %x size %d", (ULONG)addr, size));
        ulget_free_pagesMemUsed -= size;
        //dprintf(("free_pages: total alloc size %d", ulget_free_pagesMemUsed));
    }
	//dprintf(("free_pages %x", addr));
	return 0;
}
//******************************************************************************
//******************************************************************************
void *vmalloc(unsigned long size)
{
    ULONG addr = 0;
    ULONG flags = VMDHA_FIXED|VMDHA_CONTIG;

    //round to next page boundary
    size = size + PAGE_SIZE - 1;
    size &= 0xFFFFF000;

    if(VMAlloc(size, flags, (LINEAR *)&addr)) {
		DebugInt3();
		return 0;
	}
    if(addr) {
        //dprintf(("vmalloc %d -> %x (phys %x)", size, addr, virt_to_phys((void *)addr)));
        //only done to save size of memory block
        AddBaseAddress(addr, addr, size);
        ulget_free_pagesMemUsed += size;
        //dprintf(("vmalloc: total alloc size %d", ulget_free_pagesMemUsed));
    }
    return (void *)addr;
}
//******************************************************************************
//******************************************************************************
void *__vmalloc(unsigned long size, gfp_t gfp_mask)
{
	return vmalloc(size);
}
//******************************************************************************
//******************************************************************************
/**
 * __vmalloc_node - allocate virtually contiguous memory
 * @size:	    allocation size
 * @align:	    desired alignment
 * @gfp_mask:	    flags for the page level allocator
 * @node:	    node to use for allocation or NUMA_NO_NODE
 * @caller:	    caller's return address
 *
 * Allocate enough pages to cover @size from the page level allocator with
 * @gfp_mask flags.  Map them into contiguous kernel virtual space.
 *
 * Reclaim modifiers in @gfp_mask - __GFP_NORETRY, __GFP_RETRY_MAYFAIL
 * and __GFP_NOFAIL are not supported
 *
 * Any use of gfp flags outside of GFP_KERNEL should be consulted
 * with mm people.
 *
 * Return: pointer to the allocated memory or %NULL on error
 */
void *__vmalloc_node(unsigned long size, unsigned long align,
			    gfp_t gfp_mask, int node, const void *caller)
{
	return vmalloc(size);
}
//******************************************************************************
//******************************************************************************
void vfree(void *ptr)
{
    APIRET rc;
    ULONG  size = 0;

    GetBaseAddressAndFree((ULONG)ptr, (ULONG NEAR *)&size);

    if(VMFree((LINEAR)ptr)) {
        DebugInt3();
    }
    else {
        //dprintf(("vfree %x size %d", (ULONG)ptr, size));
        ulget_free_pagesMemUsed -= size;
        //dprintf(("vfree: total alloc size %d", ulget_free_pagesMemUsed));
    }
}
//******************************************************************************
//******************************************************************************
struct page * alloc_pages(int gfp_mask, unsigned long order)
{
	DebugInt3();
	return 0;
}
//******************************************************************************
//******************************************************************************
int remap_page_range(unsigned long from, unsigned long to, unsigned long size, pgprot_t prot)
{
	DebugInt3();
	return 0;
}
//******************************************************************************
//Map physical address (memory mapped io range) to linear
//******************************************************************************
void * __ioremap(unsigned long physaddr, unsigned long size, unsigned long flags)
{
    ULONG  addr = 0, Offset = 0, PhysicalAddress = 0, Length = 0;
    APIRET rc;

    PhysicalAddress = physaddr;
    Length = size;

    Offset = PhysicalAddress & (PAGE_SIZE - 1); // within Page
    Length = (Length + Offset + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
    PhysicalAddress &= ~(PAGE_SIZE - 1);
    //dprintf(("ioremap: len %d phys %x off %x", Length, PhysicalAddress, Offset));

    //round to next page boundary
	//size = size + PAGE_SIZE - 1;
	//size &= 0xFFFFF000;

    SHORT sel;
    //rc = KernVMAlloc(size, VMDHA_PHYS, (PVOID*)&addr, (PVOID*)&physaddr, &sel);
    rc = KernVMAlloc(Length, VMDHA_PHYS, (PVOID*)&addr, (PVOID*)&PhysicalAddress, &sel);
    if (rc != 0) {
        dprintf(("ioremap error: %x", rc));
        DebugInt3();
        return NULL;
    }
    return (void *)( addr + Offset) ;  //PS
}
//******************************************************************************
//******************************************************************************
void iounmap(void *addr)
{
LINEAR ad;

    ad=(LINEAR)addr;
    // *ad &= ~(0xfff);			// 12 Jun 07 SHL this looks wrong
    ad = (LINEAR)((ULONG)ad & ~0xfff);	// 12 Jun 07 SHL Round down to 4KiB
    if(VMFree((LINEAR)ad)) {
        DebugInt3();
    }
}
//******************************************************************************
//******************************************************************************
int is_access_ok(int type, void *addr, unsigned long size)
{
	return 1;
}
//******************************************************************************
//******************************************************************************
void __copy_user(void *to, const void *from, unsigned long n)
{
	if(to == NULL || from == NULL) {
		DebugInt3();
		return;
	}
    if(n == 0) return;

	memcpy(to, from, n);
}
//******************************************************************************
//******************************************************************************
unsigned long copy_to_user(void *to, const void *from, unsigned long n)
{
	if(to == NULL || from == NULL) {
		DebugInt3();
		return 0;
	}
    if(n == 0) return 0;

	memcpy(to, from, n);
	return 0;
}
//******************************************************************************
//******************************************************************************
void __copy_user_zeroing(void *to, const void *from, unsigned long n)
{
	if(to == NULL || from == NULL) {
		DebugInt3();
		return;
	}
    if(n == 0) return;

	copy_to_user(to, from, n);
}
//******************************************************************************
//******************************************************************************
unsigned long copy_from_user(void *to, const void *from, unsigned long n)
{
	if(to == NULL || from == NULL) {
		DebugInt3();
		return 0;
	}
    if(n == 0) return 0;

	memcpy(to, from, n);
	return 0;
}
//******************************************************************************
//******************************************************************************
int __get_user(int size, void *dest, void *src)
{
	if(size == 0)	return 0;

	if(dest == NULL || src == NULL) {
		DebugInt3();
		return 0;
	}
	memcpy(dest, src, size);
	return 0;
}
//******************************************************************************
//******************************************************************************
int _put_user(int size, int x, void *ptr)
{
	if(ptr == NULL || size == 0) {
		DebugInt3();
		return 0;
	}

	*(int *)ptr = x;
	return 0;
}

//******************************************************************************
#ifdef DEBUGHEAP
void *__kmalloc(int size, int flags, const char *filename, int lineno)
#else
void *__kmalloc(int size, int flags)
#endif
{
    LINEAR addr;

    if(size == 0) {
	    DebugInt3();
	    return NULL;
    }
    if(flags & GFP_DMA) {
        DebugInt3();
    }
    if(size >= 4096) {
        return vmalloc(size);
    }
#ifdef DEBUGHEAP
    addr = (LINEAR)malloc(size, filename, lineno);
#else
    addr = (LINEAR)malloc(size);
#endif
    if(addr == NULL) {
    	DebugInt3();
    	return 0;
    }
    if (flags & __GFP_ZERO)
        memset((LINEAR)addr, 0, size);
	//dprintf(("kmalloc %d returned %x", size, addr));
    return addr;
}

//******************************************************************************
#ifdef DEBUGHEAP
void __kfree(const void *ptr, const char *filename, int lineno)
#else
void __kfree(const void *ptr)
#endif
{
    ULONG addr;

    addr  = (ULONG)ptr;
    if(addr == 0)
    {
    	DebugInt3();
    	return;
    }
	//dprintf(("kfree %x", addr));
    if(IsHeapAddr(addr)) {
#ifdef DEBUGHEAP
        free((void *)addr, filename, lineno);
#else
  	    free((void *)addr);
#endif
    }
    else  vfree((PVOID)addr);
}

//******************************************************************************
//******************************************************************************
void *kcalloc(size_t n, size_t size, unsigned int flags)
{
	if (n != 0 && size > INT_MAX / n)
		return NULL;
	return kzalloc(n * size, flags);
}
//******************************************************************************
//******************************************************************************

size_t ksize(const void *block)
{
	size_t size;

	if (!block)
	    size = 0;			// Bad coder

	else if (block == ZERO_SIZE_PTR)
	    size = 0;			// Bad coder

	else if(IsHeapAddr((ULONG)block))
	    size = _msize((void _near *)block);

	else if (!GetBaseAddressNoFree((ULONG)block, (ULONG NEAR *)&size))
	    size = 0;			// Something wrong

	return size;
}
//******************************************************************************
//******************************************************************************
static inline void *__do_krealloc(const void *p, size_t new_size,
					   gfp_t flags)
{
	void *ret;
	size_t ks = 0;

	if (p)
		ks = ksize(p);

	if (ks >= new_size)
		return (void *)p;

	ret = __kmalloc(new_size, flags);
	if (ret && p)
		memcpy(ret, p, ks);

	return ret;
}
//******************************************************************************
//******************************************************************************
/**
 * krealloc - reallocate memory. The contents will remain unchanged.
 * @p: object to reallocate memory for.
 * @new_size: how many bytes of memory are required.
 * @flags: the type of memory to allocate.
 *
 * The contents of the object pointed to are preserved up to the
 * lesser of the new and old sizes.  If @p is %NULL, krealloc()
 * behaves exactly like kmalloc().  If @new_size is 0 and @p is not a
 * %NULL pointer, the object pointed to is freed.
 */
void *krealloc(const void *p, size_t new_size, gfp_t flags)
{
	void *ret;

	if (!new_size) {
		kfree(p);
		return ZERO_SIZE_PTR;
	}

	ret = __do_krealloc(p, new_size, flags);
	if (ret && p != ret)
		kfree(p);

	return ret;
}
//******************************************************************************
//******************************************************************************
/**
 *	vzalloc - allocate virtually contiguous memory with zero fill
 *	@size:	allocation size
 *	Allocate enough pages to cover @size from the page level
 *	allocator and map them into contiguous kernel virtual space.
 *	The memory allocated is set to zero.
 *
 *	For tight control over page level allocator and protection flags
 *	use __vmalloc() instead.
 */
void *vzalloc(unsigned long size)
{
	void *buf;
	buf = vmalloc(size);
	if (buf)
		memset(buf, 0, size);
	return buf;
}

//******************************************************************************
//******************************************************************************
/**
 * kvmalloc_node - attempt to allocate physically contiguous memory, but upon
 * failure, fall back to non-contiguous (vmalloc) allocation.
 * @size: size of the request.
 * @flags: gfp mask for the allocation - must be compatible (superset) with GFP_KERNEL.
 * @node: numa node to allocate from
 *
 * Uses kmalloc to get the memory but if the allocation fails then falls back
 * to the vmalloc allocator. Use kvfree for freeing the memory.
 *
 * Reclaim modifiers - __GFP_NORETRY and __GFP_NOFAIL are not supported.
 * __GFP_RETRY_MAYFAIL is supported, and it should be used only if kmalloc is
 * preferable to the vmalloc fallback, due to visible performance drawbacks.
 *
 * Please note that any use of gfp flags outside of GFP_KERNEL is careful to not
 * fall back to vmalloc.
 *
 * Return: pointer to the allocated memory of %NULL in case of failure
 */
void *kvmalloc_node(size_t size, gfp_t flags, int node)
{
	gfp_t kmalloc_flags = flags;
	void *ret;

	/*
	 * vmalloc uses GFP_KERNEL for some internal allocations (e.g page tables)
	 * so the given set of flags has to be compatible.
	 */
	if ((flags & GFP_KERNEL) != GFP_KERNEL)
		return kmalloc_node(size, flags, node);

	/*
	 * We want to attempt a large physically contiguous block first because
	 * it is less likely to fragment multiple larger blocks and therefore
	 * contribute to a long term fragmentation less than vmalloc fallback.
	 * However make sure that larger requests are not too disruptive - no
	 * OOM killer and no allocation failure warnings as we have a fallback.
	 */
	if (size > PAGE_SIZE) {
		kmalloc_flags |= __GFP_NOWARN;

		if (!(kmalloc_flags & __GFP_RETRY_MAYFAIL))
			kmalloc_flags |= __GFP_NORETRY;
	}

	ret = kmalloc_node(size, kmalloc_flags, node);

	/*
	 * It doesn't really make sense to fallback to vmalloc for sub page
	 * requests
	 */
	if (ret || size <= PAGE_SIZE)
		return ret;

	return __vmalloc_node(size, 1, flags, node,
			__builtin_return_address(0));
}
//******************************************************************************
//******************************************************************************