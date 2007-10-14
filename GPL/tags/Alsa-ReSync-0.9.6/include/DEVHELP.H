/* $Id: DEVHELP.H,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * OS/2 device driver APIs (DevHelp)
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



// DevHelp.h
//
// Watcom C++ callable entry points to the OS/2 DevHlp interface.
//
// All of the DevHlp functions in this library are prefaced with the string
// "Dev" rather than "DevHlp_" which is used by the IBM DDK libraries.  The
// names were made incompatible so that calls to the IBM DevHlp routines
// are not confused with calls to these routines (which may have different
// calling signatures).  If you need the IBM calling signatures, just use
// some inline functions which call the Dev* functions.
//
// Also, the ordering of words in multi-word names has been changed so that
// the object of an operation is the first word, and the operation name is
// the second word.  For example, "DevHlp_AllocPhys" has been renamed to
// "DevPhysAlloc".
//
// This DevHlp library does not include certain categories of DevHlps that
// are available in the IBM DDK.  In particular, ABIOS specific functions
// (function codes 29h and 34h to 37h), monitors (function codes 31h to
// 35h), and some virtual memory functions (function codes 55h to 5Ah and
// 66h) are not (yet) supported.  Also, functions which are no longer
// meaningful under OS/2 2.1, such as UnPhysToVirt and SetRomVector are not
// included either.  However, some undocumented or only partially
// documented DevHlps such as the RAS tracing facility (DevSysTrace,
// function code 28h) are included in this library.
//
// History:
//
// Sep 30, 94  David Bollo    Initial version

// Prevent multiple inclusion
#if !defined(DevHelp_h)
#define DevHelp_h 1

#if !defined(__WATCOMC__) || !defined(__cplusplus)
//#error Watcom C++ must be used for the inline DevHlp interface.
#endif


#include "devtype.h"
#include <stacktoflat.h>


// Calling conventions for device driver functions
//
//

// Strategy call calling convention
#pragma aux STRATEGY parm [fs ebx];

#pragma aux VDD __far loadds parm [ax] modify [bx cx dx si di];

// Dynamic API calling convention
#pragma aux DYNAMIC __far loadds parm [si] value [ax];

// Interface to the OS/2 Ring 0 Device Helper functions
//
//

extern void DevHelp();
#pragma aux DevHelp "DevHlp"

extern void DevHelp_ModifiedDS();
#pragma aux DevHelp_ModifiedDS "DevHlp_ModifiedDS"

VOID DevCli                   // Disable interrupts
  (
  );                          // Returns: Nothing

#pragma aux DevCli =          \
  "cli";

VOID DevSti                   // Enable interrupts
  (
  );                          // Returns: Nothing

#pragma aux DevSti =          \
  "sti";

DWORD DevPushfCli();
#pragma aux DevPushfCli =   	\
  "pushfd"                     	\
  "cli"				\
  "pop eax"			\
  value [eax];

VOID DevPopf(DWORD cpuflags);
#pragma aux DevPopf =         	\
  "push eax"			\
  "popfd"			\
   parm [eax];

WORD16 DevBlock               // Block the current thread
  (
  WORD32 Id,                  // Id to be used by DevRun call
  WORD32 WaitMilliSec,        // Milliseconds to wait for (-1 = forever)
  BYTE InterruptableFlag      // 0 if block is interruptable, else 1
  );                          // Returns: 0 = Ran, 1 = Timeout, 2 = Ctrl+C

#pragma aux DevBlock =        \
  "mov bx, ax"                \
  "shr eax,16"                \
  "mov cx, di"                \
  "shr edi,16"                \
  "mov dl,04h"                \
  "call DevHelp"              \
  "setnz al"                  \
  "setc bl"                   \
  "inc al"                    \
  "neg bl"                    \
  "and al,bl"                 \
  "xor ah,ah"                 \
  modify [edx ebx ecx]        \
  parm [eax] [edi] [dh]       \
  value [ax];



WORD16 DevRun                 // Run a blocked thread
  (
  WORD32 Id                   // Id used in DevBlock call
  );                          // Returns: Count of awakened threads

#pragma aux DevRun =          \
  "mov bx, ax"                \
  "shr eax, 16"               \
  "mov dl,05h"                \
  "call DevHelp"              \
  modify [edx ebx]            \
  parm [eax]                  \
  value [ax];

WORD16 DevSegLock             // Lock a segment down in memory
  (
  SEL Selector,               // Selector of memory to lock down
  WORD16 Type,                // Type (0 = short, 1 = long, 3 = long & hi,
                              //       4 = short & verify)
  WORD16 NoWaitFlag,          // 0 to wait for lock or 1 to return immediately
  HLOCK *Handle               // Storage for segment lock handle
  );                          // Returns: 0 = success, 1 = error

#pragma aux DevSegLock =      \
  "push edx"                  \
  "mov bh,cl"                 \
  "mov dl,13h"                \
  "call DevHelp"              \
  "setc cl"                   \
  "xor ch,ch"                 \
  "pop edx"                   \
  "mov [edx],bx"              \
  "mov [edx+2],ax"            \
  parm [ax] [cx] [bx] [edx]   \
  modify [eax ebx]            \
  value [cx];



WORD16 DevSegUnlock           // Unlock a segment locked by DevSegLock
  (
  HLOCK Handle                // Handle of locked segment
  );                          // Returns: 0 = success, other = error code

#pragma aux DevSegUnlock =    \
  "mov  bx, ax"               \
  "shr eax,16"                \
  "mov dl,14h"                \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  "neg dx"                    \
  "and ax,dx"                 \
  modify [edx ebx]            \
  parm [eax]                  \
  value [ax];

#define VMDHL_NOBLOCK           0x0001
#define VMDHL_CONTIGUOUS        0x0002
#define VMDHL_16M               0x0004
#define VMDHL_WRITE             0x0008
#define VMDHL_LONG              0x0010
#define VMDHL_VERIFY            0x0020

int DevVMLock
(
        unsigned long flags,                 
        unsigned long lin,                   
        unsigned long length,                
        LINEAR        pPageList,            
        LINEAR        pLockHandle,          
        LINEAR	      pPageListCount        
);                          // Returns: 0 = success, 1 = error

#pragma aux DevVMLock =       \
  "push edx"                  \
  "mov  dl,55h"               \
  "call DevHelp"  	      \
  "setc dl"		      \
  "pop  ebx"                  \
  "mov  dword ptr [ebx], eax" \ 
  "xor  eax, eax"             \
  "mov  al, dl"               \
  parm [eax] [ebx] [ecx] [edi] [esi] [edx] \
  modify [edx ebx] \
  value [eax];

#define VMDHA_16M               0x0001
#define VMDHA_FIXED             0x0002
#define VMDHA_SWAP              0x0004
#define VMDHA_CONTIG            0x0008
#define VMDHA_PHYS              0x0010
#define VMDHA_PROCESS           0x0020
#define VMDHA_SGSCONT           0x0040
#define VMDHA_RESERVE           0x0100
#define VMDHA_USEHIGHMEM        0x0800

int DevVMAlloc(ULONG flags, ULONG size, LINEAR physaddr, LINEAR addr);
#pragma aux DevVMAlloc = \
  "mov  dl,57h" \
  "call DevHelp"  	       \
  "setc dl"		       \
  "mov  dword ptr [esi], eax"  \ 
  "xor  eax, eax"              \
  "mov  al, dl"                \
  parm [eax] [ecx] [edi] [esi] \
  modify [edx] \
  value [eax];

ULONG DevVMFree(LINEAR LinearAddr);
#pragma aux DevVMFree = \
  "mov  dl,58h" \
  "call DevHelp"  	       \
  "setc dl"		       \
  "xor  eax, eax"              \
  "mov  al, dl"                \
  parm [eax] \
  modify [edx] \
  value [eax];

#define VMDHGP_WRITE            0x0001
#define VMDHGP_SELMAP           0x0002
#define VMDHGP_SGSCONTROL       0x0004
#define VMDHGP_4MEG             0x0008

ULONG DevVMGlobalToProcess(ULONG Flags, LINEAR LinearAddr, ULONG Length, 
			   LINEAR ProcessLinearAddr);
#pragma aux DevVMGlobalToProcess = \
  "mov  dl,5Ah" \
  "call DevHelp"  	       \
  "setc dl"		       \
  "mov  dword ptr [edi], eax"  \
  "xor  eax, eax"              \
  "mov  al, dl"                \
  parm [eax] [ebx] [ecx] [edi]\
  modify [edx] \
  value [eax];

#define VMDHPG_READONLY         0x0000
#define VMDHPG_WRITE            0x0001

ULONG DevVMProcessToGlobal(ULONG Flags, LINEAR LinearAddr, ULONG Length, 
			   LINEAR GlobalLinearAddr);
#pragma aux DevVMProcessToGlobal = \
  "mov  dl,59h"                \
  "call DevHelp"  	       \
  "setc dl"		       \
  "mov  dword ptr [edi], eax"  \
  "xor  eax, eax"              \
  "mov  al, dl"                \
  parm [eax] [ebx] [ecx] [edi] \
  modify [edx]                 \
  value [eax];

#define VMDHS_DECOMMIT          0x0001
#define VMDHS_RESIDENT          0x0002
#define VMDHS_SWAP              0x0004

ULONG DevVMSetMem(ULONG Flags, LINEAR LinearAddr, ULONG Size);
#pragma aux DevHelp_VMSetMem = \
  "mov  dl,66h"                \
  "call DevHelp"  	       \
  parm [eax] [ebx] [ecx]       \
  modify [edx]                 \
  value [eax];

ULONG DevVMUnLock(LINEAR pLockHandle);
#pragma aux DevVMUnLock = \
  "mov  dl,56h"      \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [esi]         \
  modify [edx]       \
  value [eax];
	       

WORD16 DevPhysToVirt          // Convert a physical address to a 16:16 address
  (
  PHYSICAL PhysAddr,          // Physical address to convert
  WORD16 Length,              // Length of virtual segment
  VIRTUAL FAR *VirtAddr           // Storage for virtual address
  );                          // Returns: 0 = success, other = error code

#pragma aux DevPhysToVirt =   \
  "xchg ax,bx"                \
  "mov dx,0115h"              \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  "neg dx"                    \
  "mov fs:[esi],di"           \
  "mov fs:[esi+2],es"         \
  "and ax,dx"                 \
  modify [dx es di]           \
  parm [bx ax] [cx] [fs esi]  \
  value [ax];



WORD16 DevVirtToPhys            // Convert a locked 16:16 address to physical
  (
  VIRTUAL VirtAddr,           // Virtual address to convert (must be locked)
  PHYSICAL FAR *PhysAddr      // Storage for physical address
  );                          // Returns: Nothing

#pragma aux DevVirtToPhys =   \
  "push ecx"                  \
  "push ds"		      \
  "mov si, ax"	 	      \
  "shr eax, 16"               \
  "mov ds, ax"                \
  "mov dl,16h"                \
  "call DevHelp_ModifiedDS"   \
  "setc dl"                   \
  "xor dh,dh"                 \
  "pop ds"		      \
  "pop ecx"                   \
  "mov fs:[ecx],bx"           \
  "mov fs:[ecx+2],ax"         \
  "mov ax,dx"                 \
  modify [edx ebx esi]        \
  parm [eax] [fs ecx]         \   
  value [ax];



WORD16 DevPhysToUVirt         // Convert a physical address to an LDT address
  (
  PHYSICAL PhysAddr,          // Physical address to convert
  WORD16 Length,              // Length of virtual segment
  BYTE Type,                  // Type (0 = code, 1 = data, 2 = free, 5 = video)
  BYTE Tag,                   // Tag (only with type 5)
  VIRTUAL FAR *VirtAddr       // Storage for virtual address
  );                          // Returns: 0 = success, 1 = error

#pragma aux DevPhysToUVirt =  \
  "push es"                   \
  "push edi"                  \
  "mov bx,ax"                 \
  "shr eax, 16"               \
  "mov dl,17h"                \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  "pop edi"                   \
  "mov word ptr fs:[edi], bx"   \
  "mov word ptr fs:[edi+2], es" \
  "pop es"                    \
  modify [ebx eax]            \
  parm [eax] [cx] [dh] [si] [fs edi] \
  value [dx];



WORD16 DevPhysAlloc           // Allocate a block of physical memory
  (
  WORD32 Size,                // Size of memory to allocate
  BYTE LoFlag,                // Flag: 0 = above 1 MB, 1 = below 1 MB
  PHYSICAL FAR *PhysAddr      // Storage for address of memory block
  );                          // Returns: 0 = success, other = error code

#pragma aux DevPhysAlloc =    \
  "mov bx, ax"                \
  "shr eax,16"                \
  "mov dl,18h"                \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  "neg dx"                    \
  "mov fs:[esi],bx"           \
  "mov fs:[esi+2],ax"         \
  "and ax,dx"                 \
  modify [edx ebx]            \
  parm [eax] [dh] [fs esi]    \
  value [ax];



WORD16 DevPhysFree            // Free a block of physical memory
  (
  PHYSICAL PhysAddr           // Address of memory block to be freed
  );                          // Returns: 0 = success, 1 = error

#pragma aux DevPhysFree =     \
  "xchg ax,bx"                \
  "mov dl,19h"                \
  "call DevHelp"  \
  "setc dl"                   \
  "xor dh,dh"                 \
  parm [bx ax]                \
  value [dx];



WORD16 DevIRQSet              // Register an interrupt handler for an IRQ
  (
  WORD16 Handler,             // Handler function entry-point
  WORD16 IRQ,                 // IRQ Number to handle
  BYTE SharableFlag           // Sharable: 1 = sharable, 0 = exclusive
  );                          // Returns: 0 = success, other = error code

#pragma aux DevIRQSet =       \
  "mov dl,1Bh"                \
  "call DevHelp"  \
  "setc dl"                   \
  "xor dh,dh"                 \
  "neg dx"                    \
  "and ax,dx"                 \
  modify [dl]                 \
  parm [ax] [bx] [dh]         \
  value [ax];



WORD16 DevIRQClear            // Remove a registered IRQ handler
  (
  WORD16 IRQ                  // IRQ Number to release
  );                          // Returns: 0 = success, 1 = error

#pragma aux DevIRQClear =     \
  "mov dl,1Ch"                \
  "call DevHelp"  \
  "setc dl"                   \
  "xor dh,dh"                 \
  parm [bx]                   \
  value [dx];

WORD16 DevVerifyAccess        // Verify that the caller has access to memory
  (
  VIRTUAL Address,            // Address of memory to verify
  WORD16 Length,              // Length of memory to verify
  BYTE Type                   // Type of access (0 = read, 1 = read/write)
  );                          // Returns: 0 = success, 1 = error

#pragma aux DevVerifyAccess = \
  "mov di, ax"                \
  "shr eax, 16"               \
  "mov dl,27h"                \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  modify [edx edi]            \
  parm [eax] [cx] [dh]        \
  value [dx];



WORD16 DevAttachDD             // Attach to another device driver for IDC
(
  WORD32 DriverName,           // Device driver name (from device header)
  WORD32 IDCData               // Storage for IDC data
);                             // Returns: 0 = success, 1 = error

#pragma aux DevAttachDD =     \
  "mov dl,2Ah"                \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  parm [ebx] [edi]            \
  value [dx];

WORD16 DevAllocGDTSel         // Allocate GDT selectors at Init time
  (
  WORD32 SelectorArray,       // Storage for allocated GDT selectors (16:16)
  WORD16 Count                // Number of GDT selectors to allocate
  );                          // Returns: 0 = success, other = error code

#pragma aux DevAllocGDTSel =  \
  "push es"                   \
  "mov eax, edi"	      \
  "shr eax, 16"               \
  "mov es, ax"                \
  "mov dl,2Dh"                \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  "neg dx"                    \
  "and ax,dx"                 \
  "pop es"                    \
  modify [edx eax]            \
  parm [edi] [cx]             \
  value [ax];




WORD16 DevPhysToGDTSelector   // Convert a 32 bit address to a GDT selector
  (
  PHYSICAL Address,           // 32 bit physical address to convert
  WORD16 Length,              // Length of fabricated segment
  SEL Selector                // Selector to refer to 32 bit address
  );                          // Returns: 0 = success, other = error code

#pragma aux DevPhysToGDTSelector = \
  "mov ax, bx"                \
  "shr eax,16"                \
  "mov dl,2Eh"                \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  "neg dx"                    \
  "and ax,dx"                 \
  modify [dx bx]              \
  parm [eax] [cx] [si]        \
  value [ax];




VOID DevEOI                   // Signal end of interrupt processing to PIC
  (
  WORD16 IRQ                  // IRQ number to end
  );                          // Returns: Nothing

#pragma aux DevEOI =          \
  "mov dl,31h"                \
  "call DevHelp"  \
  modify [dl]                 \
  parm [ax];


WORD16 DevRegisterPDD           // Register driver for PDD-VDD communications
  (
  WORD32 Name,                // Device driver name
  WORD32 EntryPoint           // Entry point for PDD-VDD communications
  );                          // Returns: 0 = success, 1 = error

#pragma aux DevRegisterPDD =  \
  "push ds"		      \
  "push es"		      \
  "mov si,ax"                 \
  "shr eax, 16"               \
  "mov ds, ax"                \ 
  "mov di, bx"                \
  "shr ebx, 16"               \
  "mov es, bx"                \
  "mov dl,50h"                \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  "pop es"                    \
  "pop ds"		      \
  modify [esi edi ebx]        \
  parm [eax] [ebx]            \
  value [dx];



WORD16 DevBeep                // Generate a beep
  (
  WORD16 Frequency,           // Beep pitch in hz
  WORD16 Duration             // Beep duration
  );                          // Returns: 0 = success, other = error code

#pragma aux DevBeep =         \
  "mov dl,52h"                \
  "call DevHelp"  \
  "setc dl"                   \
  "xor dh,dh"                 \
  "neg dx"                    \
  "and ax,dx"                 \
  modify [dx]                 \
  parm [bx] [cx]              \
  value [ax];



WORD16 DevFreeGDTSel          // Free an allocated GDT selector at task time
  (
  SEL Selector                // Selector to free
  );                          // Returns: 0 = success, other = error code

#pragma aux DevFreeGDTSel =   \
  "mov dl,53h"                \
  "call DevHelp"  \
  "setc dl"                   \
  "xor dh,dh"                 \
  "neg dx"                    \
  "and ax,dx"                 \
  modify [dx]                 \
  parm [ax]                   \
  value [ax];

#define GDTSEL_R3CODE           0x0000                               /*@V76282*/
#define GDTSEL_R3DATA           0x0001                               /*@V76282*/
#define GDTSEL_R2CODE           0x0003                               /*@V76282*/
#define GDTSEL_R2DATA           0x0004                               /*@V76282*/
#define GDTSEL_R0CODE           0x0005                               /*@V76282*/
#define GDTSEL_R0DATA           0x0006                               /*@V76282*/

/* GDTSEL_ADDR32 may be OR'd with above defines */                   /*@V76282*/
#define GDTSEL_ADDR32           0x0080                               /*@V76282*/


WORD16 DevPhysToGDTSel        // Map a physical address to a GDT selector
  (
  PHYSICAL Address,           // 32 bit physical address to convert
  WORD32 Length,              // Length of fabricated segment
  SEL Selector,               // Selector to refer to 32 bit address
  BYTE Access                 // Descriptor access priviledges
  );                          // Returns: 0 = success, other = error code

#pragma aux DevPhysToGDTSel = \
  "and ecx,0000FFFFh"         \
  "mov dl,54h"                \
  "call DevHelp"              \
  "setc dl"                   \
  "xor dh,dh"                 \
  "neg dx"                    \
  "and ax,dx"                 \
  parm [eax] [ecx] [si] [dh]  \
  modify [ecx edx]            \
  value [ax];



WORD16 DevVirtToLin           // Convert a virtual address to linear
  (
  SEL Selector,               // Selector to convert
  WORD32 Offset,              // Offset to convert
  LINEAR FAR *Linear              // Storage for linear address
  );                          // Returns: 0 = success, other = error code

#pragma aux DevVirtToLin =    \
  "mov dl,5Bh"                \
  "call DevHelp"              \
  "setc dl"                   \
  "mov fs:[edi], eax"         \
  "xor dh,dh"                 \
  "neg dx"                    \
  "and ax,dx"                 \
  modify [edx]		      \
  parm [ax] [esi] [fs edi]    \
  value [ax];


ULONG DevResetEventSem(ULONG hEvent, FARPTR16 pNumPosts);
#pragma aux DevResetEventSem = \
  "mov  dl,6Ah"      \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [eax] [edi]   \
  modify [edx]       \
  value [eax];

ULONG DevCloseEventSem(ULONG hEvent);
#pragma aux DevCloseEventSem = \
  "mov  dl,68h" \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [eax]         \
  modify [edx]       \
  value [eax];

ULONG DevOpenEventSem(ULONG hEvent);
#pragma aux DevOpenEventSem = \
  "mov  dl,67h" \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [eax]         \
  modify [edx]       \
  value [eax];

ULONG DevPostEventSem(ULONG hEvent);
#pragma aux DevPostEventSem = \
  "mov  dl,69h" \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [eax]         \
  modify [edx]       \
  value [eax];

ULONG DevFreeCtxHook(ULONG HookHandle);
#pragma aux DevFreeCtxHook = \
  "mov  dl,64h",     \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [eax]         \
  modify [edx]       \
  value [eax];

ULONG DevAllocateCtxHook(ULONG HookHandler, LINEAR HookHandle);
#pragma aux DevAllocateCtxHook = \
  "mov  ebx,-1"      \
  "mov  dl,63h"      \
  "call DevHelp"     \
  "setc dl"	     \
  "mov  dword ptr [edi], eax" \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [eax] [edi]   \
  modify [edx ebx] \
  value [eax];

ULONG DevArmCtxHook(ULONG HookData, ULONG HookHandle);
#pragma aux DevArmCtxHook = \
  "mov  ecx, -1"     \
  "mov  dl,65h",     \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [eax] [ebx]   \
  modify [edx ecx] \
  value [eax];

typedef struct {
  DWORD physaddr;
  DWORD size;
} PAGELIST;

ULONG DevPageListToLin(ULONG Size, PAGELIST NEAR *pPageList, LINEAR NEAR *LinearAddr);
#pragma aux DevPageListToLin = \
  "mov  dl,5Fh" \
  "call DevHelp"     \
  "setc dl"	     \
  "mov  [esi], eax"  \ 
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [ecx] [edi] [esi] \
  modify [edx]       \
  value [eax];

ULONG DevLinToPageList(LINEAR LinearAddr, ULONG Size, PAGELIST NEAR *pPageList);
#pragma aux DevLinToPageList = \
  "mov  dl,5Eh" \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  parm [eax] [ecx] [edi] \
  modify [edx]       \
  value [eax];

ULONG DevHelp_LinToGDTSelector(SEL Selector, LINEAR LinearAddr, ULONG Size);
#pragma aux DevHelp_LinToGDTSelector = \
   "mov  dl,5Ch" \
   "call DevHelp"     \
   "setc dl"	     \
   "xor  eax, eax"    \
   "mov  al, dl"      \
   value [eax] \
   modify [edx]       \
   parm [ax] [ebx] [ecx] \

#define DHGETDOSV_SYSINFOSEG            1
#define DHGETDOSV_LOCINFOSEG            2
#define DHGETDOSV_VECTORSDF             4
#define DHGETDOSV_VECTORREBOOT          5
#define DHGETDOSV_YIELDFLAG             7 
#define DHGETDOSV_TCYIELDFLAG           8 
#define DHGETDOSV_DOSCODEPAGE           11
#define DHGETDOSV_INTERRUPTLEV          13
#define DHGETDOSV_DEVICECLASSTABLE      14
#define DHGETDOSV_DMQSSELECTOR          15
#define DHGETDOSV_APMINFO               16

ULONG DevGetDOSVar(ULONG VarNumber, ULONG VarMember, VOID NEAR *KernalVar);
#pragma aux DevGetDOSVar = \
  "xor  ebx, ebx" \
  "mov  dl,24h" \
  "call DevHelp"     \
  "setc dl"	     \
  "mov  word ptr [edi],bx" \
  "mov  word ptr [edi+2],ax" \
  "xor  eax, eax"    \
  "mov  al, dl"      \
  value [eax] \
  parm [eax] [ecx] [edi] \
  modify [edx ebx];

ULONG DevSetTimer(DWORD TimerHandler);
#pragma aux DevSetTimer = \
  "mov  dl,1Dh" \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
   value [eax] \
   parm [eax] \
   modify [eax edx];

ULONG DevTickCount(DWORD TimerHandler, DWORD TickCount);
#pragma aux DevTickCount = \
  "mov  dl,33h" \
  "call DevHelp"     \
  "setc dl"	     \
  "xor  eax, eax"    \
  "mov  al, dl"      \
   value [eax] \
   parm [eax] [ebx] \
   modify [eax edx];

#ifdef FLATSTACK
#define kstrcpy strcpy
#define kmemcpy memcpy
#define kmemset memset
#else
#define kstrcpy _fstrcpy
#define kmemcpy _fmemcpy
#define kmemset _fmemset
#define strcpy  _fstrcpy
#define memcpy  _fmemcpy
#define memset  _fmemset
#endif

#endif // DevHelp_h
