/* $Id: dispatch.c,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * Strategy handlers for IOCtl and close
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
#define INCL_DOSMISC
#include <os2.h>

#include <dbgos2.h>
#include <devhelp.h>
#include <devtype.h>
#include <devrp.h>
#include "devown.h"
#include <ossidc.h>
#include <ossidc32.h>
#include <version.h>

extern "C" int cdecl printk(const char * fmt, ...);
//******************************************************************************
// Dispatch IOCtl requests received from the Strategy routine
//******************************************************************************
extern "C" int GetUniaudCardInfo(ULONG deviceid, void *info);
extern "C" int GetUniaudControlNum(ULONG deviceid);
extern "C" int GetUniaudControls(ULONG deviceid, void *pids);
extern "C" int GetUniaudControlInfo(ULONG deviceid, ULONG id, void *info);
extern "C" int GetUniaudControlValueGet(ULONG deviceid, ULONG id, void *value);
extern "C" int GetUniaudControlValuePut(ULONG deviceid, ULONG id, void *value);
extern "C" int GetNumberOfPcm(int card_id);
extern "C" int GetUniaudPcmCaps(ULONG deviceid, void *caps);
extern "C" int SetPCMInstance(int card_id, int pcm);
extern "C" int WaitForControlChange(int card_id, int timeout);
extern "C" int GetNumberOfCards(void);
extern "C" OSSRET OSS32_WaveOpen(ULONG deviceid, ULONG streamtype, OSSSTREAMID *pStreamId, int pcm, USHORT fileid);
extern "C" OSSRET OSS32_WaveClose(OSSSTREAMID streamid);
extern "C" int UniaudIoctlHWRefine(OSSSTREAMID streamid, void *pHwParams);
extern "C" int UniaudIoctlHWParamSet(OSSSTREAMID streamid, void *pHwParams);
extern "C" int UniaudIoctlSWParamSet(OSSSTREAMID streamid, void *pSwParams);
extern "C" int UniaudIoctlPCMStatus(OSSSTREAMID streamid, void *pstatus);
extern "C" int UniaudIoctlPCMWrite(OSSSTREAMID streamid, char *buf, int size);
extern "C" int UniaudIoctlPCMRead(OSSSTREAMID streamid, char *buf, int size);
extern "C" int UniaudIoctlPCMPrepare(OSSSTREAMID streamid);
extern "C" int UniaudIoctlPCMStart(OSSSTREAMID streamid);
extern "C" int UniaudIoctlPCMDrop(OSSSTREAMID streamid);
extern "C" int UniaudIoctlPCMResume(OSSSTREAMID streamid, int pause);
extern "C" void UniaudCloseAll(USHORT fileid);
extern "C" int WaitForPCMInterrupt(void *file, int timeout);
extern "C" int unlock_all;
extern "C" int OSS32_CloseUNI16(void);
extern "C" int UniaudCtlGetPowerState(ULONG deviceid, void *state);
extern "C" int UniaudCtlSetPowerState(ULONG deviceid, void *state);

typedef UCHAR LOCKHANDLE[12];

/*
 * structure passed to pcm open ioctl
 */
#pragma pack(1)
typedef struct ioctl_pcm {
    ULONG deviceid;   // card number
    ULONG streamtype; // type of stream (play or record)
    ULONG pcm;        // pcm instance
    int size;         // data size
    ULONG ret;        // return code
} ioctl_pcm;
#pragma pack()

ULONG StratIOCtl(RP __far* _rp)
{
    RPIOCtl __far* rp = (RPIOCtl __far*)_rp;
    USHORT rc = 0;;
    LOCKHANDLE lhParm, lhData;
    LINEAR linParm, linData;
    ULONG pages;
    ULONG *pData;
    ULONG card_id;
    ULONG ctl_id;

    if (rp->Category != CAT_IOCTL_OSS32)
    {
//        printk("not our cat %x. func %x\n", rp->Category, rp->Function);
        // not our category, exit with error
        return (RPERR_COMMAND | RPDONE);
    }

#ifdef DEBUG
    //printk("StratIOCtl 0x%x\n", rp->Function);
#endif
//    printk("cmd: %x, len: %i, pack: %x\n",rp->Function, rp->ParmLength, rp->ParmPacket);
    // work with Parm Packet
    if ((rp->ParmLength != 0 ||
         rp->Function == IOCTL_OSS32_ATTACH /*16 bit ioctl*/) &&
        ((rp->ParmPacket & 0xfffcffff) != 0))
    {
        // got Parm Packet
        rc = DevVirtToLin((USHORT)((ULONG)(rp->ParmPacket) >> 16),
                          (ULONG)((USHORT)(rp->ParmPacket)),
                          (UCHAR * __far *)&linParm);

        if (rc == 0)
        {
            if (rp->Function == IOCTL_OSS32_ATTACH)
            {
                rc = DevVMLock(VMDHL_LONG,
                           (ULONG)linParm,
                           4,
                           (LINEAR)-1L,
                           lhParm,
                               (UCHAR*)&pages);
            }else
            {
                rc = DevVMLock(VMDHL_LONG,
                           (ULONG)linParm,
                           rp->ParmLength,
                           (LINEAR)-1L,
                               lhParm,
                               (UCHAR*)&pages);
            }

            if (rc != 0)
            {
                printk("error in DevVMLock rc = %i\n",rc);
                return (RPERR_PARAMETER | RPDONE);
            }
        } else
        {
            printk("error in VirtToLin rc = %i\n",rc);
            return (RPERR_PARAMETER | RPDONE);
        }
    } else
    {
        // no Parm Packet
        linParm = NULL;
    }

    // work with Data Packet
    if ((rp->DataLength != 0) &&
        ((rp->DataPacket & 0xfffcffff) != 0))
    {
        // got Data Packet
        rc = DevVirtToLin((USHORT)((ULONG)(rp->DataPacket) >> 16),
                          (ULONG)((USHORT)(rp->DataPacket)),
                          (UCHAR * __far *)&linData);
        if (rc == 0)
        {
            rc = DevVMLock(VMDHL_LONG,
                           (ULONG)linData,
                           rp->DataLength,
                           (LINEAR)-1L,
                           lhData,
                           (UCHAR*)&pages);
        } else
            printk("error in VirtToLin rc = %i\n",rc);

        if (rc != 0)
        {

            printk("error in DevVMLock rc = %i\n",rc);
            // error in VirtToLin or DevVMLock
            if (linParm != NULL)
            {
                // linParm present & locked, must to unlock it
                DevVMUnLock(lhParm);
            }
            return (RPERR_PARAMETER | RPDONE);
        }
    } else
    {
        // no Data Packet
        linData = NULL;
    }

    // functions handling
    rc = RPDONE; // ok by default

    switch(rp->Function)
    {
    case IOCTL_OSS32_ATTACH:
        {
            card_id = (ULONG) *linParm;
            // Check if audio init was successful
            if (OSS32_QueryNames(card_id, NULL, 0,
                                 NULL, 0, FALSE) != OSSERR_SUCCESS)
            {
                rc = RPERR_GENERAL | RPDONE;
            }
        } break;

    case IOCTL_OSS32_VERSION:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }
            pData = (ULONG *) linData;
            (*pData) = RM_VERSION;

        } break;

    case IOCTL_OSS32_GET_PCM_NUM:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }
            card_id = (ULONG) *linParm;
            pData = (ULONG *) linData;
            (*pData) = GetNumberOfPcm(card_id);

        } break;

    case IOCTL_OSS32_CARDS_NUM:
        if (rp->DataLength < sizeof(ULONG))
        {
            // invalid Data Pkt
            rc = RPERR_PARAMETER | RPDONE;
            break;
        }
        pData = (ULONG *) linData;
        (*pData) = GetNumberOfCards();
        break;

    case IOCTL_OSS32_PCM_CAPS:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }
            card_id = (ULONG) *linParm;
            pData = (ULONG *) linData;
            GetUniaudPcmCaps(card_id,(void*)*pData);
        } break;

    case IOCTL_OSS32_CARD_INFO:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }
            pData = (ULONG *) linData;
            card_id = (ULONG) *linParm;
            GetUniaudCardInfo(card_id,(void*)*pData);
        } break;

    case IOCTL_OSS32_GET_POWER_STATE:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }
            pData = (ULONG *) linData;
            card_id = (ULONG) *linParm;
            UniaudCtlGetPowerState(card_id,(void*)*pData);
        } break;

    case IOCTL_OSS32_SET_POWER_STATE:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }
            pData = (ULONG *) linData;
            card_id = (ULONG) *linParm;
            UniaudCtlSetPowerState(card_id,(void*)*pData);
        } break;

    case IOCTL_OSS32_GET_CNTRLS_NUM:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }
            pData = (ULONG *) linData;
            card_id = (ULONG) *linParm;
            (*pData) = GetUniaudControlNum(card_id);

        } break;


    case IOCTL_OSS32_GET_CNTRLS:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }
            pData = (ULONG *) linData;
            card_id = (ULONG) *linParm;
            GetUniaudControls(card_id,(void*)*pData);
        } break;

    case IOCTL_OSS32_CNTRL_INFO:
        {

            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            if (rp->ParmLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            pData = (ULONG *) linData;
            ULONG id = *((ULONG *)linParm);
            card_id = (id >> 16) & 0x0000FFFF;
            ctl_id = id & 0x0000FFFF;
//            printk("id: %x, card: %x, ctl: %x\n",id, card_id, ctl_id);
            GetUniaudControlInfo(card_id, ctl_id, (void*)*pData);
        } break;

    case IOCTL_OSS32_CNTRL_GET:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            if (rp->ParmLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            pData = (ULONG *) linData;
            ULONG id = *((ULONG *)linParm);
            card_id = (id >> 16) & 0x0000FFFF;
            ctl_id = id & 0x0000FFFF;

            GetUniaudControlValueGet(card_id, ctl_id,(void*)*pData);
        } break;

    case IOCTL_OSS32_CNTRL_PUT:
        {

            if (rp->DataLength < sizeof(ULONG)) {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            if (rp->ParmLength < sizeof(ULONG)) {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            pData = (ULONG *)linData;
            ULONG id = *((ULONG *)linParm);
            card_id = (id >> 16) & 0x0000FFFF;
            ctl_id = id & 0x0000FFFF;

            GetUniaudControlValuePut(card_id, ctl_id,(void*)*pData);
        } break;

    case IOCTL_OSS32_SET_PCM:
        {

            if (rp->ParmLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            ULONG pcm = (ULONG) *linParm;

            printk("set PCM %i\n",pcm);
            SetPCMInstance(0, pcm);
        } break;

    case IOCTL_OSS32_CNTRL_WAIT:
        {
            if (rp->DataLength < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pData = (ULONG *) linData;
            (*pData) = WaitForControlChange(pcm->deviceid, pcm->streamtype);
        }
        break;
    case IOCTL_OSS32_PCM_OPEN:
        {
            pData = (ULONG *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            // close all pcms from uniaud16 first
            pcm->ret = OSS32_WaveOpen(pcm->deviceid, pcm->streamtype, pData,  pcm->pcm, rp->FileID);
        }
        break;

    case IOCTL_OSS32_PCM_CLOSE:
        {
            ULONG alsa_id = *((ULONG *)linParm);
            OSS32_WaveClose(alsa_id);
        }
        break;
    case IOCTL_OSS32_PCM_READ:
        {
            char *pData1 = (char *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlPCMRead(pcm->deviceid, pData1, pcm->size);
        }
        break;
    case IOCTL_OSS32_PCM_WRITE:
        {
            char *pData1 = (char *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlPCMWrite(pcm->deviceid, pData1, pcm->size);
        }
        break;
    case IOCTL_OSS32_PCM_PAUSE:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlPCMResume(pcm->deviceid, 0);
        }
        break;
    case IOCTL_OSS32_PCM_RESUME:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlPCMResume(pcm->deviceid, 1);
        }
        break;
    case IOCTL_OSS32_PCM_START:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlPCMStart(pcm->deviceid);
        }
        break;
    case IOCTL_OSS32_PCM_DROP:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlPCMDrop(pcm->deviceid);
        }
        break;
    case IOCTL_OSS32_PCM_DRAIN:
        break;
    case IOCTL_OSS32_PCM_STATUS:
        {
            void *pData1 = (void *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlPCMStatus(pcm->deviceid, pData1);
        }
        break;
    case IOCTL_OSS32_PCM_REFINEHWPARAMS:
        {
            void *pData1 = (void *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlHWRefine(pcm->deviceid, pData1);
        }
        break;
    case IOCTL_OSS32_PCM_SETHWPARAMS:
        {
            void *pData1 = (void *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlHWParamSet(pcm->deviceid, pData1);
        }
        break;
    case IOCTL_OSS32_PCM_SETSWPARAMS:
        {
            void *pData1 = (void *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlSWParamSet(pcm->deviceid, pData1);
        }
        break;
    case IOCTL_OSS32_PCM_PREPARE:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret = UniaudIoctlPCMPrepare(pcm->deviceid);
        }
        break;

    case IOCTL_OSS32_PCM_CLOSE_ALL:
        {
            int unlock  = (int)*linParm;
            if (unlock)
                unlock_all = 1;
            else
                UniaudCloseAll(0);
        }
        break;
    case IOCTL_OSS32_PCM_CLOSE_16:
        OSS32_CloseUNI16();
        break;
    case IOCTL_OSS32_PCM_WAIT_INT:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;
            pcm->ret =  WaitForPCMInterrupt((void*)pcm->deviceid, pcm->streamtype);
        }
        break;

    default:
        {
            printk("invalid function code %i\n",rp->Function);
            // invalid function code
            rc = RPERR_PARAMETER | RPDONE;
        } break;
    }

    // unlock Parm & Data if needed
    if (linParm != NULL)
    {
        // linParm present & locked, must to unlock it
        DevVMUnLock(lhParm);
    }
    if (linData != NULL)
    {
        // linData present & locked, must to unlock it
        DevVMUnLock(lhData);
    }

    // all done
    return (rc);
}

//******************************************************************************
// Dispatch Close requests received from the strategy routine
//******************************************************************************
ULONG StratClose(RP __far* _rp)
{
  RPOpenClose __far* rp = (RPOpenClose __far*)_rp;

  // only called if device successfully opened
//  printk("strat close\n");
  numOS2Opens--;

  UniaudCloseAll(rp->FileID);

  if (numOS2Opens == 0) {
	deviceOwner = DEV_NO_OWNER;
  }
  return(RPDONE);
}
//******************************************************************************
//******************************************************************************
