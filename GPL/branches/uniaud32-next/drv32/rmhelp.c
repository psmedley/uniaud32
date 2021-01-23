/*
 * Resource manager helper functions
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#define INCL_NOPMAPI
#define INCL_DOSINFOSEG
#include <os2.h>

#include <devhelp.h>
#include <devinfo.h>

#include <rmbase.h>		// Resource manager definitions.
#include "rmcalls.h"
#include <rmioctl.h>

#include <version.h>
#include <dbgos2.h>
#include <unicard.h>
#include <osspci.h>
#include <ossidc32.h>
#include "pciids.h"

#include <string.h>

#define	MAX_RESHANDLES	16

static HDRIVER  DriverHandle  = (HDRIVER)-1;
static HADAPTER	AdapterHandle = 0;

typedef struct
{
    ULONG     NumResource;
    HRESOURCE hResource[MAX_RESHANDLES];
} RESOURCES;

static RESOURCES Resource_stt = {0};

//******************************************************************************
//******************************************************************************
VOID RMCreateDriverU32(VOID)
{
    HDRIVER      hDriver;
    DRIVERSTRUCT DriverStruct;
    char         DriverName[sizeof(RM_DRIVER_NAME)];
    char         VendorName[sizeof(RM_DRIVER_VENDORNAME)];
    char         DriverDesc[sizeof(RM_DRIVER_DESCRIPTION)];

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
    DriverStruct.Date.Year    = RM_DRIVER_BUILDYEAR;                 /* ### IHV */
    DriverStruct.Date.Month   = RM_DRIVER_BUILDMONTH;                /* ### IHV */
    DriverStruct.Date.Day     = RM_DRIVER_BUILDDAY;                  /* ### IHV */
    DriverStruct.DrvrFlags    = 0;
    DriverStruct.DrvrType     = DRT_AUDIO;
    DriverStruct.DrvrSubType  = 0;
    DriverStruct.DrvrCallback = 0;

    APIRET rc = RMCreateDriver( FlatToSel((ULONG)&DriverStruct),
                                FlatToSel((ULONG)&hDriver) );

    if( rc == RMRC_SUCCESS )
    {
        DriverHandle = hDriver;
    }
    dprintf(("RMCreateDriver rc=%d DriverHandle=%x", rc, DriverHandle));
}

//******************************************************************************
//******************************************************************************
VOID RMCreateAdapterU32(ULONG DevID, ULONG *phAdapter, USHORT BusDevFunc, ULONG CardNum)
{
    APIRET rc;
    char   szAdapterName[128];
    char   szMixerName[64];

    szAdapterName[0] = szMixerName[0] = '\0';

    if(OSS32_QueryNames(CardNum, szAdapterName, sizeof(szAdapterName),
                        szMixerName, sizeof(szMixerName), FALSE) != OSSERR_SUCCESS )
    {
        return; // error
    }

    switch(DevID)
    {
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
            strcat(szAdapterName, " with ");
            strcat(szAdapterName, szMixerName);
            break;
/*
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
*/
    }

    if( !szAdapterName[0] )
    {
        strcpy(szAdapterName, "Unknown");
    }

    //copy to stack, because we need to give RM 16:16 pointers
    //(which can only be (easily) generated from 32 bits stack addresses)
    RESOURCES Resource_loc;
    // is any resoures detected and registered in RM ?
    if( Resource_stt.NumResource )
    {
        memcpy( &Resource_loc, &Resource_stt, sizeof(Resource_loc) );
    }
    else
    {
        dprintf(("No resources allocated !!!"));
        Resource_loc.NumResource = 0;
    }

    ADJUNCT adjBusDevFunc;
    adjBusDevFunc.pNextAdj    = NULL;
    adjBusDevFunc.AdjLength   = sizeof(adjBusDevFunc);
    adjBusDevFunc.AdjType     = ADJ_PCI_DEVFUNC;
    adjBusDevFunc.PCI_DevFunc = BusDevFunc;

    ADAPTERSTRUCT AdapterStruct;
    AdapterStruct.AdaptDescriptName = FlatToSel((ULONG)szAdapterName);                      /* ### IHV */
    AdapterStruct.AdaptFlags        = AS_16MB_ADDRESS_LIMIT;           // AdaptFlags         /* ### IHV */
    AdapterStruct.BaseType          = AS_BASE_MMEDIA;                  // BaseType
    AdapterStruct.SubType           = AS_SUB_MM_AUDIO;                 // SubType
    AdapterStruct.InterfaceType     = AS_INTF_GENERIC;                 // InterfaceType
    AdapterStruct.HostBusType       = AS_HOSTBUS_PCI;                  // HostBusType        /* ### IHV */
    AdapterStruct.HostBusWidth      = AS_BUSWIDTH_32BIT;               // HostBusWidth       /* ### IHV */
    AdapterStruct.pAdjunctList      = FlatToSel((ULONG)&adjBusDevFunc);// pAdjunctList       /* ### IHV */
    AdapterStruct.Reserved          = 0;

    //--- Register adapter.  We'll record any error code, but won't fail
    // the driver initialization and won't return resources.
    //NOTE: hAdapter must be used as FlatToSel only works for stack variables
    HADAPTER hAdapter;
    rc = RMCreateAdapter(DriverHandle,                     // Handle to driver
                         FlatToSel((ULONG)&hAdapter),      // (OUT) Handle to adapter
                         FlatToSel((ULONG)&AdapterStruct), // Adapter structure
                         0,                                // Parent device (defaults OK)
                         Resource_loc.NumResource ? (FlatToSel((ULONG)&Resource_loc)) : 0); // Allocated resources.

    dprintf(("RMCreateAdapter rc=%d", rc));

    if( rc == 0 )
    {
        AdapterHandle = hAdapter;
        *phAdapter    = hAdapter;
        Resource_stt.NumResource = 0;  // no resource handles to be freed
    }
    else
    {
        RMDeallocRes();
    }
}

//******************************************************************************
//******************************************************************************
BOOL RMRequestIO(ULONG ulIOBase, ULONG ulIOLength)
{
  RESOURCESTRUCT	Resource;
  HRESOURCE		hres;
  APIRET		rc;

  Resource.ResourceType              = RS_TYPE_IO;
  Resource.IOResource.BaseIOPort     = (USHORT)ulIOBase;
  Resource.IOResource.NumIOPorts     = (USHORT)ulIOLength;
  Resource.IOResource.IOFlags        = RS_IO_EXCLUSIVE;
  Resource.IOResource.IOAddressLines = ( ulIOBase > 0x3ff ) ?  16 : 10;

  rc = RMAllocResource(DriverHandle,			// Handle to driver.
		       FlatToSel((ULONG)&hres),		// OUT:  "allocated" resource node handle
		       FlatToSel((ULONG)&Resource));	// Resource to allocate.

  if( rc == 0 && Resource_stt.NumResource < MAX_RESHANDLES )
  {
    Resource_stt.hResource[Resource_stt.NumResource++] = hres;	return TRUE;
  }

  dprintf(("RMAllocResource[%d] IO rc = %d", Resource_stt.NumResource, rc));

  return FALSE;
}


//******************************************************************************
//******************************************************************************
BOOL RMRequestMem(ULONG ulMemBase, ULONG ulMemLength)
{
  RESOURCESTRUCT	Resource;
  HRESOURCE		hres;
  APIRET		rc;

  Resource.ResourceType         = RS_TYPE_MEM;
  Resource.MEMResource.MemBase  = ulMemBase;
  Resource.MEMResource.MemSize  = ulMemLength;
  Resource.MEMResource.MemFlags = RS_MEM_EXCLUSIVE;

  rc = RMAllocResource(DriverHandle,			// Handle to driver.
		       FlatToSel((ULONG)&hres),		// OUT:  "allocated" resource node handle
		       FlatToSel((ULONG)&Resource));	// Resource to allocate.

  if( rc == 0 && Resource_stt.NumResource < MAX_RESHANDLES )
  {
    Resource_stt.hResource[Resource_stt.NumResource++] = hres;	return TRUE;
  }

  dprintf(("RMAllocResource[%d] MEM rc = %d", Resource_stt.NumResource, rc));

  return rc == 0;
}


//******************************************************************************
//******************************************************************************
BOOL RMRequestIRQ(ULONG ulIrq, BOOL fShared, PHRESOURCE phRes)
{
    RESOURCESTRUCT Resource;
    HRESOURCE      hRes;
    APIRET         rc;

    memset( &Resource, 0, sizeof(Resource) );
    Resource.ResourceType          = RS_TYPE_IRQ;
    Resource.IRQResource.IRQLevel  = (USHORT)ulIrq & 0xff;
    Resource.IRQResource.PCIIrqPin = 0;
    Resource.IRQResource.IRQFlags  = ( fShared ) ? RS_IRQ_SHARED : RS_IRQ_EXCLUSIVE;

    rc = RMAllocResource(DriverHandle,                 // Handle to driver.
                         FlatToSel((ULONG)&hRes),      // OUT:  "allocated" resource node handle
                         FlatToSel((ULONG)&Resource)); // Resource to allocate.

    if (rc == 0)
    {
        *phRes = hRes;
        if (AdapterHandle)
        {
            dprintf(("RMRequestIRQ: DriverHandle=%x AdapterHandle=%x hRes=%x", DriverHandle, AdapterHandle, hRes));
            rc = RMModifyResources(DriverHandle, AdapterHandle, RM_MODIFY_ADD, hRes);
        }
        else
        {
            if (Resource_stt.NumResource < MAX_RESHANDLES )
            {
                Resource_stt.hResource[Resource_stt.NumResource++] = hRes;
            }
	}
    }
    dprintf(("RMAllocResource[%d] IRQ=%d rc=%d", Resource_stt.NumResource, ulIrq, rc));
    return rc == 0;
}

//******************************************************************************
//******************************************************************************
BOOL RMDeallocateIRQ(HRESOURCE hRes)
{
	APIRET rc;

	dprintf(("RMDeallocateIRQ: DriverHandle=%x AdapterHandle=%x hRes=%x", DriverHandle, AdapterHandle, hRes));
	rc = RMModifyResources(DriverHandle, AdapterHandle, RM_MODIFY_DELETE, hRes);

	return rc == 0;
}

//******************************************************************************
//******************************************************************************
VOID RMDeallocRes(VOID)
{
    // free resource handles
    while( Resource_stt.NumResource )
    {
        RMDeallocResource(DriverHandle, Resource_stt.hResource[--Resource_stt.NumResource]);
    }
}

/* DAZ - dirty hack so that resource manager is updated correctly
 * when using APIC and multiple adapters */
VOID RMSetHandles(HADAPTER hAdapter)
{
	AdapterHandle = hAdapter;
}
