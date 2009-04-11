/* $Id: rmcalls.h,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * OS/2 Resource Manager API definitions
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

#ifndef __RM_CALLS__
#define __RM_CALLS__

#pragma pack (1)

#include <rmbase.h>
#include <rmioctl.h>
#include <devtype.h>

#pragma pack ()

#ifdef __cplusplus
  extern "C" {
#endif

#ifndef APIRET
#define APIRET ULONG
#endif

#ifndef CDECL
#define CDECL _cdecl
#endif

APIRET CDECL RMCreateDriver(FARPTR16, FARPTR16);

APIRET CDECL RMDestroyDriver(HDRIVER hDriver);

APIRET CDECL RMCreateAdapter(HDRIVER hDriver, FARPTR16 phAdapter,
                             FARPTR16 pAdapterStruct, HDEVICE hDevice,
                             FARPTR16 pahResource);

APIRET CDECL RMDestroyAdapter(HDRIVER hDriver, HADAPTER hAdapter);

APIRET CDECL RMCreateDevice(HDRIVER hDriver, FARPTR16 phDevice,
                            FARPTR16 pDeviceStruct, HADAPTER hAdapter,
                            FARPTR16 pahResource);

APIRET CDECL RMAllocResource(HDRIVER hDriver, FARPTR16 phResource,
                             FARPTR16 pResourceStruct);

APIRET CDECL RMDeallocResource(HDRIVER hDriver, HRESOURCE hResource);

APIRET CDECL RMGetNodeInfo(RMHANDLE RMHandle, 
                           FARPTR16 pNodeInfo,
	                   USHORT   BufferSize);

APIRET CDECL RMDevIDToHandleList(RMHANDLE RMHandle, 
                                 DEVID DeviceID,
    	                         DEVID FunctionID,
                                 DEVID CompatibleID,
	  	 	         VENDID VendorID,
			         SERNUM SerialNumber,
                                 SEARCHIDFLAGS SearchFlags,
                                 HDETECTED hStartNode,
                                 FARPTR16 pHndList);

APIRET CDECL RMHandleToResourceHandleList(RMHANDLE hHandle,
                                          FARPTR16 pHndList);

#ifdef __cplusplus
	};
#endif

/****************************************************************************/
/*                                                                          */
/* Resource Manager Return Codes                                            */
/*                                                                          */
/****************************************************************************/
#define RMRC_SUCCESS               0x0000
#define RMRC_NOTINITIALIZED        0x0001
#define RMRC_BAD_DRIVERHANDLE      0x0002
#define RMRC_BAD_ADAPTERHANDLE     0x0003
#define RMRC_BAD_DEVICEHANDLE      0x0004
#define RMRC_BAD_RESOURCEHANDLE    0x0005
#define RMRC_BAD_LDEVHANDLE        0x0006
#define RMRC_BAD_SYSNAMEHANDLE     0x0007
#define RMRC_BAD_DEVHELP           0x0008
#define RMRC_NULL_POINTER          0x0009
#define RMRC_NULL_STRINGS          0x000a
#define RMRC_BAD_VERSION           0x000b
#define RMRC_RES_ALREADY_CLAIMED   0x000c
#define RMRC_DEV_ALREADY_CLAIMED   0x000d
#define RMRC_INVALID_PARM_VALUE    0x000e
#define RMRC_OUT_OF_MEMORY         0x000f
#define RMRC_SEARCH_FAILED         0x0010
#define RMRC_BUFFER_TOO_SMALL      0x0011
#define RMRC_GENERAL_FAILURE       0x0012
#define RMRC_IRQ_ENTRY_ILLEGAL     0x0013
#define RMRC_NOT_IMPLEMENTED       0x0014
#define RMRC_NOT_INSTALLED         0x0015
#define RMRC_BAD_DETECTHANDLE      0x0016
#define RMRC_BAD_RMHANDLE          0x0017
#define RMRC_BAD_FLAGS             0x0018
#define RMRC_NO_DETECTED_DATA      0x0019


/*********************/
/*  RM Types         */
/*********************/

typedef struct {
   UCHAR      MajorVersion;
   UCHAR      MinorVersion;
   USHORT     NumRSMEntries;
} RSMVERSTRUCT;

#endif /* __RM_CALLS__ */
