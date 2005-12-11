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
static FARPTR16 *pISR[NUM_IRQLEVELS] = {
   NULL,
   NULL,
   NULL,
   &ISR03,
   &ISR04,
   &ISR05,
   NULL,
   &ISR07,
   NULL,
   &ISR09,
   &ISR10,
   &ISR11,
   &ISR12,
   &ISR13,
   &ISR14,
   &ISR15,
};

static PFNIRQ pfnAlsaIRQHandler = NULL;

//******************************************************************************
//******************************************************************************
BOOL RMSetIrq(ULONG ulIrq, BOOL fShared, PFNIRQ pfnIrqHandler)
{
    USHORT rc = 1;

    if(pISR[ulIrq] == NULL) {
        DebugInt3();
        return FALSE;
    }
    if(fShared) 
    {
        rc = DevIRQSet((WORD16) *pISR[ ulIrq ],
                       (WORD16)ulIrq,
                       1 );   // first try shared shared
    }
    if (rc != 0) {                    // If error ...
        rc = DevIRQSet((WORD16) *pISR[ ulIrq ],
                       (WORD16)ulIrq,
                       0);   // failed, so try exclusive instead
    }
    if (rc != 0) {                    // If error ...
        dprintf(("ERROR: RMSetIrq %d %d %x FAILED!!", ulIrq, fShared, pfnIrqHandler));
        DebugInt3();
        return FALSE;
    }
    //Always called with the same handler
    if(pfnAlsaIRQHandler && (pfnAlsaIRQHandler != pfnIrqHandler)) {
        DebugInt3();
        return FALSE;
    }
    pfnAlsaIRQHandler = pfnIrqHandler;
    return TRUE;
}
//******************************************************************************
//******************************************************************************
BOOL RMFreeIrq(ULONG ulIrq)
{
    return (DevIRQClear((WORD16)ulIrq) == 0);
}
//******************************************************************************
//******************************************************************************
ULONG ALSA_Interrupt(ULONG irqnr);
#pragma aux ALSA_Interrupt "ALSA_Interrupt" parm [ebx]
ULONG ALSA_Interrupt(ULONG irqnr)
{
   if(pfnAlsaIRQHandler == NULL) {
       DebugInt3();
       return FALSE;
   }
#ifdef DEBUG
   dprintf2(("irq %d",irqnr));
#endif
   if(pfnAlsaIRQHandler(irqnr)) {
       cli();
       // We've cleared all service requests.  Send EOI and clear
       // the carry flag (tells OS/2 kernel that Int was handled).
       DevEOI( (WORD16)irqnr );
       return TRUE;
   }
   // Indicate Interrupt not serviced by setting carry flag before
   // returning to OS/2 kernel.  OS/2 will then shut down the interrupt!
   // NOTE: Make sure interrupts are not turned on again when this irq isn't ours!
   return FALSE;
}
//******************************************************************************
//******************************************************************************
