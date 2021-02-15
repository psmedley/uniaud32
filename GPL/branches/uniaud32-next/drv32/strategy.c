/* $Id: strategy.c,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * Strategy entry point
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
 * (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
 * Copyright (c) 2013-2015 David Azarewicz david@88watts.net
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
#define INCL_DOSINFOSEG    // Need Global info seg in rm.cpp algorithms
#include <os2.h>

#include <devhelp.h>
#include <ossidc32.h>
#include <dbgos2.h>
#include <string.h>
#include <u32ioctl.h>
#include "devown.h"
#include "strategy.h"

ULONG StratRead(REQPACKET __far *_rp);
ULONG StratIOCtl(REQPACKET __far *_rp);

ULONG DiscardableInit(REQPACKET __far*);
ULONG deviceOwner = DEV_NO_OWNER;
ULONG numOS2Opens = 0;

extern DBGINT DbgInt;

//******************************************************************************
ULONG StratOpen(REQPACKET __far* rp)
{
  if (numOS2Opens == 0)
  {
    deviceOwner = DEV_PDD_OWNER;
  }
  numOS2Opens++;
  return RPDONE;
}

//******************************************************************************
ULONG StratClose(REQPACKET __far* rp)
{
  // only called if device successfully opened
  //  printk("strat close\n");
  numOS2Opens--;

  UniaudCloseAll(rp->open_close.usSysFileNum);

  if (numOS2Opens == 0)
  {
    deviceOwner = DEV_NO_OWNER;
  }
  return RPDONE;
}

//******************************************************************************
//DAZ #pragma off (unreferenced)
static ULONG StratWrite(REQPACKET __far* rp)
//DAZ #pragma on (unreferenced)
{
  return RPDONE | RPERR;
}

//******************************************************************************
// External initialization entry-point
//******************************************************************************
ULONG StratInit(REQPACKET __far* rp)
{
  ULONG rc;

  memset(&DbgInt, 0, sizeof(DbgInt));
  DbgPrintIrq();

  //DAZ RPInit __far* rp = (RPInit __far*)_rp;
  rc = DiscardableInit(rp);
  //dprintf(("StratInit End rc=%d", rc));
  DbgPrintIrq();
  DbgInt.ulState = 1;
  return rc;
}

//******************************************************************************
// External initialization complete entry-point
//******************************************************************************
//DAZ #pragma off (unreferenced)
ULONG StratInitComplete(REQPACKET __far* rp)
//DAZ #pragma on (unreferenced)
{
  DbgInt.ulState = 2;
  DbgPrintIrq();
  //dprintf(("StratInitComplete"));
  return(RPDONE);
}

//******************************************************************************
//DAZ #pragma off (unreferenced)
ULONG StratShutdown(REQPACKET __far *rp)
//DAZ #pragma on (unreferenced)
{
  //DAZ RPShutdown __far *rp = (RPShutdown __far *)_rp;

  //dprintf(("StratShutdown %d", rp->Function));
  if(rp->shutdown.Function == 1) //end of shutdown
  {
    OSS32_Shutdown();
  }
  return(RPDONE);
}

//******************************************************************************
// Handle unsupported requests
static ULONG StratError(REQPACKET __far* rp)
{
  return RPERR_BADCOMMAND | RPDONE;
}

//******************************************************************************
// Strategy dispatch table
//
// This table is used by the strategy routine to dispatch strategy requests
//******************************************************************************
typedef ULONG (*RPHandler)(REQPACKET __far* rp);
RPHandler StratDispatch[] =
{
  StratInit,          // 00 (BC): Initialization
  StratError,         // 01 (B ): Media check
  StratError,         // 02 (B ): Build BIOS parameter block
  StratError,         // 03 (  ): Unused
  StratRead,          // 04 (BC): Read
  StratError,         // 05 ( C): Nondestructive read with no wait
  StratError,         // 06 ( C): Input status
  StratError,         // 07 ( C): Input flush
  StratWrite,         // 08 (BC): Write
  StratError,         // 09 (BC): Write verify
  StratError,         // 0A ( C): Output status
  StratError,         // 0B ( C): Output flush
  StratError,         // 0C (  ): Unused
  StratOpen,          // 0D (BC): Open
  StratClose,         // 0E (BC): Close
  StratError,         // 0F (B ): Removable media check
  StratIOCtl,         // 10 (BC): IO Control
  StratError,         // 11 (B ): Reset media
  StratError,         // 12 (B ): Get logical unit
  StratError,         // 13 (B ): Set logical unit
  StratError,         // 14 ( C): Deinstall character device driver
  StratError,         // 15 (  ): Unused
  StratError,         // 16 (B ): Count partitionable fixed disks
  StratError,         // 17 (B ): Get logical unit mapping of fixed disk
  StratError,         // 18 (  ): Unused
  StratError,         // 19 (  ): Unused
  StratError,         // 1A (  ): Unused
  StratError,         // 1B (  ): Unused
  StratShutdown,        // 1C (BC): Notify start or end of system shutdown
  StratError,         // 1D (B ): Get driver capabilities
  StratError,         // 1E (  ): Unused
  StratInitComplete       // 1F (BC): Notify end of initialization
};

//******************************************************************************
// Strategy entry point
//
// The strategy entry point must be declared according to the STRATEGY
// calling convention, which fetches arguments from the correct registers.
//******************************************************************************
#pragma aux (STRATEGY) Strategy "ALSA_STRATEGY";
ULONG Strategy(REQPACKET __far* rp)
{
  if (rp->bCommand < sizeof(StratDispatch)/sizeof(StratDispatch[0])) return(StratDispatch[rp->bCommand](rp));
  else return(RPERR_BADCOMMAND | RPDONE);
}

