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

extern "C" {
#define INCL_NOPMAPI
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#include <os2.h>
}
#include <devhelp.h>
#include <ossidc.h>
#include <string.h>
#include <dbgos2.h>
#include <stacktoflat.h>
#include <limits.h>
#ifdef KEE
#include <kee.h>
#endif
#include "malloc.h"

#pragma off (unreferenced)

#define PAGE_SIZE 4096

extern "C" {

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
    ULONG                  base;
    ULONG                  retaddr;
    ULONG                  size;
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
ULONG GetBaseAddress(ULONG addr, ULONG *pSize)
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
//NOTE: Assumes memory is continuous!!
//******************************************************************************
unsigned long virt_to_phys(void * address)
{
#ifdef KEE
    KEEVMPageList pagelist;
    ULONG         nrpages;

	if(KernLinToPageList(address, PAGE_SIZE, &pagelist, &nrpages)) {
		DebugInt3();
		return 0;
	}
	return pagelist.addr;
#else
    LINEAR addr = (LINEAR)address;
    PAGELIST pagelist;

	if(DevLinToPageList(addr, PAGE_SIZE, (PAGELIST NEAR *)__Stack32ToFlat((ULONG)&pagelist))) {
		DebugInt3();
		return 0;
	}
	return pagelist.physaddr;
#endif
}
//******************************************************************************
//******************************************************************************
void * phys_to_virt(unsigned long address)
{
    APIRET rc = 0;
    ULONG addr = 0;

#ifdef KEE
    SHORT sel;
    rc = KernVMAlloc(PAGE_SIZE, VMDHA_PHYS, (PVOID*)&addr, (PVOID*)&address, &sel);
#else
    rc = DevVMAlloc(VMDHA_PHYS, PAGE_SIZE, (LINEAR)&address, __Stack32ToFlat((ULONG)&addr));
#endif
    if (rc != 0) {
        DebugInt3();
        return NULL;
    }
    return (void *)addr;
}
//******************************************************************************
extern "C" int fStrategyInit;
//******************************************************************************
APIRET VMAlloc(ULONG size, ULONG flags, LINEAR *pAddr)
{
    APIRET rc;
    ULONG addr;

    if(fStrategyInit && !(flags & VMDHA_16M)) {
        flags |= VMDHA_USEHIGHMEM;
    }

__again:

#ifdef KEE
    SHORT sel;

    rc = KernVMAlloc(size, flags, (PVOID*)&addr, (PVOID*)-1, &sel);
#else
    rc = DevVMAlloc(flags, size, (LINEAR)-1, __Stack32ToFlat((ULONG)&addr));
#endif
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

#ifdef KEE
	rc = KernVMFree((PVOID)addr);
#else
	rc = DevVMFree((LINEAR)addr);
#endif
    if(rc) {
        DebugInt3();
    }

    return rc;
}
//******************************************************************************
ULONG ulget_free_pagesMemUsed = 0;

#define GFP_DMA	        0x80
#define GFP_DMAHIGHMEM	0x100
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
            //try once more
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
                    dprintf(("get_free_dma_pages failed %x size:%x st:%x end:%x, trying wasteful method instead",physaddr,size,startpage,endpage));
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
        dprintf(("get_free_dma_pages %d -> %x (phys %x)", size, (ULONG)addr, virt_to_phys((void *)addr)));
        ulget_free_pagesMemUsed += size;
        dprintf(("get_free_dma_pages: total alloc size %d", ulget_free_pagesMemUsed));
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
        dprintf(("get_free_pages %d (%d) -> %x (phys %x)", allocsize, size, (ULONG)addr, virt_to_phys((void *)addr)));
        ulget_free_pagesMemUsed += allocsize;
        dprintf(("get_free_pages: total alloc size %d", ulget_free_pagesMemUsed));
    }
    return (void *)addr;
}
//******************************************************************************
//******************************************************************************
int free_pages(unsigned long addr, unsigned long order)
{
    ULONG rc, size = 0;

    //check if it really is the base of the allocation (see above)
    addr = GetBaseAddress(addr, (ULONG NEAR *)__Stack32ToFlat(&size));

    if(VMFree((LINEAR)addr)) {
        DebugInt3();
    }
    else {
        dprintf(("free_pages %x size %d", (ULONG)addr, size));
        ulget_free_pagesMemUsed -= size;
        dprintf(("free_pages: total alloc size %d", ulget_free_pagesMemUsed));
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
void vfree(void *ptr)
{
    APIRET rc;
    ULONG  size = 0;

    GetBaseAddress((ULONG)ptr, (ULONG NEAR *)__Stack32ToFlat(&size));

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
int remap_page_range(unsigned long from, unsigned long to, unsigned long size, unsigned long prot)
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

#ifdef KEE
    SHORT sel;

	//rc = KernVMAlloc(size, VMDHA_PHYS, (PVOID*)&addr, (PVOID*)&physaddr, &sel);
    rc = KernVMAlloc(Length, VMDHA_PHYS, (PVOID*)&addr, (PVOID*)&PhysicalAddress, &sel);
#else
    //rc = DevVMAlloc(VMDHA_PHYS, size, (LINEAR)&physaddr, __Stack32ToFlat((ULONG)&addr));
    rc = DevVMAlloc(VMDHA_PHYS, Length, (LINEAR)&PhysicalAddress, __Stack32ToFlat((ULONG)&addr));
#endif
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

	kmemcpy(to, from, n);
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

	kmemcpy(to, from, n);
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

	kmemcpy(to, from, n);
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
	kmemcpy(dest, src, size);
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
	//dprintf(("kmalloc %d returned %x", size, addr));
    return addr;
}
//******************************************************************************
//******************************************************************************
#ifdef DEBUGHEAP
void __kfree(const void *ptr, const char *filename, int lineno)
#else
void __kfree(const void *ptr)
#endif
{
    ULONG addr;

    addr  = (ULONG)ptr;
    if(addr == NULL) {
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
void *kzalloc(size_t size, unsigned int flags)
{
	void *ret;
	ret = _kmalloc(size, flags);
	if (ret)
		memset(ret, 0, size);
	return ret;
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

}