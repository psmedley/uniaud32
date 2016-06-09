/*
 * Custom 32 bits stack management
 *
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
 */

#define INCL_NOPMAPI
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include <devhelp.h>
#include <ossidc.h>
#include <dbgos2.h>
#include <stacktoflat.h>
#ifdef KEE
#include <kee.h>
#endif

#define MAX_STACK               16
#define STACKSIZE	        (16*1024)
#define TOTAL_STACKSIZE	    MAX_STACK*(16*1024)
#define MAX_STACK_SEL       TOTAL_STACKSIZE/(64*1024)

#define STACK_FREE	    0
#define STACK_USED	    1

typedef struct {
	int  	state;
	LINEAR	addr;
    SEL     sel;
} PDDStack;

static PDDStack Ring0Stack[MAX_STACK]        = {0};
static SEL      Ring0StackSel[MAX_STACK_SEL] = {0};
static LINEAR StackBase                      = NULL;
static BOOL fInitialized                     = FALSE;

//*********************************************************************************************
//StackAlloc:
//
// Allocate stack when called the first time;
// Return private stack base
//
// NOTE: Stack is 16 bits when called
//*********************************************************************************************
DWORD StackAlloc();
#pragma aux StackAlloc "StackAlloc";
DWORD StackAlloc()
{
    int    i;
    DWORD  cpuflags;
    APIRET rc;
    SEL    gdts[MAX_STACK_SEL];
    SEL FAR48 *gdt;

    cpuflags = DevPushfCli();

    if(fInitialized == FALSE) {
    	fInitialized = TRUE;

        //allocate our private stack
        rc = DevVMAlloc(VMDHA_USEHIGHMEM|VMDHA_FIXED|VMDHL_CONTIGUOUS, TOTAL_STACKSIZE, (LINEAR)-1, (LINEAR)&StackBase);

        if(rc) {
            if (rc == 87)
            {
                rc = DevVMAlloc(VMDHA_FIXED|VMDHL_CONTIGUOUS, TOTAL_STACKSIZE, (LINEAR)-1, (LINEAR)&StackBase);
            }
            if (rc)
            {
                DebugInt3();
                return 0;
            }
        }
    	Ring0Stack[0].addr = StackBase;

        //allocate GDT selectors (so we can map a flat stack address to a 16:16 pointer)
        rc = DevAllocGDTSel(MAKE_FARPTR16_STACK((LINEAR)&gdts[0]), MAX_STACK_SEL);
        if(rc) {
            DebugInt3();
            return 0;
        }
        //and map linear stack addresses to GDT selectors
        for(i=0;i<MAX_STACK_SEL;i++) {
            LINEAR addr = (LINEAR)StackBase + i*64*1024;

            gdt = (SEL FAR48 *)MAKE_FARPTR32_STACK((LINEAR)&gdts[i]);
            if(DevHelp_LinToGDTSelector(*gdt, addr, 64*1024)) {
                DebugInt3();
                return 0;
            }
            Ring0StackSel[i] = *gdt;
        }

    	for(i=0;i<MAX_STACK;i++) {
		    Ring0Stack[i].addr = StackBase + i*STACKSIZE;
            Ring0Stack[i].sel  = Ring0StackSel[i*STACKSIZE/(64*1024)];
	    }
    }

    for(i=0;i<MAX_STACK;i++) {
	    if(Ring0Stack[i].state == STACK_FREE) {
    		Ring0Stack[i].state = STACK_USED;
		    DevPopf(cpuflags);
		    return (ULONG)(Ring0Stack[i].addr + STACKSIZE);
	    }
    }
    DevPopf(cpuflags);
    DebugInt3();    //oh, oh!!
    return 0;
}
//*********************************************************************************************
//*********************************************************************************************
void StackFree(DWORD stackaddr);
#pragma aux StackFree "StackFree" parm [eax];
void StackFree(DWORD stackaddr)
{
    int i;
    DWORD cpuflags;

    cpuflags = DevPushfCli();

    stackaddr -= STACKSIZE;
    for(i=0;i<MAX_STACK;i++) {
	    if(Ring0Stack[i].addr == (LINEAR)stackaddr) {
    		Ring0Stack[i].state = STACK_FREE;
		    DevPopf(cpuflags);
		    return;
	    }
    }
    DevPopf(cpuflags);
    DebugInt3();
}
//*********************************************************************************************
// FlatToSel:
//  Convert a flat stack address to a 16:16 address
//
//*********************************************************************************************
ULONG FlatToSel(ULONG addr32)
{
    ULONG offset;

    offset = addr32 - (ULONG)StackBase;

    if(offset > TOTAL_STACKSIZE) {
        //bad boy!
        DebugInt3();
        return 0;
    }

    return MAKE_FP16(Ring0StackSel[offset/(64*1024)], offset);
}
//*********************************************************************************************
//*********************************************************************************************

