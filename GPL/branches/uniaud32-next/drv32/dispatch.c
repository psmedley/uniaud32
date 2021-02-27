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
#include <ossidc.h>
#include <ossidc32.h>
#include <version.h>
#include <kee.h>
#include <u32ioctl.h>
#include "strategy.h"

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

ULONG StratIOCtl(REQPACKET __far* rp)
{
    USHORT rc = 0;
    KEEVMLock lhParm, lhData;
    char  *linParm;
    char  *linData;
    ULONG *pData;
    ULONG card_id;
    ULONG ctl_id;

    if (rp->ioctl.bCategory != CAT_IOCTL_OSS32)
    {
        //printk("not our cat %x. func %x\n", rp->Category, rp->Function);
        // not our category, exit with error
        return (RPERR_BADCOMMAND | RPDONE);
    }

    #ifdef DEBUG
    //printk("StratIOCtl 0x%x\n", rp->Function);
    #endif
    //    printk("cmd: %x, len: %i, pack: %x\n",rp->ioctl.bFunction, rp->ioctl.usParmLen, rp->ioctl.pvParm);
    // work with Parm Packet
    if ((rp->ioctl.usParmLen != 0 ||
         rp->ioctl.bFunction == IOCTL_OSS32_ATTACH /*16 bit ioctl*/) &&
        ((rp->ioctl.pvParm & 0xfffcffff) != 0))
    {
        // got Parm Packet
        linParm = (char*)KernSelToFlat(rp->ioctl.pvParm);

        if (rp->ioctl.bFunction == IOCTL_OSS32_ATTACH)
        {
            rc = KernVMLock(KEE_VML_LONGLOCK, (PVOID)linParm, 4, &lhParm, (KEEVMPageList*)-1, 0);
        }
        else
        {
            rc = KernVMLock(KEE_VML_LONGLOCK, (PVOID)linParm, rp->ioctl.usParmLen, &lhParm, (KEEVMPageList*)-1, 0);
        }

        if (rc != 0)
        {
            printk("error in KernVMLock rc = %i\n",rc);
            return (RPERR_PARAMETER | RPDONE);
        }
    }
    else
    {
        // no Parm Packet
        linParm = NULL;
    }

    // work with Data Packet
    if ((rp->ioctl.usDataLen != 0) &&
        ((rp->ioctl.pvData & 0xfffcffff) != 0))
    {
        // got Data Packet
        linData = (char*)KernSelToFlat(rp->ioctl.pvData);

        rc = KernVMLock(KEE_VML_LONGLOCK, (PVOID)linData, rp->ioctl.usDataLen, &lhData, (KEEVMPageList*)-1, 0);
        if (rc != 0)
        {
            printk("error in KernVMLock rc = %i\n",rc);
            if (linParm != NULL)
            {
                // linParm present & locked, must to unlock it
                KernVMUnlock(&lhParm);
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

    switch(rp->ioctl.bFunction)
    {
    case IOCTL_OSS32_ATTACH:
        {
          dprintf(("OSS32_ATTACH\n"));
            card_id = (ULONG) *linParm;
            // Check if audio init was successful
            if (OSS32_QueryNames(card_id, NULL, 0, NULL, 0, FALSE) != OSSERR_SUCCESS)
            {
                rc = RPERR_GENERAL | RPDONE;
            }
        } break;

    case IOCTL_OSS32_VERSION:
        {
          dprintf(("OSS32_VERSION\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
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
          dprintf(("GET_PCM_NUM\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
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
        dprintf(("OSS32_CARDS_NUM\n"));
        if (rp->ioctl.usDataLen < sizeof(ULONG))
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
          dprintf(("OSS32_PCM_CAPS\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
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
          dprintf(("OSS32_CARD_INFO\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
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
          dprintf(("OSS32_GET_POWER_STATE\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
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
          dprintf(("OSS32_SET_POWER_STATE\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
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
          dprintf(("OSS32_GET_CNTRLS_NUM\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
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
          dprintf(("OSS32_GET_CNTRLS\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
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
          dprintf(("OSS32_CNTRL_INFO\n"));

            if (rp->ioctl.usDataLen < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            if (rp->ioctl.usParmLen < sizeof(ULONG))
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
          dprintf(("OSS32_CNTRL_GET\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
            {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            if (rp->ioctl.usParmLen < sizeof(ULONG))
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
          dprintf(("OSS32_CNTRL_PUT\n"));

            if (rp->ioctl.usDataLen < sizeof(ULONG)) {
                // invalid Data Pkt
                rc = RPERR_PARAMETER | RPDONE;
                break;
            }

            if (rp->ioctl.usParmLen < sizeof(ULONG)) {
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
          dprintf(("OSS32_SET_PCM\n"));

            if (rp->ioctl.usParmLen < sizeof(ULONG))
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
          dprintf(("OSS32_CNTRL_WAIT\n"));
            if (rp->ioctl.usDataLen < sizeof(ULONG))
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

          dprintf(("OSS32_PCM_OPEN\n"));
            // close all pcms from uniaud16 first
            pcm->ret = OSS32_WaveOpen(pcm->deviceid, pcm->streamtype, pData,  pcm->pcm, rp->ioctl.usSysFileNum);
        }
        break;

    case IOCTL_OSS32_PCM_CLOSE:
        {
            ULONG alsa_id = *((ULONG *)linParm);

          dprintf(("OSS32_PCM_CLOSE\n"));
            OSS32_WaveClose(alsa_id);
        }
        break;
    case IOCTL_OSS32_PCM_READ:
        {
            char *pData1 = (char *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_READ\n"));
            pcm->ret = UniaudIoctlPCMRead(pcm->deviceid, pData1, pcm->size);
        }
        break;
    case IOCTL_OSS32_PCM_WRITE:
        {
            char *pData1 = (char *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_WRITE\n"));
            pcm->ret = UniaudIoctlPCMWrite(pcm->deviceid, pData1, pcm->size);
        }
        break;
    case IOCTL_OSS32_PCM_PAUSE:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_PAUSE\n"));
            pcm->ret = UniaudIoctlPCMResume(pcm->deviceid, 0);
        }
        break;
    case IOCTL_OSS32_PCM_RESUME:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_RESUME\n"));
            pcm->ret = UniaudIoctlPCMResume(pcm->deviceid, 1);
        }
        break;
    case IOCTL_OSS32_PCM_START:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_START\n"));
            pcm->ret = UniaudIoctlPCMStart(pcm->deviceid);
        }
        break;
    case IOCTL_OSS32_PCM_DROP:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_DROP\n"));
            pcm->ret = UniaudIoctlPCMDrop(pcm->deviceid);
        }
        break;
    case IOCTL_OSS32_PCM_DRAIN:
        break;
    case IOCTL_OSS32_PCM_STATUS:
        {
            void *pData1 = (void *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_STATUS\n"));
            pcm->ret = UniaudIoctlPCMStatus(pcm->deviceid, pData1);
        }
        break;
    case IOCTL_OSS32_PCM_REFINEHWPARAMS:
        {
            void *pData1 = (void *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_REFINEHWPARAMS\n"));
            pcm->ret = UniaudIoctlHWRefine(pcm->deviceid, pData1);
        }
        break;
    case IOCTL_OSS32_PCM_SETHWPARAMS:
        {
            void *pData1 = (void *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_SETHWPARAMS\n"));
            pcm->ret = UniaudIoctlHWParamSet(pcm->deviceid, pData1);
        }
        break;
    case IOCTL_OSS32_PCM_SETSWPARAMS:
        {
            void *pData1 = (void *)linData;
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_SETSWPARAMS\n"));
            pcm->ret = UniaudIoctlSWParamSet(pcm->deviceid, pData1);
        }
        break;
    case IOCTL_OSS32_PCM_PREPARE:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_PREPARE\n"));
            pcm->ret = UniaudIoctlPCMPrepare(pcm->deviceid);
        }
        break;

    case IOCTL_OSS32_PCM_CLOSE_ALL:
        {
            int unlock  = (int)*linParm;

          dprintf(("OSS32_PCM_CLOSE_ALL\n"));
            if (unlock)
                unlock_all = 1;
            else
                UniaudCloseAll(0);
        }
        break;
    case IOCTL_OSS32_PCM_CLOSE_16:
          dprintf(("OSS32_PCM_CLOSE_16\n"));
        OSS32_CloseUNI16();
        break;
    case IOCTL_OSS32_PCM_WAIT_INT:
        {
            ioctl_pcm *pcm = (ioctl_pcm *)linParm;

          dprintf(("OSS32_PCM_WAIT_INT\n"));
            pcm->ret =  WaitForPCMInterrupt((void*)pcm->deviceid, pcm->streamtype);
        }
        break;

    default:
        {
            printk("invalid function code %i\n",rp->ioctl.bFunction);
            // invalid function code
            rc = RPERR_PARAMETER | RPDONE;
        } break;
    }

    // unlock Parm & Data if needed
    if (linParm != NULL)
    {
        // linParm present & locked, must to unlock it
        KernVMUnlock(&lhParm);
    }
    if (linData != NULL)
    {
        // linData present & locked, must to unlock it
        KernVMUnlock(&lhData);
    }

    // all done
    return (rc);
}
