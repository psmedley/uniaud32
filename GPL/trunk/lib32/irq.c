/* $Id: irq.c,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
/*
 * OS/2 implementation of Linux irq kernel services
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

#include "linux.h"
#include <linux/init.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>

#define LINUX
#include <ossidc.h>
#include <ossidc32.h>
#include <osspci.h>
#include <dbgos2.h>
#include "irqos2.h"


static IRQHANDLER_INFO irqHandlers[MAX_IRQS][MAX_SHAREDIRQS] = {0};
static ULONG           nrIrqHandlers[MAX_IRQS] = {0};
static ULONG           eoiIrq[MAX_IRQS] = {0};

BOOL fInInterrupt = FALSE;
extern BOOL fSuspended; //pci.c

//******************************************************************************
//******************************************************************************
int request_irq(unsigned int irq,
                void (*handler)(int, void *, struct pt_regs *),
                unsigned long x0, const char *x1, void *x2)
{
    int i;

    if(RMRequestIRQ(hResMgr, irq, (x0 & SA_SHIRQ) ? TRUE : FALSE) == FALSE) {
        dprintf(("RMRequestIRQ failed for irq %d", irq));
        return NULL;
    }

    if(irq > 0xF) {
        DebugInt3();
        return 0;
    }

    for(i=0;i<MAX_SHAREDIRQS;i++) 
    {
        if(irqHandlers[irq][i].handler == 0) 
        {
            irqHandlers[irq][i].handler = handler;
            irqHandlers[irq][i].x0      = x0;
            irqHandlers[irq][i].x1      = (char *)x1;
            irqHandlers[irq][i].x2      = x2;
            nrIrqHandlers[irq]++;

            if(nrIrqHandlers[irq] == 1) 
            {
                if(RMSetIrq(irq, (x0 & SA_SHIRQ) ? TRUE : FALSE, &oss_process_interrupt) == FALSE) 
                {
                    break;
                }
            }
            return 0;
        }
    }
    dprintf(("request_irq: Unable to register irq handler for irq %d\n", irq));
    return 1;
}
//******************************************************************************
//******************************************************************************
void free_irq(unsigned int irq, void *userdata)
{
    int i;

    for(i=0;i<MAX_SHAREDIRQS;i++) 
    {
        if(irqHandlers[irq][i].x2 == userdata) {
            irqHandlers[irq][i].handler = 0;
            irqHandlers[irq][i].x0      = 0;
            irqHandlers[irq][i].x1      = 0;
            irqHandlers[irq][i].x2      = 0;
            if(--nrIrqHandlers[irq] == 0) {
                RMFreeIrq(irq);
            }
            break;
        }
    }
}
//******************************************************************************
//******************************************************************************
void eoi_irq(unsigned int irq)
{
    if(irq > 0xf) {
        DebugInt3();
        return;
    }
    eoiIrq[irq]++;
}
//******************************************************************************
//******************************************************************************
BOOL oss_process_interrupt(int irq)
{
 BOOL rc;
 int  i;

#ifdef DEBUG
// dprintf(("int proc"));
#endif

    if(fSuspended) 
    {//If our device is suspended, then we can't receive interrupts, so it must
     //be for some other device
     //Don't pass it to the linux handler as the device doesn't respond as expected
        //when suspended
#ifdef DEBUG
        dprintf(("IRQ %d suspended",irq));
#endif
        return FALSE;
    }

    for(i=0;i<MAX_SHAREDIRQS;i++) 
    {
        if(irqHandlers[irq][i].handler != 0)
        {
            fInInterrupt = TRUE;
            irqHandlers[irq][i].handler(irq, irqHandlers[irq][i].x2, 0);
            rc = (eoiIrq[irq] > 0);
            fInInterrupt = FALSE;
            if(rc) {
                //ok, this interrupt was intended for us; notify the 16 bits MMPM/2 driver
                OSS32_ProcessIRQ();
                eoiIrq[irq] = 0;
                return TRUE;
            }
        }
    }
    return FALSE;
}
//******************************************************************************
//******************************************************************************
int in_interrupt()
{
    return fInInterrupt;
}
//******************************************************************************
//******************************************************************************
void disable_irq(int irq)
{
    dprintf(("disable_irq %d NOT implemented", irq));
}
//******************************************************************************
//******************************************************************************
