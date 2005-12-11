/* $Id: rm.cpp,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * OS/2 Resource Manager C++ interface
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

extern "C" int cdecl printk(const char * fmt, ...);

#include "rm.hpp"                      // Will include os2.h, etc.
#include <devhelp.h>
#include <devinfo.h>
#include <malloc.h>
#include <string.h>
#include <dbgos2.h>
#include <version.h>
#include <osspci.h>
#include <linux\pci.h>
#include "pciids.h"
#include <unicard.h>
#include <ossidc32.h>

/**@external LDev_Resources::isEmpty
 *  Returns TRUE iff the LDev_Resources structure has no information.
 * @param None.
 * @return BOOL
 */
BOOL LDev_Resources::isEmpty()
{
   BOOL bIsEmpty = TRUE;

   for ( int i=0; i<MAX_ISA_Dev_IO; ++i) {
      if (uIOBase[i] != NoIOValue)
         bIsEmpty = FALSE;
      if ((i < MAX_ISA_Dev_IRQ) && (uIRQLevel[i] != NoIOValue))
         bIsEmpty = FALSE;
      if ((i < MAX_ISA_Dev_DMA) && (uDMAChannel[i] != NoIOValue))
         bIsEmpty = FALSE;
      if ((i < MAX_ISA_Dev_MEM) && (uMemBase[i] != 0xffffffff))
         bIsEmpty = FALSE;
   }
   return bIsEmpty;
}


/**@external LDev_Resources::vClear
 *  Set an LDev_Resources structure to Empty.
 * @param None.
 * @return VOID
 */
void LDev_Resources::vClear()
{
   memset( (PVOID) this, NoIOValue, sizeof(LDev_Resources) );
}


/*
 * --- Linkages required by system Resource Manager (rm.lib).
 */

extern "C" PFN    RM_Help;
extern "C" PFN    RM_Help0;
extern "C" PFN    RM_Help3;
extern "C" ULONG  RMFlags;

/*
 * --- Public member functions.
 */

/**@external ResourceManager
 *  Constructor for RM object.
 * @notes Creates a "driver" node for this device driver, but does not
 *  allocate resources.
 */
ResourceManager::ResourceManager() :
           busnr(0), devnr(0), funcnr(0), devfn(0), DevID(0), idxRes(0)
{
   APIRET rc;
   DRIVERSTRUCT DriverStruct;
   char DriverName[sizeof(RM_DRIVER_NAME)];
   char VendorName[sizeof(RM_DRIVER_VENDORNAME)];
   char DriverDesc[sizeof(RM_DRIVER_DESCRIPTION)];
   FARPTR16 p;
   GINFO FAR *pGIS = 0;
   HDRIVER hDriver;

   /* Warp version level, bus type, machine ID, and much other information is
    * readily available.  Reference the RM ADD sample in the DDK for code.
    *
    * Create a "driver" struct for this driver.  This must ALWAYS be the
    * first true RM call executed, as it attaches the Resource Manager
    * library and performs setup for the other RM calls.
    */
   memset( (PVOID) &DriverStruct, 0, sizeof(DriverStruct) );

   //copy strings to stack, because we need to give RM 16:16 pointers
   //(which can only be (easily) generated from 32 bits stack addresses)
   strcpy(DriverName, RM_DRIVER_NAME);
   strcpy(VendorName, RM_DRIVER_VENDORNAME);
   strcpy(DriverDesc, RM_DRIVER_DESCRIPTION);

   DriverStruct.DrvrName     = FlatToSel((ULONG)DriverName);        /* ### IHV */
   DriverStruct.DrvrDescript = FlatToSel((ULONG)DriverDesc);        /* ### IHV */
   DriverStruct.VendorName   = FlatToSel((ULONG)VendorName);        /* ### IHV */
   DriverStruct.MajorVer     = CMVERSION_MAJOR;          //rmbase.h /* ### IHV */
   DriverStruct.MinorVer     = CMVERSION_MINOR;          //rmbase.h /* ### IHV */
   DriverStruct.Date.Year    = RM_DRIVER_BUILDYEAR;                    /* ### IHV */
   DriverStruct.Date.Month   = RM_DRIVER_BUILDMONTH;                   /* ### IHV */
   DriverStruct.Date.Day     = RM_DRIVER_BUILDDAY;                     /* ### IHV */
   DriverStruct.DrvrType     = DRT_AUDIO;
   DriverStruct.DrvrSubType  = 0;
   DriverStruct.DrvrCallback = NULL;

   if(_hDriver == 0) {
       //hDriver must be used as FlatToSel only works for stack variables
       rc = RMCreateDriver( FlatToSel((ULONG)&DriverStruct), FlatToSel((ULONG)&hDriver) );
       if( rc == RMRC_SUCCESS ) {
          _state = rmDriverCreated;
          _hDriver = hDriver;
       }
       else {
          _state = rmDriverFailed;
          _hDriver = 0;
       }
   }
   else {
       //Only create one driver object
       _state = rmDriverCreated;
   }
   // Build a pointer to the Global Information Segment.
   rc = DevGetDOSVar( DHGETDOSV_SYSINFOSEG, 0, (VOID NEAR *)&p );
   if (rc) {
      _rmDetection = FALSE;
   }
   else {
      SEL FAR48 *pSel = (SEL FAR48 *)MAKE_FARPTR32(p);
      pGIS = (GINFO FAR *)MAKE_FARPTR32((ULONG)(*pSel << 16));
      _rmDetection =
         ( (pGIS->MajorVersion > 20) ||
           ((pGIS->MajorVersion == 20) && (pGIS->MinorVersion > 30)) );
   }
   detectedResources.vClear();
   resources.vClear();
}

#pragma off (unreferenced)
int ResourceManager::bIsDevDetected( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice, int idx)
#pragma on (unreferenced)
/*
;  PURPOSE: Search the Resource Manager's "current detected" tree for
;           the matching PnP ID.
;
;  IN:    - DevID - PnP Device ID being sought (Compressed Ascii).
;         - ulSearchFlags - Search flags, ref rmbase.h SEARCH_ID_*;  also
;           documented as SearchFlags parm in PDD RM API RMDevIDToHandleList().
;           Defines whether DevID is a Device ID, Logical device ID, Compatible
;           ID, or Vendor ID.
;
;  OUT:     Boolean indicator, TRUE when number of matching detected devices > 0.
;
*/
{
    int detected = 0;

    detected = getPCIConfiguration(DevID, idx);
    if(!detected) {
        return 0;
   }

   this->DevID = DevID;

   //Manual detection in ResourceManager class constructor;
   return (_state == rmDriverCreated || _state == rmAdapterCreated);
}



/**@internal GetRMDetectedResources
 *  Return the set of IO ports, IRQ levels, DMA channels, & memory ranges
 *  required by the specified device, as detected by the OS/2 resource
 *  manager.
 * @param Refer to _bIsDevDetected() for parameters.
 * @notes It's expectded that the spec'd DevID & flags will select a single
 *  device in the system.  If multiples are found, the first matching device
 *  is referenced.
 * @return LDev_Resources* object, filled in with required resources.  Object
 *  is allocated from heap, is responsibility of caller to free object.
 *  Ordering on the resources is preserved.   Unused fields are set to NoIOValue.
 * @return NULL on error situations.
 */
#pragma off (unreferenced)
BOOL ResourceManager::GetRMDetectedResources ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice)
#pragma on (unreferenced)
{
   //Fill in resources read from PCI Configuration space
   detectedResources.uIRQLevel[0]  = pciConfigData->InterruptLine;
   if(detectedResources.uIRQLevel[0] == 0 || detectedResources.uIRQLevel[0] > 255)  {
       //Does not matter if device doesn't have any resources (like a PCI bus controller)
       dprintf(("Invalid PCI irq %x", (int)detectedResources.uIRQLevel[0]));
       detectedResources.uIRQLevel[0] = NoIOValue;
   }

   int idxmem = 0, idxio = 0;
   for(int i=0;i<MAX_PCI_BASE_ADDRESS;i++)
   {
       if(pciConfigData->Bar[i] != -1)
       {
           ULONG l, barsize;
           int reg = PCI_BASE_ADDRESS_0 + (i << 2);

           pci_read_config_dword(reg, &l);
           if (l == 0xffffffff)
           {
               dprintf(("l == 0xffffffff\n"));
               return 0;
           }

           pci_write_config_dword(reg, ~0);
           pci_read_config_dword(reg, &barsize);
           pci_write_config_dword(reg, l);

           if (!barsize || barsize == 0xffffffff)
               continue;

           if ((l & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY) {
               barsize = ~(barsize & PCI_BASE_ADDRESS_MEM_MASK);
               detectedResources.uMemBase[idxmem]     = (pciConfigData->Bar[i] & 0xFFFFFFF0);
               detectedResources.uMemLength[idxmem++] = barsize + 1;
           }
           else {
               barsize = ~(barsize & PCI_BASE_ADDRESS_IO_MASK) & 0xffff;
               detectedResources.uIOBase[idxio]     = (USHORT)(pciConfigData->Bar[i] & 0xFFFFFFF0);
               detectedResources.uIOLength[idxio++] = (USHORT)barsize + 1;
           }
       }
   }
   return TRUE;
}


/**@external pGetDevResources
 *
 *  Allocate set of IO ports, IRQ levels, DMA channels requested by the
 *  specified device.
 *
 * @param Refer to bIsDevDetected()
 *
 * @return LDev_Resources object, filled in with required resources.  Object
 *  is returned on stack. Ordering on the resources is preserved.   Unused
 *  fields are set to 0xFF.
 *
 * @notes The allocation from OS/2's RM is required; if not performed, the
 *  resources could be allocated by a driver which loads after this one.
 *  Also perform other bookeepping by registering driver, adapter, and
 *  device with the system RM.
 *
 * @notes Additional comments from DevCon 'ADD' sample for RM: "Create all
 *  Resource Manager nodes required by this driver.  The Resource Manager
 *  structures created in this module may be thought of as being allocated
 *  in a seperate tree called the 'driver' tree.  The snooper resource nodes
 *  were created in the 'current detected' tree.  Therefore, these
 *  allocations will not compete with previous snooper resource allocations.
 *
 * @notes
 *
 *  - Fetch defaults for named device
 *     - Warp3:  GetSpecDefaults( pnpID )
 *     - (not implemented:)  snoop on Warp3 -> GetSnoopedResources()
 *     - Warp4:  GetLDev_Resources( pnpID ) (rename to GetRMDetectedResources())
 *     - @return LDev_Resources structure
 *  - Fill in any user overrides
 *     - object Override( pnpID ) subclasses an LDev_Resources
 *     - has an added "exists" flag, set true if any overrides exist
 *     - on creation, interacts with parsed information for overrides
 *     - bool Override.exist( pnpID )
 *     - LDev_Resources Override.apply( pnpID, LDev_Resources )
 *  - Format LDev_Resources into RESOURCELIST
 *     - pResourceList = MakeResourceList( LDev_Resources )
 *  - Allocate adapter if this is the 1st time through
 *     - RMCreateAdapter()
 *  - Allocate resources to device
 *     - Call GetDescriptiveName( pnpID ) if Warp3
 *     - Call RMCreateDevice() to allocate the resources
 *  - Add as last step:
 *     - if (Warp3 or any command line overrides) and
 *     - if resources are successfully allocated
 *     - then SLAM the chip with the allocated resources
 */

LDev_Resources* ResourceManager::pGetDevResources ( DEVID DevID , ULONG ulSearchFlags, bool fPciDevice)
{
    // Initialize resource object.  Use detected information if available,
    // otherwise use hardcoded defaults.
    if(GetRMDetectedResources( DevID, ulSearchFlags, fPciDevice) == FALSE) {
//        dprintf(("error in GetRMDetectedResources1\n"));
        return NULL;
    }

    if(_pahRMAllocDetectedResources() == FALSE) {
//        dprintf(("error in _pahRMAllocDetectedResources2\n"));
        _state = rmAllocFailed;
        return NULL;
    }
    return &detectedResources;
}
//*****************************************************************************
//*****************************************************************************
BOOL ResourceManager::registerResources()
{
    APIRET rc;

    char szDeviceName[128];
    char szMixerName[64];

    if(OSS32_QueryNames(OSS32_DEFAULT_DEVICE, szDeviceName, sizeof(szDeviceName),
                        szMixerName, sizeof(szMixerName), FALSE) != OSSERR_SUCCESS)
    {
        DebugInt3();
        return FALSE;
    }

    switch(DevID) {
    case PCIID_VIA_686A:
    case PCIID_VIA_8233:
    case PCIID_SI_7012:
    case PCIID_INTEL_82801:
    case PCIID_INTEL_82901:
    case PCIID_INTEL_92801BA:
    case PCIID_INTEL_440MX:
    case PCIID_INTEL_ICH3:
    case PCIID_INTEL_ICH4:
    case PCIID_INTEL_ICH5:
    case PCIID_INTEL_ICH6:
    case PCIID_INTEL_ICH7:
    case PCIID_NVIDIA_MCP_AUDIO:
    case PCIID_NVIDIA_MCP2_AUDIO:
    case PCIID_NVIDIA_MCP3_AUDIO:
    case PCIID_NVIDIA_CK8S_AUDIO:
    case PCIID_NVIDIA_CK8_AUDIO:
        strcat(szDeviceName, " with ");
        strcat(szDeviceName, szMixerName);
        break;
    case PCIID_CREATIVELABS_SBLIVE:
    case PCIID_ALS4000:
    case PCIID_CMEDIA_CM8338A:
    case PCIID_CMEDIA_CM8338B:
    case PCIID_CMEDIA_CM8738:
    case PCIID_CMEDIA_CM8738B:
    case PCIID_CIRRUS_4281:
    case PCIID_CIRRUS_4280:
    case PCIID_CIRRUS_4612:
    case PCIID_CIRRUS_4615:
    case PCIID_ESS_ALLEGRO_1:
    case PCIID_ESS_ALLEGRO:
    case PCIID_ESS_MAESTRO3:
    case PCIID_ESS_MAESTRO3_1:
    case PCIID_ESS_MAESTRO3_HW:
    case PCIID_ESS_MAESTRO3_2:
    case PCIID_ESS_CANYON3D_2LE:
    case PCIID_ESS_CANYON3D_2:
    case PCIID_ESS_ES1938:
    case PCIID_AUREAL_VORTEX:
    case PCIID_AUREAL_VORTEX2:
    case PCIID_AUREAL_ADVANTAGE:
    case PCIID_ENSONIQ_CT5880:
    case PCIID_ENSONIQ_ES1371:
    case PCIID_YAMAHA_YMF724:
    case PCIID_YAMAHA_YMF724F:
    case PCIID_YAMAHA_YMF740:
    case PCIID_YAMAHA_YMF740C:
    case PCIID_YAMAHA_YMF744:
    case PCIID_YAMAHA_YMF754:
    case PCIID_ESS_M2E:
    case PCIID_ESS_M2:
    case PCIID_ESS_M1:
    case PCIID_ALI_5451:
    case PCIID_TRIDENT_4DWAVE_DX:
    case PCIID_TRIDENT_4DWAVE_NX:
    case PCIID_SI_7018:
    case PCIID_FM801:
    case PCIID_ATIIXP_SB200:
    case PCIID_ATIIXP_SB300:
    case PCIID_ATIIXP_SB400:
    case PCIID_AUDIGYLS:
    case PCIID_AUDIGYLS1:
    case PCIID_AUDIGYLS2:
        break;

    default:
        DebugInt3();
        return FALSE;
    }

    //--- Here, we got all the resources we wanted.  Register adapter if not yet done.
    //### Problem here with mult adpaters, would need to cross ref PnP ID's to adapters created.
    if (_state != rmAdapterCreated) {
        rc = _rmCreateAdapter();
    }
    _rmCreateDevice(szDeviceName);
    return TRUE;
}
//*****************************************************************************
//*****************************************************************************


/*
 * --- Private member functions.
 */


/**@internal _pahRMAllocDetectedResources
 *  Allocate a set of resources from OS/2 by interfacing with the
 *  OS/2 resource manager.
 * @param PRESOURCELIST pResourceList - list of resources to allocate.
 * @return TRUE  - on success
 * @return FALSE - on failure
 * @notes Logs appropriate errors in global error log if any allocation
 *  problem.
 * @notes Either all resources are allocated, or none.  If there is a
 *  failure within this function when some (but not all) resources are
 *  allocated, then any allocated resources are freed.
 */
BOOL ResourceManager::_pahRMAllocDetectedResources( )
{
    int j;

    for ( j=0; j<MAX_ISA_Dev_IO; ++j) {
        if (detectedResources.uIOBase[j] != NoIOValue) {
            if(requestIORange(detectedResources.uIOBase[j], detectedResources.uIOLength[j]) == FALSE) {
                dprintf(("error in IO range\n"));
                return FALSE;
            }
        }
    }
    for ( j=0; j<MAX_ISA_Dev_IRQ; ++j) {
        if (detectedResources.uIRQLevel[j] != NoIOValue) {
            //shared irq is not necessarily true, but let's assume that for now
            if(requestIRQ(detectedResources.uIRQLevel[j], TRUE) == FALSE) {
                dprintf(("error in IRQ\n"));
                return FALSE;
            }
        }
    }
    for ( j=0; j<MAX_ISA_Dev_DMA; ++j) {
        if (detectedResources.uDMAChannel[j] != NoIOValue) {
            if(requestDMA(detectedResources.uDMAChannel[j]) == FALSE) {
                dprintf(("error in DMA\n"));
                return FALSE;
            }
        }
    }

    for ( j=0; j<MAX_ISA_Dev_MEM; ++j) {
        if (detectedResources.uMemBase[j] != 0xffffffff) {
            if(requestMemRange(detectedResources.uMemBase[j], detectedResources.uMemLength[j]) == FALSE) {
                dprintf(("error in mem range\n"));
               return FALSE;
           }
       }
    }
    return TRUE;
}
//*****************************************************************************
//*****************************************************************************
BOOL ResourceManager::isPartOfAllocatedResources(int type, ULONG ulBase, ULONG ulLength)
{
    int j;

    switch(type) {
    case RS_TYPE_IO:
        for ( j=0; j<MAX_ISA_Dev_IO; ++j) {
            if (resources.uIOBase[j] == ulBase && ulLength == resources.uIOLength[j]) {
                return TRUE;
            }
        }
        break;
    case RS_TYPE_DMA:
        for ( j=0; j<MAX_ISA_Dev_DMA; ++j) {
            if (resources.uDMAChannel[j] == ulBase) {
                return TRUE;
            }
        }
        break;
    case RS_TYPE_MEM:
        for ( j=0; j<MAX_ISA_Dev_MEM; ++j) {
            if (ulBase >= resources.uMemBase[j] && ulBase+ulLength <= resources.uMemBase[j] + resources.uMemLength[j]) {
                return TRUE;
            }
        }
        break;
    case RS_TYPE_IRQ:
        for ( j=0; j<MAX_ISA_Dev_IRQ; ++j) {
            if (resources.uIRQLevel[j] == ulBase) {
                return TRUE;
            }
        }
        break;
    }
    return FALSE;
}
//*****************************************************************************
//*****************************************************************************
BOOL ResourceManager::requestIORange(ULONG ulBase, ULONG ulLength)
{
    RESOURCESTRUCT Resource;
    HRESOURCE      hres = 0;
    APIRET         rc;
    int            j;

    if(isPartOfAllocatedResources(RS_TYPE_IO, ulBase, ulLength)) {
        return TRUE;
    }

    memset(__Stack32ToFlat(&Resource), 0, sizeof(Resource));
    Resource.ResourceType          = RS_TYPE_IO;
    Resource.IOResource.BaseIOPort = (USHORT)ulBase;
    Resource.IOResource.NumIOPorts = (USHORT)ulLength;
    Resource.IOResource.IOFlags    = RS_IO_EXCLUSIVE;
    if (ulBase > 0x3ff)
       Resource.IOResource.IOAddressLines = 16;
    else
       Resource.IOResource.IOAddressLines = 10;

    rc = RMAllocResource( _hDriver,                       // Handle to driver.
                          FlatToSel((ULONG)&hres),           // OUT:  "allocated" resource node handle
                          FlatToSel((ULONG)&Resource) );     // Resource to allocate.

    dprintf(("RMAllocResource IO rc = %d\n", rc));
    if (rc == RMRC_SUCCESS || rc == RMRC_RES_ALREADY_CLAIMED || rc == 14) {
        //insert into allocated resources array
        for ( j=0; j<MAX_ISA_Dev_IO; ++j) {
            if (resources.uIOBase[j] == NoIOValue) {
                resources.uIOBase[j]   = (USHORT)ulBase;
                resources.uIOLength[j] = (USHORT)ulLength;
                break;
            }
        }
        if(j != MAX_ISA_Dev_IO) {
            hResource[idxRes] = hres;
            idxRes++;
            return TRUE;
        }
        DebugInt3();
    }
    releaseAllResources();
    return FALSE;
}
//*****************************************************************************
//*****************************************************************************
BOOL ResourceManager::requestMemRange(ULONG ulBase, ULONG ulLength)
{
    RESOURCESTRUCT Resource;
    HRESOURCE      hres = 0;
    APIRET         rc;
    int            j;

    if(isPartOfAllocatedResources(RS_TYPE_MEM, ulBase, ulLength)) {
        return TRUE;
    }
    memset(__Stack32ToFlat(&Resource), 0, sizeof(Resource));
    Resource.ResourceType        = RS_TYPE_MEM;
    Resource.MEMResource.MemBase = ulBase;
    Resource.MEMResource.MemSize = ulLength;
    Resource.MEMResource.MemFlags= RS_MEM_EXCLUSIVE;

    rc = RMAllocResource( _hDriver,                       // Handle to driver.
                          FlatToSel((ULONG)&hres),           // OUT:  "allocated" resource node handle
                          FlatToSel((ULONG)&Resource) );     // Resource to allocate.

    dprintf(("RMAllocResource MEM rc = %d\n", rc));
    if (rc == RMRC_SUCCESS || rc == RMRC_RES_ALREADY_CLAIMED) {
        //insert into allocated resources array
        for ( j=0; j<MAX_ISA_Dev_MEM; ++j) {
            if (resources.uMemBase[j] == 0xffffffff) {
                resources.uMemBase[j]   = ulBase;
                resources.uMemLength[j] = ulLength;
                break;
            }
        }
        if(j != MAX_ISA_Dev_MEM) {
            hResource[idxRes] = hres;
            idxRes++;
            return TRUE;
        }
        DebugInt3();
    }
    releaseAllResources();
    return FALSE;
}
//*****************************************************************************
//*****************************************************************************
BOOL ResourceManager::requestIRQ(USHORT usIRQ, BOOL fShared)
{
    RESOURCESTRUCT Resource;
    HRESOURCE      hres = 0;
    APIRET         rc;
    int            j;

    if(isPartOfAllocatedResources(RS_TYPE_IRQ, usIRQ, 0)) {
        return TRUE;
    }
    memset(__Stack32ToFlat(&Resource), 0, sizeof(Resource));
    Resource.ResourceType          = RS_TYPE_IRQ;
    Resource.IRQResource.IRQLevel  = usIRQ;
    Resource.IRQResource.IRQFlags  = (fShared) ? RS_IRQ_SHARED : RS_IRQ_EXCLUSIVE;
    Resource.IRQResource.PCIIrqPin = RS_PCI_INT_NONE;

    rc = RMAllocResource( _hDriver,                       // Handle to driver.
                          FlatToSel((ULONG)&hres),           // OUT:  "allocated" resource node handle
                          FlatToSel((ULONG)&Resource) );     // Resource to allocate.

    dprintf(("RMAllocResource IRQ rc = %d\n", rc));
    if (rc == RMRC_SUCCESS || rc == RMRC_RES_ALREADY_CLAIMED) {
        //insert into allocated resources array
        for ( j=0; j<MAX_ISA_Dev_IRQ; ++j) {
            if (resources.uIRQLevel[j] == NoIOValue) {
                resources.uIRQLevel[j] = usIRQ;
                break;
            }
        }
        if(j != MAX_ISA_Dev_IRQ) {
            hResource[idxRes] = hres;
            idxRes++;
            return TRUE;
        }
        DebugInt3();
    }
    releaseAllResources();
    return FALSE;
}
//*****************************************************************************
//*****************************************************************************
BOOL ResourceManager::requestDMA(USHORT usDMA)
{
    RESOURCESTRUCT Resource;
    HRESOURCE      hres = 0;
    APIRET         rc;
    int            j;

    if(isPartOfAllocatedResources(RS_TYPE_DMA, usDMA, 0)) {
        return TRUE;
    }
    memset(__Stack32ToFlat(&Resource), 0, sizeof(Resource));
    Resource.ResourceType           = RS_TYPE_DMA;
    Resource.DMAResource.DMAChannel = usDMA;
    Resource.DMAResource.DMAFlags   = RS_DMA_EXCLUSIVE;

    rc = RMAllocResource( _hDriver,                       // Handle to driver.
                          FlatToSel((ULONG)&hres),           // OUT:  "allocated" resource node handle
                          FlatToSel((ULONG)&Resource) );     // Resource to allocate.

    if (rc == RMRC_SUCCESS) {
        //insert into allocated resources array
        for ( j=0; j<MAX_ISA_Dev_DMA; ++j) {
            if (resources.uDMAChannel[j] == NoIOValue) {
                resources.uDMAChannel[j] = usDMA;
                break;
            }
        }
        if(j != MAX_ISA_Dev_DMA) {
            hResource[idxRes] = hres;
            idxRes++;
            return TRUE;
        }
        DebugInt3();
    }
    releaseAllResources();
    return FALSE;
}
//*****************************************************************************
//*****************************************************************************
void ResourceManager::releaseAllResources()
{
    int i;

    for(i=0;i<idxRes;i++) {
        RMDeallocResource(_hDriver, hResource[i]);
    }
    idxRes = 0;
}
//*****************************************************************************
//*****************************************************************************
/**@internal _rmCreateAdapter
 *  Create the "adapter" node.  The "adapter" node belongs to this driver's
 *  "driver" node.  Also as part of this operation, the "resource" nodes
 *  associated with this driver will be moved to the "adapter" node.
 * @param None.
 * @notes Changes state of the RM object to 'rmAdapterCreated'.
 * @return APIRET rc - 0 iff good creation.  Returns non-zero and logs a soft
 *  error on failure.
 */
APIRET ResourceManager::_rmCreateAdapter()
{
   APIRET rc = 0;
   ADAPTERSTRUCT AdapterStruct;
   char AdapterName[sizeof(RM_ADAPTER_NAME)];
   HADAPTER hAdapter;

   if (!_hAdapter && _state != rmAdapterCreated)
   {
      //copy string to stack, because we need to give RM 16:16 pointers
      //(which can only be (easily) generated from 32 bits stack addresses)
      strcpy(AdapterName, RM_ADAPTER_NAME);

      memset( (PVOID) &AdapterStruct, 0, sizeof(AdapterStruct) );
      AdapterStruct.AdaptDescriptName = FlatToSel((ULONG)AdapterName);        /* ### IHV */
      AdapterStruct.AdaptFlags        = AS_16MB_ADDRESS_LIMIT;    // AdaptFlags         /* ### IHV */
      AdapterStruct.BaseType          = AS_BASE_MMEDIA;           // BaseType
      AdapterStruct.SubType           = AS_SUB_MM_AUDIO;          // SubType
      AdapterStruct.InterfaceType     = AS_INTF_GENERIC;          // InterfaceType
      AdapterStruct.HostBusType       = AS_HOSTBUS_PCI;           // HostBusType        /* ### IHV */
      AdapterStruct.HostBusWidth      = AS_BUSWIDTH_32BIT;        // HostBusWidth       /* ### IHV */
      AdapterStruct.pAdjunctList      = NULL;                     // pAdjunctList       /* ### IHV */

      //--- Register adapter.  We'll record any error code, but won't fail
      // the driver initialization and won't return resources.
      //NOTE: hAdapter must be used as FlatToSel only works for stack variables
      rc = RMCreateAdapter( _hDriver,          // Handle to driver
                            FlatToSel((ULONG)&hAdapter),        // (OUT) Handle to adapter
                            FlatToSel((ULONG)&AdapterStruct),    // Adapter structure
                            NULL,              // Parent device (defaults OK)
                            NULL );            // Allocated resources.  We assign ownership
                                               // of the IO, IRQ, etc. resources to the
                                               // device (via RMCreateDevice()), not to the
                                               // adapter (as done in disk DD sample).
      if (rc == RMRC_SUCCESS) {
         _state = rmAdapterCreated;
         _hAdapter = hAdapter;
      }
   }
   else {
      //Only create one RM Adapter object
      _state = rmAdapterCreated;
   }
   return rc;
}

/**@internal _rmCreateDevice
 *  Create Device node in the OS/2 RM allocation tree.  Device nodes belong
 *  to the Adapter node, just like the Resource nodes.
 * @param PSZ pszName - Descriptive name of device.
 * @param LPAHRESOURCE pahResource - Handles of allocated resources that are
 *  owned by this device.
 * @return APIRET rc - Value returned by RMCreateDevice() call.
 * @notes Same "soft" error strategy as adapter registration: we'll record
 *  any errors but hold onto the resources and continue to operate the
 *  driver.
 * @notes
 */
APIRET ResourceManager::_rmCreateDevice( char *lpszDeviceName)
{
    DEVICESTRUCT DeviceStruct;
    HDEVICE      hDevice;
    APIRET       rc;
    typedef struct _myhresource {
        ULONG     NumResource;
        HRESOURCE hResource[MAX_ResourceCount];      /*First Entry in Array of HRESOURCE */
    } MYAHRESOURCE;

    MYAHRESOURCE hres;

    hres.NumResource = idxRes;
    for(int i=0;i<idxRes;i++) {
        hres.hResource[i] = hResource[i];
    }

    memset( (PVOID) &DeviceStruct, 0, sizeof(DeviceStruct));

    //NOTE: Assumes szDeviceName is a stack pointer!!
    DeviceStruct.DevDescriptName = FlatToSel((ULONG)lpszDeviceName);
    DeviceStruct.DevFlags        = DS_FIXED_LOGICALNAME;
    DeviceStruct.DevType         = DS_TYPE_AUDIO;
    DeviceStruct.pAdjunctList    = NULL;

    rc = RMCreateDevice(_hDriver,      // Handle to driver
                        FlatToSel((ULONG)&hDevice),      // (OUT) Handle to device, unused.
                        FlatToSel((ULONG)&DeviceStruct), // Device structure
                        _hAdapter,     // Parent adapter
                        FlatToSel((ULONG)&hres)); // Allocated resources
    return rc;
}
//******************************************************************************
//******************************************************************************
int ResourceManager::getPCIConfiguration(ULONG pciId, int idx)
{
    ULONG devNr, busNr, funcNr, temp, cfgaddrreg, detectedId;
    int found = 0;

    cfgaddrreg = _inpd(PCI_CONFIG_ADDRESS);
    for(busNr=0;busNr<MAX_PCI_BUSSES;busNr++)     //BusNumber<255
    {
        for(devNr=0;devNr<32;devNr++)
        {
            for(funcNr=0;funcNr<8;funcNr++)
            {
                temp = ((ULONG)((ULONG)devNr<<11UL) + ((ULONG)busNr<<16UL) + ((ULONG)funcNr << 8UL));

                _outpd(PCI_CONFIG_ADDRESS, PCI_CONFIG_ENABLE|temp);
                detectedId = _inpd(PCI_CONFIG_DATA);
                if(detectedId == pciId)
                {
                    found++;
                    if (found == (idx+1))
                        break;
                }
            }
            if (found == (idx+1)) break;
//            if(found) break;
        }
        if (found == (idx+1)) break;
//        if(found) break;
    }

    if(!found) {
        _outpd(PCI_CONFIG_ADDRESS, cfgaddrreg);
        return 0;
    }

    for(int i=0;i<64;i++)
    {
        temp = ((ULONG)((ULONG)devNr<<11UL) + ((ULONG)busNr<<16UL) + ((ULONG)funcNr << 8UL) + (i << 2));
        _outpd(PCI_CONFIG_ADDRESS, PCI_CONFIG_ENABLE|temp);

        PCIConfig[i] = _inpd(PCI_CONFIG_DATA);
    }
    _outpd(PCI_CONFIG_ADDRESS, cfgaddrreg);

    pciConfigData = (PCIConfigData *)&PCIConfig[0];

    if(pciConfigData->Bar[0] == 0 || pciConfigData->Bar[0] == 0xFFFFFFFF)
    {
        dprintf(("WARNING: No BAR address for PCI device!!"));
    }
    busnr  = busNr  & 0xFF;
    devnr  = devNr  & 0x1F;
    funcnr = funcNr & 0x7;
    devfn  = (devnr << 3) | funcnr;
    printk("found %i devices for %X PCI ID\n", found, pciId);
    return found;
}
//******************************************************************************
//******************************************************************************
void ResourceManager::pci_write_config_dword(ULONG where, ULONG value)
{
    _outpd(PCI_CONFIG_ADDRESS, CONFIG_CMD(busnr,devfn, where));
    _outpd(PCI_CONFIG_DATA, value);
}
//******************************************************************************
//******************************************************************************
void ResourceManager::pci_read_config_dword(ULONG where, ULONG *pValue)
{
    _outpd(PCI_CONFIG_ADDRESS, CONFIG_CMD(busnr,devfn, where));
    *pValue = _inpd(PCI_CONFIG_DATA);
}
//******************************************************************************
//******************************************************************************
HDRIVER  ResourceManager::_hDriver = 0;
HADAPTER ResourceManager::_hAdapter= 0;
//******************************************************************************
//******************************************************************************
HRESMGR RMFindPCIDevice(ULONG vendorid, ULONG deviceid, IDC_RESOURCE *lpResource, int idx)
{
    LDev_Resources* pRMResources;
    ResourceManager* pRM = NULL;               // Resource manager object.
    int i;
    ULONG pcidevid;

    pcidevid = (deviceid << 16) | vendorid;

    pRM = new ResourceManager();        // Create the RM object.
    if (!pRM) {
        goto fail;
    }
    if(pRM->getState() != rmDriverCreated) {
        goto fail;
    }

    if(!pRM->bIsDevDetected(pcidevid, SEARCH_ID_DEVICEID, TRUE, idx)) {
        dprintf(("Error in bIsDevDetected"));
        goto fail;
    }

    pRMResources = pRM->pGetDevResources(pcidevid, SEARCH_ID_DEVICEID, TRUE);
    if (!pRMResources) {
        dprintf(("Error in pGetDevResources"));
        goto fail;
    }
    lpResource->busnr  = pRM->getBusNr();
    lpResource->devnr  = pRM->getDeviceNr();
    lpResource->funcnr = pRM->getFunctionNr();
    lpResource->devfn  = pRM->getDevFuncNr();
    dprintf(("Detected device %x%x bus %d dev %d func %d", vendorid, deviceid, lpResource->busnr, lpResource->devnr, lpResource->funcnr));

    // Available device resources identified
    for(i=0;i<MAX_ISA_Dev_IO;i++) {
        lpResource->io[i] = pRMResources->uIOBase[i];
        lpResource->iolength[i] = pRMResources->uIOLength[i];
        if(lpResource->io[i] != 0xffff)
            dprintf(("IO resource %x length %d", (ULONG)lpResource->io[i], (ULONG)lpResource->iolength[i]));
    }
    for(i=0;i<MAX_ISA_Dev_IRQ;i++) {
        lpResource->irq[i] = pRMResources->uIRQLevel[i];
        if(lpResource->irq[i] != 0xffff)
            dprintf(("IRQ resource %d ", (ULONG)lpResource->irq[i]));
    }
    for(i=0;i<MAX_ISA_Dev_DMA;i++) {
        lpResource->dma[i] = pRMResources->uDMAChannel[i];
    }
    for(i=0;i<MAX_ISA_Dev_MEM;i++) {
        lpResource->mem[i] = pRMResources->uMemBase[i];
        lpResource->memlength[i] = pRMResources->uMemLength[i];
        if(lpResource->mem[i] != 0xffffffff)
            dprintf(("Memory resource %x length %d", (ULONG)lpResource->mem[i], (ULONG)lpResource->memlength[i]));
    }

    return (HRESMGR)pRM;

fail:
    if(pRM) delete pRM;
    return 0;
}
//******************************************************************************
//register resources & destroy resource manager object
//******************************************************************************
void RMFinialize(HRESMGR hResMgr)
{
    if(hResMgr) {
        ResourceManager* pRM = (ResourceManager*)hResMgr;
        pRM->registerResources();
        delete pRM;
    }
}
//******************************************************************************
//destroy resource manager object
//******************************************************************************
void RMDestroy(HRESMGR hResMgr)
{
    if(hResMgr) {
        ResourceManager* pRM = (ResourceManager*)hResMgr;
        delete pRM;
    }
}
//******************************************************************************
//******************************************************************************
BOOL RMRequestIO(HRESMGR hResMgr, ULONG ulIOBase, ULONG ulIOLength)
{
    ResourceManager* pRM = (ResourceManager*)hResMgr;

    if(!pRM) {
        DebugInt3();
        return FALSE;
    }
    return pRM->requestIORange(ulIOBase, ulIOLength);
}
//******************************************************************************
//******************************************************************************
BOOL RMRequestMem(HRESMGR hResMgr, ULONG ulMemBase, ULONG ulMemLength)
{
    ResourceManager* pRM = (ResourceManager*)hResMgr;

    if(!pRM) {
        DebugInt3();
        return FALSE;
    }
    return pRM->requestMemRange(ulMemBase, ulMemLength);
}
//******************************************************************************
//******************************************************************************
BOOL RMRequestIRQ(HRESMGR hResMgr, ULONG ulIrq, BOOL fShared)
{
    ResourceManager* pRM = (ResourceManager*)hResMgr;

    if(!pRM) {
        DebugInt3();
        return FALSE;
    }
    return pRM->requestIRQ((USHORT)ulIrq, fShared);
}
//******************************************************************************
//******************************************************************************
BOOL RMRequestDMA(HRESMGR hResMgr, ULONG ulDMA)
{
    ResourceManager* pRM = (ResourceManager*)hResMgr;

    if(!pRM) {
        DebugInt3();
        return FALSE;
    }
    return pRM->requestDMA((USHORT)ulDMA);
}
//******************************************************************************
//******************************************************************************

