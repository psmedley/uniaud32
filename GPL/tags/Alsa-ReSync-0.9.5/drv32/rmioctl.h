/* $Id: rmioctl.h,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * OS/2 Resource Manager IOCTL definitions
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

#ifndef __RM_HEADER__
#include "rmbase.h"
#endif

#ifndef __RM_IOCTL__
#define __RM_IOCTL__

#define CAT_RM                    0x80      /* Resource Manager Category  */
#define FUNC_RM_GET_NODEINFO      0x01      /* Get RM Structure           */
#define FUNC_RM_ENUM_NODES        0x02      /* Get Physical Topology      */
#define FUNC_RM_ENUM_DRIVERS      0x03      /* Get DriverHandles          */


/******************************************/
/* RM_NODE   Ring 3 Super Structure       */
/******************************************/

typedef struct {
   ULONG          Count;
   RESOURCESTRUCT Resource[1];
} RESOURCELIST, NEAR *NPRESOURCELIST, FAR48 *LPRESOURCELIST;

typedef struct {

   ULONG             VersionInfo;
   ULONG             NodeType;
   RMHANDLE          DriverHandle;
   union {
     FARPTR16  pAdapterNode;
     FARPTR16   pDeviceNode;
     FARPTR16     pLDevNode;
     FARPTR16  pSysNameNode;
     FARPTR16   pDriverNode;
     FARPTR16 pDetectedNode;
     FARPTR16 pResourceNode;
   };
   FARPTR16     pResourceList;
} RM_NODE, NEAR *NPRM_NODE;

/* pRM_NODE->NodeType */

#define RMTYPE_ADAPTER    0x0000
#define RMTYPE_DEVICE     0x0001
#define RMTYPE_LDEV       0x0002
#define RMTYPE_SYSNAME    0x0003
#define RMTYPE_DRIVER     0x0004
#define RMTYPE_DETECTED   0x0005
#define RMTYPE_RESOURCE   0x0006

/**************************/
/* FUNC_RM_GETNODEINFO    */
/**************************/

typedef struct {
   RMHANDLE RMHandle;
   ULONG    Linaddr;
} RM_GETNODE_PARM,  NEAR *NPRM_GETNODE_PARM;

typedef struct {
   ULONG          RMNodeSize;
   RM_NODE        RMNode;
} RM_GETNODE_DATA,  NEAR *NPRM_GETNODE_DATA, FAR48 *LPRM_GETNODE_DATA;


#define MAX_RESOURCES  10
#define MAX_TEXT_DATA 128

#define MAX_RM_NODE_SIZE ( sizeof(RM_NODE) +                          \
                           sizeof(ADAPTERSTRUCT)  + MAX_TEXT_DATA +  \
                           sizeof(DRIVERSTRUCT)   + MAX_TEXT_DATA +  \
                           sizeof(RESOURCESTRUCT) * MAX_RESOURCES       )


/**************************/
/* FUNC_RM_ENUM_NODE      */
/**************************/

typedef struct {
   USHORT   Command;       /* Size of Data Packet      */
} RM_ENUMNODES_PARM,  NEAR *NPRM_ENUMNODES_PARM;

/* pRM_ENUMNODES_PARM->Command */
#define RM_COMMAND_PHYS    0
#define RM_COMMAND_LOG     1
#define RM_COMMAND_DRVR    2
#define RM_COMMAND_PREVIOUS_DETECT  3
#define RM_COMMAND_CURRENT_DETECT   4

typedef struct {
   RMHANDLE RMHandle;
   ULONG    Depth;
} NODEENTRY, NEAR *NPNODEENTRY;

typedef struct {
   ULONG     NumEntries;        /* Number of Topology Entries       */
   NODEENTRY NodeEntry[1];      /* First Entry in Map of Nodes      */
} RM_ENUMNODES_DATA, NEAR *NPRM_ENUMNODES_DATA;


#endif  /* __RM_IOCTL__ */
