/* $Id: rm.hpp,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * OS/2 Resource Manager C++ interface header
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

#ifndef RM_HPP                  // Skip this file if already included.
#define RM_HPP

#define INCL_NOPMAPI
#define INCL_DOSINFOSEG      // Need Global info seg in rm.cpp algorithms
#include <os2.h>

#include <devtype.h>
#include <devinfo.h>
#include <include.h>            // Defn's for WatCom based drivers.

extern "C" {
   #include <rmbase.h>          // Resource manager definitions.
   #include "rmcalls.h"
   #include <rmioctl.h>
}


#define NumLogicalDevices 5     // Number of PNP logical devices on adapter.

#define MAX_DevID 1             // Maximum number of devices with a particular
                                // PnP Device ID that we're prepared to deal with.
                                // Intention is that this defines the number of
                                // adapters we're prepared to deal with.
#define MAX_DescTextLen MAX_TEXT_DATA
                                // MAX_TEXT_DATA in rmioctl.h.  Max length
                                // of the descriptive text on any of the free
                                // ASCIIZ fields in the RM structures.



/*
 * --- LDev_Resources class.
 */

/* These "maximums" for number of I/O, IRQ, etc. per logical device, are defined
   by PnP ISA spec Ver 1.0a 5/5/94, sect 4.6, p. 20. */

#define MAX_ISA_Dev_IO   8
#define MAX_ISA_Dev_IRQ  2
#define MAX_ISA_Dev_DMA  2
#define MAX_ISA_Dev_MEM  8
#define MAX_ResourceCount ( MAX_ISA_Dev_IO + MAX_ISA_Dev_IRQ \
                          + MAX_ISA_Dev_DMA + MAX_ISA_Dev_MEM )

// This value indicates an empty entry in an LDev_Resource data item.
#define NoIOValue 0xffff

class LDev_Resources {          // A class to hold the collection of resources
                                // needed by a single logical device.
 public:

   /* Public data.  Any unused data member is set to all 0xF.  Arrays are
    * always initialized to all 0xF's, then filled in in order, from 0..N.
    */
      USHORT uIOBase[ MAX_ISA_Dev_IO ];
      USHORT uIOLength[ MAX_ISA_Dev_IO ];
      USHORT uIRQLevel[ MAX_ISA_Dev_IRQ ];
      USHORT uDMAChannel[ MAX_ISA_Dev_DMA ];
      ULONG  uMemBase[ MAX_ISA_Dev_MEM ];
      ULONG  uMemLength[ MAX_ISA_Dev_MEM ];

   /* Information available from RM interface but not used:
    *   For all:  Flags (Exclusive, Multiplexed, Shared, Grant-Yield)
    *   IO:       IOAddressLines (10 or 16)
    *   IRQ:      PCI Irq Pin (10 or 16)
    *   DMA:      (Flags only)
    *   Memory:   Base, Length
    */

   // Are there any values in this structure?
   BOOL isEmpty ( void );

   // Clear the structure (set all fields to "NoIOValue".
   void vClear ( void );
};

#define MAX_PCI_BASE_ADDRESS	7

#pragma pack(1)
typedef struct
{
        USHORT VendorID;
        USHORT DeviceID;
        USHORT Command;
        USHORT Status;
        UCHAR  RevisionID;
        UCHAR  filler1[7];
        ULONG  Bar[MAX_PCI_BASE_ADDRESS];
        USHORT SubsystemVendorID;
        USHORT SubsystemID;
        ULONG  filler3[3];
        UCHAR  InterruptLine;
        UCHAR  InterruptPin;
        UCHAR  Max_Gnt;
        UCHAR  Max_Lat;
        UCHAR  TRDY_Timeout;
        UCHAR  Retry_Timeout;
        UCHAR  filler4[0x9a];
        UCHAR  CapabilityID;
        UCHAR  NextItemPtr;
        USHORT PowerMgtCapability;
        USHORT PowerMgtCSR;
} PCIConfigData;
#pragma pack()

#define PCI_CONFIG_ENABLE       0x80000000
#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC

void _outpd(int port, unsigned long data);
#pragma aux _outpd =       \
  "out dx, eax"                  \
  parm [dx] [eax];

unsigned long _inpd(int port);
#pragma aux _inpd =       \
  "in eax,dx"            \
  parm [dx]             \
  value [eax];


#define CONFIG_CMD(busnr, devfn, where)   (0x80000000 | (busnr << 16) | (devfn << 8) | (where & ~3))

/*
 * --- ResourceManager class.
 */

enum RMObjectState { rmDriverCreated, rmDriverFailed, rmAdapterCreated, rmAdapterFailed, rmAllocFailed };

class ResourceManager {         // A class to encapsulate system resource
                                // allocation issues.  Ref. documentation
 public:                        // at bottom of file.

   ResourceManager (  );
        // Register the device driver (this activates the RM interface).
        // Intention is that only one ResourceManager object is created
        // in driver, which will handle all audio.

   int bIsDevDetected ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice, int idx);
        // Return an indicator of whether or not named device is present.
        // in:   - PnP Device ID, compressed ASCII.
        //       - Search flags, ref rmbase.h SEARCH_ID_*;  also documented
        //         as SearchFlags parm in PDD RM API RMDevIDToHandleList().
        //         Only one of the search flags can be selected.
        // out:    True iff at least one device is detected
        // rem:    It's expected that the 1st parm, the PnP DevID, will
        //         be a DEVICE ID and the

   //###
   BOOL GetRMDetectedResources ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice );

   LDev_Resources* pGetDevResources ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice);

   inline RMObjectState getState() { return _state; };
        // Return state of RM object.

   BOOL requestIORange(ULONG ulBase, ULONG ulLength);
   BOOL requestMemRange(ULONG ulBase, ULONG ulLength);
   BOOL requestIRQ(USHORT usIRQ, BOOL fShared);
   BOOL requestDMA(USHORT usDMA);
   void releaseAllResources();

   BOOL registerResources();

   inline ULONG getBusNr()         { return busnr; };
   inline ULONG getDeviceNr()      { return devnr; };
   inline ULONG getFunctionNr()    { return funcnr; };
   inline ULONG getDevFuncNr()     { return devfn; };

 private:

   //--- Private data

   RMObjectState _state;   // Current state of object, as enumerated above.

   BOOL  _rmDetection;     // TRUE if OS/2 RM detection services available.

   //--- Private methods

   BOOL _pahRMAllocDetectedResources();

   APIRET _rmCreateAdapter();
   APIRET _rmCreateDevice( char *lpszDeviceName );

   BOOL   isPartOfAllocatedResources(int type, ULONG ulBase, ULONG ulLength);

   int    getPCIConfiguration(ULONG pciId, int idx);
   void   pci_write_config_dword(ULONG where, ULONG value);
   void   pci_read_config_dword(ULONG where, ULONG *pValue);

   ULONG          PCIConfig[64];
   PCIConfigData *pciConfigData;
   ULONG          busnr, devfn, devnr, funcnr;

   HRESOURCE      hResource[MAX_ResourceCount];
   int            idxRes;

   DEVID          DevID;

   LDev_Resources detectedResources;
   LDev_Resources resources;

static HDRIVER    _hDriver;    // Handle for our driver - assigned to us by RM.
static HADAPTER   _hAdapter;   // Handle for our adapter - output from RM.

};

extern ResourceManager* pRM;               // Resource manager object.


// definitions for 16 bits resource manager buffers
// (also in startup.inc!!)
#define MAXSIZE_RMNodeData      1024
#define MAXSIZE_RMResources     128
#define MAXSIZE_RMHandleList    128
#define MAXSIZE_RMResourceList  256

#endif  /* RM_HPP */

