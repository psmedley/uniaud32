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


BOOL fInInterrupt = FALSE;

//******************************************************************************
//******************************************************************************

static IRQ_SLOT		arSlots[MAX_IRQ_SLOTS] = { 0 };
static ULONG		   eoiIrq[255] = {0};


//******************************************************************************
//******************************************************************************
static IRQ_SLOT *FindSlot(unsigned irq)
{
	IRQ_SLOT	  *pSlot;

	for( pSlot = arSlots; pSlot != &arSlots[MAX_IRQ_SLOTS]; pSlot++ )
	{
		if( pSlot->irqNo == irq )	return pSlot;
	}

	return NULL;
}


//******************************************************************************
//******************************************************************************

int request_irq(unsigned irq, irq_handler_t handler,
			unsigned long ulSharedFlag, const char *pchName, void *pUserData)
{
	IRQ_SLOT 	*pSlot = FindSlot(irq & 0xff);
	unsigned 	u, uSlotNo = (unsigned)-1;
	ULONG hRes;

	rprintf(("request_irq: irq %d", irq & 0xff ));
	if ( !pSlot ) {
		// find empty slot
		for( uSlotNo = 0; uSlotNo < MAX_IRQ_SLOTS; uSlotNo++ ) {
			if( arSlots[uSlotNo].flHandlers == 0 ) {
				pSlot = &arSlots[uSlotNo];
				break;
			}
		}
	}

	if ( pSlot ) {
		hRes = 0;
		if (RMRequestIRQ(irq, (ulSharedFlag & SA_SHIRQ) != 0, &hRes) == FALSE) {
			rprintf(("RMRequestIRQ failed for irq %d", irq));
			return 1;
		}
		pSlot->irqNo = irq & 0xff;
		pSlot->hRes = hRes;

		for ( u = 0; u < MAX_SHAREDIRQS; u++ ) {
			if ( pSlot->irqHandlers[u].handler == NULL ) {
				pSlot->irqHandlers[u].handler = handler;
				pSlot->irqHandlers[u].x0 = ulSharedFlag;
				pSlot->irqHandlers[u].x1 = (char *)pchName;
				pSlot->irqHandlers[u].x2 = pUserData;

				if( pSlot->flHandlers != 0 || ALSA_SetIrq( irq & 0xff, uSlotNo, (ulSharedFlag & SA_SHIRQ) != 0) ) {
					pSlot->flHandlers |= 1 << u;
					return 0;
				}

				break;
			}
		}
	}

	rprintf(("request_irq: Unable to register irq handler for irq %d", irq & 0xff ));
	return 1;
}


//******************************************************************************
//******************************************************************************
void free_irq(unsigned int irq, void *userdata)
{
	unsigned 	u;
	IRQ_SLOT 	*pSlot;

	if( (pSlot = FindSlot(irq&0xff)) != NULL ) {
		for( u = 0; u < MAX_SHAREDIRQS; u++ ) {
			if( pSlot->irqHandlers[u].x2 == userdata ) {
				pSlot->flHandlers &= ~(1 << u);
				if( pSlot->flHandlers == 0 ) {
					rprintf(("free_irq: irq %d", irq & 0xff ));
					ALSA_FreeIrq(pSlot->irqNo);
					pSlot->irqNo = 0;
					RMDeallocateIRQ(pSlot->hRes);
					pSlot->hRes = 0;
					// pSlot->fEOI = 0;
				}

				pSlot->irqHandlers[u].handler = NULL;
				pSlot->irqHandlers[u].x0 = 0;
				pSlot->irqHandlers[u].x1 = NULL;
				pSlot->irqHandlers[u].x2 = NULL;

				return;

			}
		}
	}
}


//******************************************************************************
//******************************************************************************
void eoi_irq(unsigned int irq)
{
	/*(void)irq; */
	/*
	 IRQ_SLOT	*pSlot = FindSlot(irq);

	 if( pSlot )	pSlot->fEOI = 1;
	 */
	eoiIrq[irq & 0xff]++;
}


//******************************************************************************
//******************************************************************************
BOOL process_interrupt(ULONG ulSlotNo, ULONG *pulIrq)
{
	unsigned	u;
	int rc;
	IRQ_SLOT	*pSlot;

	//dprintf(("enter int proc %d %d",ulSlotNo, *pulIrq));

	if( ulSlotNo < MAX_IRQ_SLOTS )
	{
		pSlot = &arSlots[ulSlotNo];

		for( u = 0; u < MAX_SHAREDIRQS; u++ )
		{
			if(pSlot && pSlot->irqHandlers[u].handler )
			{
				fInInterrupt = TRUE;
#if 0
				rc = pSlot->irqHandlers[u].handler(pSlot->irqNo,
												   pSlot->irqHandlers[u].x2, 0);
#else
				rc = pSlot->irqHandlers[u].handler(pSlot->irqNo,
												   pSlot->irqHandlers[u].x2);
#endif

				// HDA Hardware generates controller interrupts and stream interrupts
				// the uniaud16 driver only cares about stream interrupts.
				// azx_interrupt in alsa-kernel/pci/hda/hda_intel.c will return rc 2 if
				// the interrupt is from the  controller. There is no need to call uniaud16
				// for these interrupts
				if ( rc == 2 ) {
					fInInterrupt = FALSE;
					*pulIrq = pSlot->irqNo;
					eoiIrq[pSlot->irqNo] = 0;
					return TRUE;
				}

				if (rc == 1) eoi_irq(pSlot->irqNo);
				rc = (eoiIrq[pSlot->irqNo] > 0);
				fInInterrupt = FALSE;

				if( rc /*== 1 || pSlot->fEOI*/ ) {

					*pulIrq = pSlot->irqNo;
					//		pSlot->fEOI = 0;

					//ok, this interrupt was intended for us; notify the 16 bits MMPM/2 driver
					OSS32_ProcessIRQ();
					//dprintf(("exit(1) int proc %d %d",ulSlotNo, *pulIrq));
					eoiIrq[pSlot->irqNo] = 0;
					return TRUE;
				}
			}
		}
	}
	//dprintf(("exit(0) int proc %d %d",ulSlotNo, *pulIrq));

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

