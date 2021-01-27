/* $Id: malloc.cpp,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */

/* SCCSID = %W% %E% */
/****************************************************************************
 *                                                                          *
 * Copyright (c) IBM Corporation 1994 - 1997.                               *
 *                                                                          *
 * The following IBM OS/2 source code is provided to you solely for the     *
 * the purpose of assisting you in your development of OS/2 device drivers. *
 * You may use this code in accordance with the IBM License Agreement       *
 * provided in the IBM Device Driver Source Kit for OS/2.                   *
 *                                                                          *
 ****************************************************************************/
/**@internal %W%
 *  Implementation of the driver's built in memory management.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-3 and Ring-0, 16-bit,
 *  kernel stack.
 * @notes
 * @history
 *  01-Jul-95  Timur Tabi   Creation
 */

#include <os2.h>
#include <string.h>                    // memcpy(), memset()
#include <stdio.h>
#include <dbgos2.h>

#include <devhelp.h>
#include <kee.h>
#include "malloc.h"

#if !defined(min)
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#define  SIGNATURE      0xBEEFDEAD      // "DeadBeef" when view in hex in word order.

#pragma pack(4)
typedef struct _MEMBLOCK {
    ULONG                  ulSignature;
    ULONG                  ulSize;
    struct _MEMBLOCK NEAR *pmbNext;
#ifdef DEBUGHEAP
    char             NEAR *pszFile;
    ULONG                  ulLine;
#endif
    char                   achBlock[4];
} MEMBLOCK, NEAR *PMEMBLOCK;
#pragma pack()

#define HDR_SIZE  ( sizeof(MEMBLOCK) - 4 )

// We won't leave a hole in memory any smaller than MIN_FragSize.
#define MIN_FragSize  4

//--- Global Variables used to manage the heap:
//
PMEMBLOCK  pmbUsed  = 0;    // Points to head of list of allocated memory blocks.
                          // Newly allocated blocks are inserted into head of list.
PMEMBLOCK  pmbFree  = 0;    // Points to head of list of available memory blocks.
unsigned   uMemFree = 0;   // N bytes available for allocation.
LINEAR     acHeap   = NULL;

ULONG ulnAlloc = 0;           // Current number of allocated blocks.
ULONG ulcAlloc = 0;           // Total memory in allocated blocks incl. headers.
ULONG ulnAllocCalls = 0;      // Cumulative total, number of malloc() calls.
ULONG ulnFree  = 0;           // Current number of free blocks.
ULONG ulcFree  = 0;           // Total memory in free blocks incl. headers.
ULONG ulnFreeCalls  = 0;      // Cumulative total, number of free() calls.
ULONG ulnCompact = 0;         // Num of times we've joined two adjacent free
                             // blocks into a single larger block.
ULONG ulcTotalHeap = HEAP_SIZE;


#pragma off (unreferenced)

//--- Heap methods:

//*****************************************************************************
//*****************************************************************************
void dumpheap(void)
{
    SHORT sI;
    PMEMBLOCK pmb;
    ULONG ulSize;

	ulSize = 0;
    pmb=pmbUsed;
	dprintf(("HEAP:Heap Dump acHeap=%lx Used=%p Free=%p\n", (void __far *)acHeap, pmbUsed, pmbFree));
    dprintf(("  Used blocks\n"));
    for (sI=0; sI<10; sI++) {
        dprintf(("  pmb=%p, length=%lx\n",(void __far *) pmb, pmb->ulSize));
        ulSize+=pmb->ulSize;
        pmb=pmb->pmbNext;
        if (!pmb) break;
    }
    dprintf(("  Total used = %lx\n", ulSize));

    ulSize=0;
    pmb=pmbFree;
    dprintf(("  Free blocks\n"));
    for (sI=0; sI<50; sI++) {
        dprintf(("  pmb=%p, length=%lx\n",(void __far *) pmb, pmb->ulSize));
        ulSize+=pmb->ulSize;
        pmb=pmb->pmbNext;
        if (!pmb) break;
    }
    dprintf(("  Total free = %lx\n", ulSize));
}
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
SHORT SignatureCheck ( PMEMBLOCK p, PSZ idText )
{
    short sGoodPointer;

    // Set bGoodPointer to indicate whether or not 'p' points within heap.
    sGoodPointer = ((LINEAR)p >= (acHeap))
					&& ((LINEAR)p <= (( acHeap) + HEAP_SIZE)) ;
					//### heap might have less than HEAP_SIZE bytes

    // Check for correct signature.
    if (sGoodPointer) sGoodPointer = (p->ulSignature == SIGNATURE) && p->ulSize < HEAP_SIZE;

    if (!sGoodPointer) {
		dprintf(("SignatureCheck: Bad pointer %p %s\n", p, idText));
        DebugInt3();
    }
    return sGoodPointer;
}
//*****************************************************************************
//*****************************************************************************
void HeapCheck ( PSZ idText )
{
    PMEMBLOCK p;
    for ( ulnAlloc = 0, ulcAlloc = 0, p = pmbUsed; p; p = p->pmbNext ) {
        ++ulnAlloc;
        ulcAlloc += p->ulSize + HDR_SIZE;
        SignatureCheck( p,(PSZ) "HeapCheck() Used list" );
		if (p == p->pmbNext) {
		 	dprintf(("HeapCheck: used %p points to itself\n", p));
			DebugInt3();
		}
    }
    for ( ulnFree = 0, ulcFree = 0, p = pmbFree; p; p = p->pmbNext ) {
        ++ulnFree;
        ulcFree += p->ulSize + HDR_SIZE;
        SignatureCheck( p,(PSZ) "HeapCheck() Free list" );
		if (p == p->pmbNext) {
		 	dprintf(("HeapCheck: free %p points to itself\n", p));
			DebugInt3();
		}
    }
	if (ulcAlloc + ulcFree != ulcTotalHeap) {
		dprintf(("HeapCheck: Alloc+Free != Total\n"));
		//dumpheap();
		DebugInt3();
	}
}

//*****************************************************************************
//*****************************************************************************

// Threshold for creating a new free block.
#define  MIN_Leftover  (HDR_SIZE + MIN_FragSize)

/*
   Formats the back part of an existing free MEMBLOCK as a new, smaller
   "Free" MEMBLOCK.  Doesn't update Global vbls (caller responsibility).
      IN:   pmbOldFree - pointer to existing memory block.
            uSize - offset, which won't be included in new allocation.
                    it's assumed HDR is not included in uSize.
      OUT:  pointer to new free MEMBLOCK, is  a new block of free memory,
            is created at pmbOldFree + uRequest.  The memory block header
            in both the new block and the old block are properly initialized
            on return, but we don't update the Free list or Allocated list.
      OUT:  NULL if the new free memory block is smaller than the
            framentation threshold.

The fragmentation consideration comes into play when we find a block that's
big enough to hold the requested memory, but not big enough for the leftover
part to be useful as another free block.

    _______________________free block_______________________________________
   |  free    |                                                             |
   |  block   |       space available                                       |
   |  header  |          (pmbFree->uSize bytes long)                        |
   |(HDR_SIZE |                                                             |
   |__bytes)__|_____________________________________________________________|

   <-- HDR --> <------------------------- l -------------------------------->

Must either be allocated in total, or must become:

    _______________________used block_______________________________________
   |  used    |                                     |  free    |            |
   |  block   |  space allocated                    |  block   | space      |
   |  header  |  == uSize (following DWORD align't) |  header  | available  |
   |(HDR_SIZE |     (pmbUsed->uSize bytes long)     |(HDR_SIZE |            |
   |__bytes)__|_____________________________________|__bytes)__|____________|

   <-- HDR --> <-------------- n ------------------><-- HDR --> <--- m ----->

To be split into an allocated and a new, smaller free block, 'm' must be at
least MIN_FragSize bytes long.

Everything should remain 4 byte aligned since the HDR_SIZE, MIN_FragSize,
and the space allocated (after we munge it) are all multiples of 4.

This means that in order for a free block to be chosen, one of the following
equation must hold

  Case 1:  n  <=  l  < (n + HDR + MIN_FragSize)
  Here, we'll allocate the entire block.  This makes the block somewhat
  larger than the request, but prevents some degree of fragmentation.

  Case 2:  (n + HDR + MIN_FragSize) <= l
  We split the block into an allocated part to satisfy the allocation request,
  and a free block which can be allocated in a subsequent request.

 @internal pmbAllocateBlock
 *  Update all data structures for allocation of one memory block.  It's
 *  assumed, on entry, that the block is large enough for the requested
 *  allocation.
 * @param PMEMBLOCK pmb - the current memory block on the Free list to
 *  allocate.
 * @param USHORT uRequest - size of the memory request to fill, not counting
 *  memory block header.
 * @param PMEMBLOCK pmbPrev - the memory block which precedes the block being
 *  allocated in the Free list.  NULL if no predecessor (ie, pmb is pointed to
 *  by ::pmbFree).
 * @return PMEMBLOCK - pointer to the data part of the allocated memory block,
 *  suitable for return to malloc() caller.
 */
void NEAR *npvAllocateBlock( PMEMBLOCK pmb, ULONG ulRequest, PMEMBLOCK pmbPrev )
{

    //pmb points to the selected block.
    //pmbPrev points to the block prior to the selected block, NULL if pmbFree.
    PMEMBLOCK pmbOldNext;            // Original free block that follows the block being allocated.
    PMEMBLOCK pmbNewFree;            // Leftover free memory from the allocated block.

    // Split this block into an allocated + free part if it's big enough.
	pmbNewFree = 0;
	if (pmb->ulSize >= (ulRequest + MIN_Leftover)) {
		// We've got enough leftover to make a new free block.

		/* create the new free block */
		pmbNewFree = (PMEMBLOCK) ((char __near *) pmb + HDR_SIZE + ulRequest );
		pmbNewFree->ulSignature = SIGNATURE;
		pmbNewFree->ulSize = pmb->ulSize - (ulRequest + HDR_SIZE);
		pmbNewFree->pmbNext = pmb->pmbNext;

		// Update the size of the original free block that was passed in.
		pmb->ulSize -= (pmbNewFree->ulSize + HDR_SIZE);
	}

    // Update global memory counter.
    uMemFree -= (pmb->ulSize + HDR_SIZE);

    // Add this block into the front of the Allocated list.
    pmbOldNext = pmb->pmbNext;
    pmb->pmbNext = pmbUsed;
    pmbUsed = pmb;

    // Remove the new allocation from the Free list.
    if (pmbNewFree) { // If we split off a new free block
        pmbNewFree->pmbNext = pmbOldNext; // If we're not at head of Free list
        if (pmbPrev) pmbPrev->pmbNext = pmbNewFree;
        else pmbFree = pmbNewFree;
    } else { // We allocated the entire free block.
        if (pmbPrev) pmbPrev->pmbNext = pmbOldNext; // If we're not at head of Free list
        else pmbFree = pmbOldNext;
    }

    return (void NEAR *) pmb->achBlock;
}


/* malloc()
This function searches the list of free blocks until it finds one big enough
to hold the amount of memory requested, which is passed in uSize.  The uSize
request is sized up for:
  - a memory block header
  - four byte alignment
  - minimum fragmentation

See helper function make_new_free() for discussion of fragmentation handling.
*/

#ifdef DEBUGHEAP
void NEAR *malloc(ULONG ulSize, const char *filename, int lineno)
#else
void NEAR *malloc( ULONG ulSize )
#endif
{
    ULONG ulRequest;                    // Request size after alignment.
    PMEMBLOCK pmb, pmbPrev;             // Use to walk free lists.
    void NEAR *npvReturn;         // Return value.
    ULONG ulCpuFlags;

    //dprintf(("malloc(). size: %d", ulSize));
    if(acHeap == NULL) return 0;
    if (!ulSize || ulSize > HEAP_SIZE) {
        DebugInt3();
        return 0;
    }

    ulCpuFlags = DevPushfCli();
	npvReturn = 0;
    ++ulnAllocCalls;                      // Diagnostics.
    HeapCheck((PSZ) "malloc() entry" );

    ulRequest = (ulSize + 3) & -4;        // Force DWORD alignment.

	for (pmbPrev=NULL, pmb=pmbFree; pmb; pmbPrev=pmb, pmb=pmb->pmbNext) {
		if (pmb->ulSize >= ulRequest) {
			npvReturn = npvAllocateBlock(pmb, ulRequest, pmbPrev);
			break;
		}
	}

    if (npvReturn) {
#ifdef DEBUGHEAP
        PMEMBLOCK pBlock = (PMEMBLOCK) (((PUCHAR) npvReturn) - HDR_SIZE);

        pBlock->pszFile = (char *)filename;
        pBlock->ulLine  = lineno;
#endif
        SignatureCheck( (PMEMBLOCK) (((PUCHAR) npvReturn) - HDR_SIZE), (PSZ) "malloc() exit, allocated block" );
    } else {
		dprintf(("malloc(): out of memory"));
        DebugInt3();
    }

    HeapCheck((PSZ) "malloc() exit" );
    DevPopf(ulCpuFlags);
    return npvReturn;
}

/* void compact(void)
This function compacts the free blocks together.  This function is a
companion to free(), and thus the algorithm is tailored to how free()
works.  Change the algorithm in free(), and you'll have to change this
function too.

When free() frees a block, it sets the head of the free list, pmbFree, to
point to it.  Then the newly freed block points to whatever the old pmbFree
pointed to.  In other words, each new free block is added to the head of
the free list.

If compact() is always called after a block is freed, then it can be
guaranteed that the free list is always compacted (i.e. you won't find
two adjacent free blocks anywhere in the heap) _before_ each call to free().
Therefore, the newly freed block can be located in only one of the
following positions:
1. Not adjacent to any other free blocks (compacting not necessary)
2. Physically before another free block
3. Physically after another free block
4. Between two free blocks (i.e. the block occupies the entire space
   between two free blocks)

Since the newly freed block is the first in the free list, compact()
starts with the second block in the list (i.e. pmbFree->pmbNext).
Each free block is then compared with the newly freed block for
adjacency.  If a given block is located before the new block, then it
can't possibly be also located after, and vice versa.  Hence, the
"else if" statement in the middle of the loop.

Also, the newly freed block can only be adjacent to at most two other
blocks.  Therefore, the operation of combining two adjacent free blocks can
only happen at most twice.  The variable ulnFreed counts the number of times
two blocks are combined.  The function exits if ulnFreed reaches two.  ulnFreed
is initially 0.

Helper macro after() takes a PMEMBLOCK (call it pmb) as a parameter,
and calculates where an adjacent free block would exist if it were
physically located after pmb.

Helper function h_remove() removes an element from the free list.
*/

#define after(pmb) ((PMEMBLOCK) ((char *) pmb + pmb->ulSize + HDR_SIZE))

void h_remove(PMEMBLOCK pmb)
{
    PMEMBLOCK pmbPrev;

    if (pmb == pmbFree)
    {
        pmbFree = pmbFree->pmbNext;
        return;
    }

    for (pmbPrev=pmbFree; pmbPrev; pmbPrev=pmbPrev->pmbNext)
    {
        if (pmbPrev->pmbNext == pmb)
        {
            pmbPrev->pmbNext = pmb->pmbNext;
            return;
        }
	}
}

//*****************************************************************************
void compact(void)
{
    PMEMBLOCK pmb;
    SHORT sFreed;

	sFreed = 0;
    for (pmb=pmbFree->pmbNext; pmb; pmb=pmb->pmbNext) {
        if (after(pmb)  == pmbFree) {
			// dprintf(("HEAP: compact found pointer %p (size=%ui) before pmbFree %p\n", (void __far *) pmb, pmb->uSize, (void __far *) pmbFree));
            pmb->ulSize += HDR_SIZE + pmbFree->ulSize;
            h_remove(pmbFree);
            if (++sFreed == 2) break;
        } else if (after(pmbFree) == pmb) {
			// dprintf(("HEAP: compact found pointer %p (size=%ui) after pmbFree %p\n", (void __far *) pmb, pmb->uSize, (void __far *) pmbFree);
            pmbFree->ulSize += HDR_SIZE + pmb->ulSize;
            h_remove(pmb);
            if (++sFreed == 2) break;
        }
    }

    ulnCompact += sFreed;
}
//*****************************************************************************
//*****************************************************************************
#ifdef DEBUGHEAP
void free(void NEAR *pvBlock, const char *filename, int lineno)
#else
void free(void NEAR *pvBlock)
#endif
{
    PMEMBLOCK pmb,pmbPrev,pmbBlock;
    ULONG ulCpuFlags;

    if(acHeap == NULL) {
        DebugInt3();
        return;
    }

    ++ulnFreeCalls;
    if (!pvBlock) return;     // support freeing of NULL

    ulCpuFlags = DevPushfCli();
    pmbBlock=(PMEMBLOCK) ((char NEAR *) pvBlock - HDR_SIZE);

    SignatureCheck( pmbBlock,(PSZ) "free() entry, Block to be freed" );
    HeapCheck((PSZ) "free() entry" );

    uMemFree += pmbBlock->ulSize + HDR_SIZE;

	/* find the block on the used chain */
	for (pmbPrev=NULL, pmb=pmbUsed; pmb; pmbPrev=pmb, pmb=pmb->pmbNext) {
		if (pmb == pmbBlock) { /* found the block */
			if (pmbPrev) { /* it is in the middle of the chain */
				pmbPrev->pmbNext = pmb->pmbNext; /* delete this block from the chain */
			} else { /* it is the first block on the used list */
				pmbUsed = pmbUsed->pmbNext;	// the 2nd block is now the head
			}
			pmbBlock->pmbNext = pmbFree; /* This block is now free, so it points to the first free block */
			pmbFree = pmbBlock; /* This is now the first free block */
			break;
		}
	}
	if (pmb == 0) {
	 	dprintf(("free: block not found %x\n", pmb));
		DebugInt3();
	}
	compact();

    HeapCheck((PSZ) "free() exit" );
    DevPopf(ulCpuFlags);
}
//*****************************************************************************
//*****************************************************************************
unsigned _memfree(void)
{
   return uMemFree;
}
//*****************************************************************************
//*****************************************************************************
/**@internal _msize
 */
unsigned _msize(void NEAR *pvBlock)
{
    PMEMBLOCK pmb;

    if (!pvBlock)
        return 0;

    pmb = (PMEMBLOCK) ((char NEAR *) pvBlock - HDR_SIZE);

    return pmb->ulSize;
}


#ifdef DEBUGHEAP
void NEAR *realloc(void NEAR *pvBlock, ULONG ulLength, const char *filename, int lineno)
#else
void NEAR *realloc(void NEAR *pvBlock, ULONG ulLength)
#endif
{
    void NEAR *pv;

    if (!pvBlock)                 // if ptr is null, alloc block
#ifdef DEBUGHEAP
        return malloc(ulLength, filename, lineno);
#else
        return malloc(ulLength);
#endif

    if (!ulLength) {              // if length is 0, free block
#ifdef DEBUGHEAP
        free(pvBlock, filename, lineno);
#else
        free(pvBlock);
#endif
        return 0;
    }

#ifdef DEBUGHEAP
    pv = malloc(ulLength, filename, lineno);          // attempt to allocate the new block
#else
    pv = malloc(ulLength);          // attempt to allocate the new block
#endif
    if (!pv)                      // can't do it?
        return 0;                  // just fail.  Version 2 will try harder

    memcpy(pv, pvBlock, min( _msize(pvBlock), ulLength));

#ifdef DEBUGHEAP
    free(pvBlock, filename, lineno);
#else
    free(pvBlock);
#endif
    return pv;
}
//*****************************************************************************
//*****************************************************************************
BOOL IsHeapAddr(ULONG addr)
{
    int bGoodPointer;

    bGoodPointer = ((LINEAR)addr >= acHeap)
                    && ((LINEAR)addr <= (acHeap + HEAP_SIZE)) ;

    return bGoodPointer;
}
//*****************************************************************************
extern APIRET VMAlloc(ULONG size, ULONG flags, LINEAR *pAddr) ;
//*****************************************************************************
ULONG HeapInit(ULONG ulSize)
{
    LINEAR addr;
    SHORT sel;

    if (!ulSize) ulSize = DEFAULT_HEAP;

    if (ulSize > HEAP_SIZE) ulSize = HEAP_SIZE;

    if(VMAlloc(ulSize, VMDHA_FIXED, &addr)) {
	    DebugInt3();
	    return 0;
    }
    acHeap = addr;
	//dprintf(("HeapInit(): addr: %x", acHeap));

    pmbFree = (PMEMBLOCK) acHeap;
    pmbFree->ulSize = ulSize - HDR_SIZE;
    pmbFree->ulSignature = SIGNATURE;
    pmbFree->pmbNext = 0;
    uMemFree = pmbFree->ulSize;
    return pmbFree->ulSize;
}
//*****************************************************************************
//*****************************************************************************
