/* $Id: osspci.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * Header for OSS PCI definitions
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

#ifndef __OSSPCI_H__
#define __OSSPCI_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RES_IRQ	2
#define MAX_RES_DMA	2
#define MAX_RES_IO	8
#define MAX_RES_MEM	8

#define RM_PNP_DEVICE   0
#define RM_PCI_DEVICE	1

#define MAX_PCI_DEVICES         16
#define MAX_PCI_BUSSES      16

    //#pragma pack(4) !!! by vladest
#pragma pack(1)

typedef struct 
{
    ULONG  busnr;
    ULONG  devnr;
    ULONG  funcnr;
    ULONG  devfn;
	USHORT irq[MAX_RES_IRQ];
	USHORT dma[MAX_RES_DMA];
	USHORT io[MAX_RES_IO];
	USHORT iolength[MAX_RES_IO];
	ULONG  mem[MAX_RES_MEM];
	ULONG  memlength[MAX_RES_MEM];
} IDC_RESOURCE;

#pragma pack()

typedef ULONG HRESMGR;

HRESMGR RMFindPCIDevice(ULONG vendorid, ULONG deviceid, IDC_RESOURCE *lpResource, int idx);
BOOL    RMRequestIO(HRESMGR hResMgr, ULONG ulIOBase, ULONG ulIOLength);
BOOL    RMRequestMem(HRESMGR hResMgr, ULONG ulMemBase, ULONG ulMemLength);
BOOL    RMRequestIRQ(HRESMGR hResMgr, ULONG ulIrq, BOOL fShared);
BOOL    RMRequestDMA(HRESMGR hResMgr, ULONG ulDMA);
//register resources & destroy resource manager object
void    RMFinialize(HRESMGR hResMgr);
//destroy resource manager object
void    RMDestroy(HRESMGR hResMgr);

extern HRESMGR hResMgr;


#ifdef __cplusplus
}
#endif

#endif
