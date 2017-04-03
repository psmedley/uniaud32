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

VOID	RMInit(VOID);
BOOL    RMRequestIO(ULONG ulIOBase, ULONG ulIOLength);
BOOL    RMRequestMem(ULONG ulMemBase, ULONG ulMemLength);
BOOL    RMRequestIRQ(ULONG ulIrq, BOOL fShared, ULONG *phRes);
BOOL    RMDeallocateIRQ(ULONG hRes);
VOID	RMDone(ULONG DevID, ULONG *phAdapter, ULONG *phDevice);
VOID	RMSetHandles(ULONG hAdapter, ULONG hDevice);

#ifdef __cplusplus
}
#endif

#endif
