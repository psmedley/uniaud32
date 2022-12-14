/* $Id: pciids.h,v 1.2 2003/07/21 18:35:39 vladest Exp $ */
/*
 * PCI ID definitions for the supported chipsets
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

#ifndef __PCIIDS_H__
#define __PCIIDS_H__

#define PCI_VENDOR_ID_CMEDIA            0x13F6
#define PCI_DEVICE_ID_CMEDIA_CM8338A    0x0100
#define PCI_DEVICE_ID_CMEDIA_CM8338B    0x0101
#define PCI_DEVICE_ID_CMEDIA_CM8738     0x0111
#define PCI_DEVICE_ID_CMEDIA_CM8738B    0x0112

#define PCI_VENDOR_ID_INTEL             0x8086
#define PCI_DEVICE_ID_INTEL_82801       0x2415
#define PCI_DEVICE_ID_INTEL_82901       0x2425
#define PCI_DEVICE_ID_INTEL_82801BA     0x2445
#define PCI_DEVICE_ID_INTEL_440MX       0x7195
#define PCI_DEVICE_ID_INTEL_ICH3        0x2485
#define PCI_DEVICE_ID_INTEL_ICH4        0x24C5
#define PCI_DEVICE_ID_INTEL_ICH5        0x24D5
#define PCI_DEVICE_ID_INTEL_ICH6        0x2668 /* ICH6 */
#define PCI_DEVICE_ID_INTEL_ICH7        0x27d8 /* ICH7 */

#define PCI_VENDOR_ID_CIRRUS            0x1013
#define PCI_DEVICE_ID_CIRRUS_4281       0x6005
#define PCI_DEVICE_ID_CIRRUS_4280       0x6001
#define PCI_DEVICE_ID_CIRRUS_4612       0x6003
#define PCI_DEVICE_ID_CIRRUS_4615       0x6004

#ifndef PCI_VENDOR_ID_ESS
#define PCI_VENDOR_ID_ESS               0x125D
#endif
#define PCI_DEVICE_ID_ESS_ALLEGRO_1	    0x1988
#define PCI_DEVICE_ID_ESS_ALLEGRO	    0x1989
#define PCI_DEVICE_ID_ESS_CANYON3D_2LE	0x1990
#define PCI_DEVICE_ID_ESS_CANYON3D_2	0x1992
#define PCI_DEVICE_ID_ESS_MAESTRO3	    0x1998
#define PCI_DEVICE_ID_ESS_MAESTRO3_1	0x1999
#define PCI_DEVICE_ID_ESS_MAESTRO3_HW	0x199a
#define PCI_DEVICE_ID_ESS_MAESTRO3_2	0x199b
#define PCI_DEVICE_ID_ESS_ES1938        0x1969


#define PCIID_CREATIVELABS_SBLIVE  0x00021102
#define PCIID_VIA_686A             0x30581106
#define PCIID_VIA_8233             0x30591106
#define PCIID_ALS4000              0x40004005
#define PCIID_CMEDIA_CM8338A       ((PCI_DEVICE_ID_CMEDIA_CM8338A<<16) | PCI_VENDOR_ID_CMEDIA)
#define PCIID_CMEDIA_CM8338B       ((PCI_DEVICE_ID_CMEDIA_CM8338B<<16) | PCI_VENDOR_ID_CMEDIA)
#define PCIID_CMEDIA_CM8738        ((PCI_DEVICE_ID_CMEDIA_CM8738<<16)  | PCI_VENDOR_ID_CMEDIA)
#define PCIID_CMEDIA_CM8738B       ((PCI_DEVICE_ID_CMEDIA_CM8738B<<16) | PCI_VENDOR_ID_CMEDIA)
#define PCIID_INTEL_82801          ((PCI_DEVICE_ID_INTEL_82801<<16)    | PCI_VENDOR_ID_INTEL)
#define PCIID_INTEL_82901          ((PCI_DEVICE_ID_INTEL_82901<<16)    | PCI_VENDOR_ID_INTEL)
#define PCIID_INTEL_92801BA        ((PCI_DEVICE_ID_INTEL_82801BA<<16)  | PCI_VENDOR_ID_INTEL)
#define PCIID_INTEL_440MX          ((PCI_DEVICE_ID_INTEL_440MX<<16)    | PCI_VENDOR_ID_INTEL)
#define PCIID_INTEL_ICH3           ((PCI_DEVICE_ID_INTEL_ICH3<<16)     | PCI_VENDOR_ID_INTEL)
#define PCIID_INTEL_ICH4           ((PCI_DEVICE_ID_INTEL_ICH4<<16)     | PCI_VENDOR_ID_INTEL)
#define PCIID_INTEL_ICH5           ((PCI_DEVICE_ID_INTEL_ICH5<<16)     | PCI_VENDOR_ID_INTEL)
#define PCIID_INTEL_ICH6           ((PCI_DEVICE_ID_INTEL_ICH6<<16)     | PCI_VENDOR_ID_INTEL)
#define PCIID_INTEL_ICH7           ((PCI_DEVICE_ID_INTEL_ICH7<<16)     | PCI_VENDOR_ID_INTEL)
#define PCIID_SI_7012		   0x70121039

#define PCIID_NVIDIA_MCP_AUDIO	   0x01b110de
#define PCIID_NVIDIA_MCP2_AUDIO	   0x006a10de
#define PCIID_NVIDIA_MCP3_AUDIO	   0x00da10de
#define PCIID_NVIDIA_CK8S_AUDIO    0x00ea10de
#define PCIID_NVIDIA_CK8_AUDIO     0x008a10de

#define PCIID_CIRRUS_4281          ((PCI_DEVICE_ID_CIRRUS_4281<<16)    | PCI_VENDOR_ID_CIRRUS)
#define PCIID_CIRRUS_4280          ((PCI_DEVICE_ID_CIRRUS_4280<<16)    | PCI_VENDOR_ID_CIRRUS)
#define PCIID_CIRRUS_4612          ((PCI_DEVICE_ID_CIRRUS_4612<<16)    | PCI_VENDOR_ID_CIRRUS)
#define PCIID_CIRRUS_4615          ((PCI_DEVICE_ID_CIRRUS_4615<<16)    | PCI_VENDOR_ID_CIRRUS)

#define PCIID_ESS_ALLEGRO_1        ((PCI_DEVICE_ID_ESS_ALLEGRO_1<<16)  | PCI_VENDOR_ID_ESS)
#define PCIID_ESS_ALLEGRO          ((PCI_DEVICE_ID_ESS_ALLEGRO<<16)    | PCI_VENDOR_ID_ESS)
#define PCIID_ESS_MAESTRO3         ((PCI_DEVICE_ID_ESS_MAESTRO3<<16)   | PCI_VENDOR_ID_ESS)
#define PCIID_ESS_MAESTRO3_1       ((PCI_DEVICE_ID_ESS_MAESTRO3_1<<16) | PCI_VENDOR_ID_ESS)
#define PCIID_ESS_MAESTRO3_HW      ((PCI_DEVICE_ID_ESS_MAESTRO3_HW<<16)| PCI_VENDOR_ID_ESS)
#define PCIID_ESS_MAESTRO3_2       ((PCI_DEVICE_ID_ESS_MAESTRO3_2<<16) | PCI_VENDOR_ID_ESS)
#define PCIID_ESS_CANYON3D_2LE      ((PCI_DEVICE_ID_ESS_CANYON3D_2LE<<16)| PCI_VENDOR_ID_ESS)
#define PCIID_ESS_CANYON3D_2       ((PCI_DEVICE_ID_ESS_CANYON3D_2<<16) | PCI_VENDOR_ID_ESS)
#define PCIID_ESS_ES1938           ((PCI_DEVICE_ID_ESS_ES1938<<16)     | PCI_VENDOR_ID_ESS)

#define	PCIID_AUREAL_VORTEX        0x000112EB
#define	PCIID_AUREAL_VORTEX2       0x000212EB
#define	PCIID_AUREAL_ADVANTAGE     0x000312EB

#define PCIID_ENSONIQ_CT5880       0x58801274
#define PCIID_ENSONIQ_ES1371	   0x13711274

#define PCIID_ENSONIQ_ES1371	   0x13711274
#define PCIID_YAMAHA_YMF724        0x00041073
#define PCIID_YAMAHA_YMF724F       0x000d1073
#define PCIID_YAMAHA_YMF740        0x000a1073
#define PCIID_YAMAHA_YMF740C       0x000c1073
#define PCIID_YAMAHA_YMF744        0x00101073
#define PCIID_YAMAHA_YMF754        0x00121073

#define PCIID_ESS_M2E              0x1978125D
#define PCIID_ESS_M2	           0x1968125D
#define PCIID_ESS_M1	           0x0100125D

#define PCI_VENDOR_ID_ALI	   0x10b9

#define PCIID_ALI_5451	           0x545110b9

#define PCIID_TRIDENT_4DWAVE_DX	0x20001023
#define PCIID_TRIDENT_4DWAVE_NX	0x20011023

#define PCIID_SI_7018		0x70181039

#define PCIID_FM801             0x08011319
#define PCIID_ATIIXP_SB200      0x43411002 /* SB200 */
#define PCIID_ATIIXP_SB300      0x43611002 /* SB300 */
#define PCIID_ATIIXP_SB400      0x43701002 /* SB400 */

#define PCIID_AUDIGYLS  0x10021102
#define PCIID_AUDIGYLS1 0x10061102
#define PCIID_AUDIGYLS2 0x10071102

#endif //__PCIIDS_H__
