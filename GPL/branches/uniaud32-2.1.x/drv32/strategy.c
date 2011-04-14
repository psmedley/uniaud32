/* $Id: strategy.c,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * Strategy entry point
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
#define INCL_DOSINFOSEG 	 // Need Global info seg in rm.cpp algorithms
#include <os2.h>

#include <devhelp.h>
#include <devrp.h>
#include <devown.h>
#include "strategy.h"
#include <ossidc32.h>
#include <dbgos2.h>
#include <string.h>

ULONG StratRead(RP __far *_rp);
ULONG StratIOCtl(RP __far *_rp);
ULONG StratClose(RP __far *_rp);

ULONG DiscardableInit(RPInit __far*);
ULONG deviceOwner = DEV_NO_OWNER;
ULONG numOS2Opens = 0;
extern "C" BOOL fRewired; //pci.c

extern "C" DBGINT DbgInt;

//******************************************************************************
//******************************************************************************
ULONG StratOpen(RP __far*)
{
	if (numOS2Opens == 0) {
		deviceOwner = DEV_PDD_OWNER;
	}
	numOS2Opens++;
		return RPDONE;
}
//******************************************************************************
//******************************************************************************
#pragma off (unreferenced)
static ULONG StratWrite(RP __far* _rp)
#pragma on (unreferenced)
{
  return RPDONE | RPERR;
}
//******************************************************************************
// External initialization entry-point
//******************************************************************************
ULONG StratInit(RP __far* _rp)
{
	ULONG rc;

	memset(&DbgInt, 0, sizeof(DbgInt));
	DbgPrintIrq();

	RPInit __far* rp = (RPInit __far*)_rp;
	rc = DiscardableInit(rp);
	//dprintf(("StratInit End rc=%d", rc));
	DbgPrintIrq();
	DbgInt.ulState = 1;
	return rc;
}
//******************************************************************************
// External initialization complete entry-point
#ifdef ACPI
#include "irqos2.h"
#endif //ACPI
//******************************************************************************
#pragma off (unreferenced)
ULONG StratInitComplete(RP __far* _rp)
#pragma on (unreferenced)
{
	DbgInt.ulState = 2;
#ifdef ACPI
	PciAdjustInterrupts();
#endif
	DbgPrintIrq();
	//dprintf(("StratInitComplete"));
	return(RPDONE);
}
//******************************************************************************
//******************************************************************************
#pragma off (unreferenced)
ULONG StratShutdown(RP __far *_rp)
#pragma on (unreferenced)
{
	RPShutdown __far *rp = (RPShutdown __far *)_rp;

	//dprintf(("StratShutdown %d", rp->Function));
	if(rp->Function == 1) {//end of shutdown
		OSS32_Shutdown();
	}
	return(RPDONE);
}
//******************************************************************************
//******************************************************************************
// Handle unsupported requests
static ULONG StratError(RP __far*)
{
  return RPERR_COMMAND | RPDONE;
}
//******************************************************************************
// Strategy dispatch table
//
// This table is used by the strategy routine to dispatch strategy requests
//******************************************************************************
typedef ULONG (*RPHandler)(RP __far* rp);
RPHandler StratDispatch[] =
{
  StratInit,				  // 00 (BC): Initialization
  StratError,				  // 01 (B ): Media check
  StratError,				  // 02 (B ): Build BIOS parameter block
  StratError,				  // 03 (  ): Unused
  StratRead,				  // 04 (BC): Read
  StratError,				  // 05 ( C): Nondestructive read with no wait
  StratError,				  // 06 ( C): Input status
  StratError,				  // 07 ( C): Input flush
  StratWrite,				  // 08 (BC): Write
  StratError,				  // 09 (BC): Write verify
  StratError,				  // 0A ( C): Output status
  StratError,				  // 0B ( C): Output flush
  StratError,				  // 0C (  ): Unused
  StratOpen,				  // 0D (BC): Open
  StratClose,				  // 0E (BC): Close
  StratError,				  // 0F (B ): Removable media check
  StratIOCtl,				  // 10 (BC): IO Control
  StratError,				  // 11 (B ): Reset media
  StratError,				  // 12 (B ): Get logical unit
  StratError,				  // 13 (B ): Set logical unit
  StratError,				  // 14 ( C): Deinstall character device driver
  StratError,				  // 15 (  ): Unused
  StratError,				  // 16 (B ): Count partitionable fixed disks
  StratError,				  // 17 (B ): Get logical unit mapping of fixed disk
  StratError,				  // 18 (  ): Unused
  StratError,				  // 19 (  ): Unused
  StratError,				  // 1A (  ): Unused
  StratError,				  // 1B (  ): Unused
  StratShutdown,			  // 1C (BC): Notify start or end of system shutdown
  StratError,				  // 1D (B ): Get driver capabilities
  StratError,				  // 1E (  ): Unused
  StratInitComplete 		  // 1F (BC): Notify end of initialization
};
//******************************************************************************
// Strategy entry point
//
// The strategy entry point must be declared according to the STRATEGY
// calling convention, which fetches arguments from the correct registers.
//******************************************************************************
ULONG Strategy(RP __far* rp);
#pragma aux (STRATEGY) Strategy "ALSA_STRATEGY";
ULONG Strategy(RP __far* rp)
{
	if (fRewired) {
		fRewired = FALSE;
		rprintf(("Strategy: Resuming"));
		OSS32_APMResume();
		DbgPrintIrq();
	}

	if (rp->Command < sizeof(StratDispatch)/sizeof(StratDispatch[0]))
	   	return(StratDispatch[rp->Command](rp));
	else return(RPERR_COMMAND | RPDONE);
}
//******************************************************************************
//******************************************************************************
