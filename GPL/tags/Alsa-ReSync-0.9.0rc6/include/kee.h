/* $Id: kee.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * OS/2 KEE API definitions
 *
 * (C) 1999-2000 Holger Veit
 * (C) 2000-2002 InnoTek Systemberatung GmbH
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

#ifndef __KEE_H__
#define __KEE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* The spinlock data type. KEE spinlock functions either put 0 (unused)
 * or 0xff (used) into the variable, and 0xff000000 when the lock is
 * acquired.
 */
typedef ULONG KEESpinLock;

/*ordinal 10 */
APIRET APIENTRY KernAllocSpinLock(KEESpinLock* sl);

/*ordinal 11*/
/* XXX I suppose this routine should receive a KEESpinLock*, but
 * so far, it just returns NO_ERROR, and not even clearing the Spinlock
 * ownership. I think this is a bug in 14.039F_SMP
 */
APIRET APIENTRY KernFreeSpinLock(/*KEESpinLock* sl*/);

/*ordinal 12*/
VOID APIENTRY KernAcquireSpinLock(KEESpinLock* sl);

/*ordinal 13*/
VOID APIENTRY KernReleaseSpinLock(KEESpinLock* sl);

typedef struct {
  ULONG mtx[2];
} KEEMutexLock;

/*ordinal 20*/
APIRET APIENTRY KernAllocMutexLock(KEEMutexLock* ml);

/*ordinal 21*/
/* XXX Like ordinal 11, I think this should really do something. Actually,
 * it just returns NO_ERROR. At least, it accesses the argument. But
 * it doesn't do anything. Again, likely an error in 14.039F_SMP
 */
APIRET APIENTRY KernFreeMutexLock(KEEMutexLock* ml);

/*ordinal 22*/
VOID APIENTRY KernRequestSharedMutex(KEEMutexLock* ml);

/*ordinal 23*/
VOID APIENTRY KernReleaseSharedMutex(KEEMutexLock* ml);

/*ordinal 24*/
INT APIENTRY KernTryRequestSharedMutex(KEEMutexLock* ml);

/*ordinal 25*/
VOID APIENTRY KernRequestExclusiveMutex(KEEMutexLock* ml);

/*ordinal 26*/
VOID APIENTRY KernReleaseExclusiveMutex(KEEMutexLock* ml);

/*ordinal 27*/
INT APIENTRY KernTryRequestExclusiveMutex(KEEMutexLock* ml);

#define KEE_BLOCK_NOSIGNALS	0x00000001
#define KEE_BLOCK_SPINLOCK      0x00000002
#define KEE_BLOCK_EXMUTEXLOCK   0x00000004
#define KEE_BLOCK_SHMUTEXLOCK   0x00000008
#define KEE_BLOCK_NOACQUIRE     0x00000010

/*ordinal 30*/
APIRET APIENTRY KernBlock(ULONG id, ULONG timeout, ULONG flags,
			  PVOID ptr, PULONG retdata);

#define KEE_WAKE_SINGLE         0x00000001
#define KEE_WAKE_PRIOINCR       0x00000004
#define KEE_WAKE_RETDATA        0x00000008

/*ordinal 31*/
APIRET APIENTRY KernWakeup(ULONG id, ULONG flags, PULONG ptr, ULONG retdata);

/*ordinal 40*/
// Returns: flat base of kernel stack in eax
//NOTE: esp and ebp are modified when this function returns 
//      --> can cause problems if compiler uses i.e. ebp for storing a value!!
ULONG APIENTRY KernThunkStackTo16(VOID);

/*ordinal 41*/
// Returns: flat base of kernel stack in edx
//          16 bits stack selector in eax (not during interrupts (bug??))
//NOTE: esp and ebp are modified when this function returns 
//      --> can cause problems if compiler uses i.e. ebp for storing a value!!

#ifdef	__WATCOMC__
ULONG KernThunkStackTo32(VOID); 
#pragma aux KernThunkStackTo32 "KernThunkStackTo32" modify exact [eax ecx edx] value [edx];
#else
ULONG APIENTRY KernThunkStackTo32(VOID); 
#endif


/*ordinal 42*/
VOID APIENTRY KernSerialize16BitDD(VOID);

/*ordinal 43*/
VOID APIENTRY KernUnserialize16BitDD(VOID);

/*ordinal 44*/
VOID APIENTRY KernArmHook(ULONG hook,ULONG data);

/*ordinal 45*/
APIRET APIENTRY KernAllocateContextHook(PVOID pfHandler,ULONG dummy,
					PULONG pHook);

/*ordinal 50*/
APIRET APIENTRY KernCopyIn(PVOID trgt, PVOID src, ULONG size);

/*ordinal 51*/
APIRET APIENTRY KernCopyOut(PVOID trgt, PVOID src, ULONG size);


/* same bits as with DevHlp_VMAlloc, see explanation there */
#define KEE_VMA_16MB	0x00000001
#define KEE_VMA_FIXED	0x00000002
#define KEE_VMA_SWAP    0x00000004
#define KEE_VMA_CONTIG	0x00000008
#define KEE_VMA_PHYS	0x00000010
#define KEE_VMA_PROCESS	0x00000020
#define KEE_VMA_SGSCONT	0x00000040
#define KEE_VMA_GETSEL	0x00000080
#define KEE_VMA_RESERVE	0x00000100
#define KEE_VMA_SHARED	0x00000400
#define KEE_VMA_USEHIGHMEM 0x00000800

/*ordinal 60*/
APIRET APIENTRY KernVMAlloc(ULONG size, ULONG flags, PVOID* linaddr,
			    PVOID* physaddr, PSHORT sel);

/*ordinal 61*/
APIRET APIENTRY KernVMFree(PVOID linaddr);

/* this is the lockhandle, like with DevHlp_VMLock */
typedef struct {
	UCHAR lock[12];
} KEEVMLock;

/* this is a page list element, like PageList_s in 16 bit Devhlp */
typedef struct {
	ULONG addr;
	ULONG size;
} KEEVMPageList;

/* the same bits as with DevHlp_VMLock */
#define KEE_VML_NOTBLOCK	0x00000001
#define KEE_VML_CONTIG		0x00000002
#define KEE_VML_16M		0x00000004
#define KEE_VML_WRITEABLE	0x00000008
#define KEE_VML_LONGLOCK	0x00000010
#define KEE_VML_VERIFYONLY	0x00000020
#define KEE_VML_unknown		0x80000000

/*ordinal 62*/
APIRET APIENTRY KernVMLock(ULONG flags,PVOID linaddr,ULONG size,
			   KEEVMLock* lock, KEEVMPageList* pglist,
			   PULONG pgsize);

/*ordinal 63*/
APIRET APIENTRY KernVMUnlock(KEEVMLock* lock);

/*ordinal 64*/
APIRET APIENTRY KernLinToPageList(PVOID linaddr,ULONG size,KEEVMPageList* list, ULONG *pgcnt);

#define KEE_VMS_UNCOMMIT	0x00000001
#define KEE_VMS_RESIDENT	0x00000002
#define KEE_VMS_SWAPPABLE	0x00000004

/*ordinal 65*/
APIRET APIENTRY KernVMSetMem(ULONG flags, PVOID linaddr, ULONG size);

/*ordinal 66*/
ULONG KernSelToFlat(ULONG addr16);

/*ordinal 70*/
APIRET APIENTRY KernDynamicAPI(PVOID addr, ULONG cnt, ULONG dummy, PUSHORT sel);

/*ordinal 80*/
APIRET APIENTRY KernRASSysTrace(ULONG major,ULONG minor,PVOID buf, ULONG size);

/*ordinal 81*/
APIRET APIENTRY KernPerfSysTrace(ULONG major,ULONG minor,PVOID buf, ULONG size);

/* this is actually a pointer to the SFT entry for the file */
typedef ULONG KEEhfile;

/*ordinal 100*/
APIRET APIENTRY KernLockFile(HFILE hfd,KEEhfile* khfd);

/*ordinal 101*/
APIRET APIENTRY KernUnLockFile(KEEhfile khfd);

/* this is the file size as returned by ordinal 102 */
typedef QWORD KEEfilesize;

/*ordinal 102*/
APIRET APIENTRY KernGetFileSize(KEEhfile khfd,KEEfilesize* sz);

/*ordinal 103*/
APIRET APIENTRY KernTestFileCache(KEEhfile khfd);

/*ordinal 104*/
APIRET APIENTRY KernReadFileAt(KEEhfile khfd,PVOID buf, QWORD off,
			       ULONG nbytes, PULONG nread);

typedef struct {
	ULONG pagelistsz;
	KEEVMPageList* pagelist;
	ULONG physlistsz;
	KEEVMPageList* physlist;
} KEECachePages;

/*ordinal 105*/
APIRET APIENTRY KernReadFileAtCache(KEEhfile khfd,KEECachePages** ptr,
				    QWORD off, ULONG nbytes, PULONG nread);

/*ordinal 106*/
APIRET APIENTRY KernReturnFileCache(KEEhfile khfd,KEECachePages* ptr);

typedef struct {
	ULONG data[8];
} KEEUnicodeStruct;

/*ordinal 120*/
APIRET APIENTRY KernCreateUconvObject(USHORT codepage, KEEUnicodeStruct* ucs);

/*ordinal 121*/
APIRET APIENTRY KernStrFromUcs(KEEUnicodeStruct* ucs, PCHAR trgt,
			       PCHAR usrc, ULONG trgtsize, ULONG srcsize);
/*ordinal 122*/
APIRET APIENTRY KernStrToUcs(KEEUnicodeStruct* ucs, PCHAR utrgt, PCHAR src,
			     ULONG trgtsize,ULONG srcsize);

#ifdef __cplusplus
}
#endif


#endif
