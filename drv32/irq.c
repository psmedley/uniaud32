/* $Id: irq.cpp,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * IRQ handler functions
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
 *
 */

#define INCL_NOPMAPI
#define INCL_DOSINFOSEG      // Need Global info seg in rm.cpp algorithms
#include <os2.h>

#include <devtype.h>
#include <devinfo.h>
#include <devhelp.h>
#include <include.h>            // Defn's for WatCom based drivers.
#include <irqos2.h>
#include <dbgos2.h>
#include "irq.h"

// List of handlers here.
static FARPTR16 *pISR[MAX_IRQ_SLOTS] = {
   &ISR00,
   &ISR01,
   &ISR02,
   &ISR03,
   &ISR04,
   &ISR05,
   &ISR06,
   &ISR07
};

extern DBGINT DbgInt;

//******************************************************************************
BOOL ALSA_SetIrq(ULONG ulIrq, ULONG ulSlotNo, BOOL fShared)
{
    USHORT rc = 1;

    if( ulSlotNo >= MAX_IRQ_SLOTS ) {
        DebugInt3();
        return FALSE;
    }

    if(fShared)
    {
	rc = DevIRQSet((WORD16) *pISR[ulSlotNo],
                       (WORD16)ulIrq,
                       1 );   // first try shared shared
    }

    if (rc != 0) {                    // If error ...
       rprintf(("ERROR: RMSetIrq %d %d %x - failed to set shared - trying exclusive!!", ulIrq, fShared, ulSlotNo));
	rc = DevIRQSet((WORD16) *pISR[ulSlotNo],
                       (WORD16)ulIrq,
                       0);   // failed, so try exclusive instead
    }

    if (rc != 0) {                    // If error ...
        rprintf(("ERROR: RMSetIrq %d %d %x FAILED shared and exclusive mode!!", ulIrq, fShared, ulSlotNo));
        DebugInt3();
        return FALSE;
    }

    return TRUE;
}

//******************************************************************************
BOOL ALSA_FreeIrq(ULONG ulIrq)
{
    return (DevIRQClear((WORD16)ulIrq) == 0);
}

//******************************************************************************
#pragma aux ALSA_Interrupt "ALSA_Interrupt" parm [ebx]
ULONG ALSA_Interrupt(ULONG ulSlotNo)
{
    ULONG	ulIrqNo;

   // enable interrupts that have higher priority we should
   // allow higher priority interrupts
   sti();
   if( process_interrupt(ulSlotNo, &ulIrqNo) ) {
		DbgInt.ulIntServiced[DbgInt.ulState]++;
       // We've cleared all service requests.
       // Clear (disable) Interrupts, Send EOI
       // and clear the carry flag (tells OS/2 kernel that Int was handled).
       // Note carry flag is handled in setup.asm
       cli();
       DevEOI( (WORD16)ulIrqNo );
       return TRUE;
   }
	DbgInt.ulIntUnserviced[DbgInt.ulState]++;
   // Indicate Interrupt not serviced by setting carry flag before
   // returning to OS/2 kernel.  OS/2 will then shut down the interrupt!
   // NOTE: Make sure interrupts are not turned on again when this irq isn't ours!
   return FALSE;
}

