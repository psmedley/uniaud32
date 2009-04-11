/* $Id: soundmixer.c,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
/*
 * MMPM/2 to OSS interface translation layer
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

#include <sound/core.h>
#include <sound/control.h>
#include <sound/info.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/minors.h>
#include <linux/file.h>
#include <linux/soundcard.h>

#define LINUX
#include <ossidc32.h>
#include <stacktoflat.h>
#include <stdlib.h>
#include "soundoss.h"

extern OpenedHandles opened_handles[8 * 256]; // from sound.c

static struct {
    char *name;
    unsigned int index;
    unsigned int recsrc;
} ossid[OSS_MIXER_NRDEVICES] = {
	/* OSS_MIXER_VOLUME   */ { "Master", 0 , -1},
	/* OSS_MIXER_BASS     */ { "Tone Control - Bass", 0, -1},
	/* OSS_MIXER_TREBLE   */ { "Tone Control - Treble", 0, -1},
	/* OSS_MIXER_SYNTH    */ { "Synth", 0 , OSS32_MIX_RECSRC_SYNTH},
	/* OSS_MIXER_PCM      */ { "PCM", 0 , -1},
	/* OSS_MIXER_SPEAKER  */ { "PC Speaker", 0 , -1},
	/* OSS_MIXER_LINE     */ { "Line", 0 , OSS32_MIX_RECSRC_LINE},
	/* OSS_MIXER_MIC      */ { "Mic", 0, OSS32_MIX_RECSRC_MIC},
	/* OSS_MIXER_CD       */ { "CD", 0 , OSS32_MIX_RECSRC_CD},
	/* OSS_MIXER_IMIX     */ { "Monitor Mix", 0 , OSS32_MIX_RECSRC_MIXER},
	/* OSS_MIXER_ALTPCM   */ { "PCM",	1 , -1},
	/* OSS_MIXER_RECLEV   */ { "-- nothing --", 0 , -1},
	/* OSS_MIXER_IGAIN    */ { "Capture", 0 , -1},
	/* OSS_MIXER_OGAIN    */ { "Playback", 0 , -1},
	/* OSS_MIXER_LINE1    */ { "Aux", 0 , OSS32_MIX_RECSRC_AUX},
	/* OSS_MIXER_LINE2    */ { "Aux", 1 , -1},
	/* OSS_MIXER_LINE3    */ { "Aux", 2 , -1},
	/* OSS_MIXER_DIGITAL1 */ { "Digital", 0 , -1},
	/* OSS_MIXER_DIGITAL2 */ { "Digital", 1 , -1},
	/* OSS_MIXER_DIGITAL3 */ { "Digital", 2 , -1},
	/* OSS_MIXER_PHONEIN  */ { "Phone", 0 , OSS32_MIX_RECSRC_PHONE},
	/* OSS_MIXER_PHONEOUT */ { "Phone", 1 , -1},
	/* OSS_MIXER_VIDEO    */ { "Video", 0 , OSS32_MIX_RECSRC_VIDEO},
	/* OSS_MIXER_RADIO    */ { "Radio", 0 , -1},
	/* OSS_MIXER_MONITOR  */ { "Monitor", 0 , -1},
	/* OSS_MIXER_3DDEPTH  */ { "3D Control - Depth", 0 , -1},
        /* OSS_MIXER_3DCENTER */ { "3D Control - Center", 0 , -1},
        /* OSS_MIXER_FRONT    */ { "Front", 0 , -1},
};
char *szRecSources[OSS32_MIX_RECSRC_MAX] = {
    "Mic", "CD", "Line", "Video", "Aux", "Mix", "Mix Mono", "Phone", "Synth"
};

static unsigned char LinToLog[OSS32_MAX_VOLUME+1] = {  
  0,   0,   0,   0,   1,   2,   2,   5,   5,  10,
 10,  10,  16,  19,  20,  22,  24,  25,  27,  27, 
 28,  28,  29,  30,  30,  35,  35,  35,  39,  39,  
 43,  44,  45,  47,  48,  49,  50,  51,  52,  53,  
 55,  56,  57,  59,  60,  62,  63,  64,  65,  66,  
 67,  68,  69,  70,  71,  72,  73,  74,  74,  75,
 76,  77,  78,  79,  79,  80,  81,  82,  83,  84,  
 85,  86,  87,  88,  89,  90,  91,  92,  92,  93,  
 93,  94,  94,  95,  95,  96,  96,  97,  97,  98, 
 98,  99,  99,  99,  99, 100, 100, 100, 100, 100,
 100
};

//******************************************************************************
//Convert linear volume to logarithmic
//******************************************************************************
ULONG ConvertVolume(ULONG ulLinVolume, ULONG ulLogVolMax)
{
    if(ulLinVolume > OSS32_MAX_VOLUME) {
        ulLinVolume = OSS32_MAX_VOLUME;
    }
    ulLinVolume = LinToLog[ulLinVolume];

    return (ulLinVolume * ulLogVolMax) / OSS32_MAX_VOLUME;
}

//******************************************************************************
//******************************************************************************
OSSRET OSS32_MixOpen(ULONG deviceid, OSSSTREAMID *pStreamId)
{
    mixerhandle *pHandle = NULL;
    int          ret, i, j, sz;

    if(pStreamId == NULL) {
        DebugInt3();
        return OSSERR_INVALID_PARAMETER;
    }
    *pStreamId = 0;

    if(alsa_fops == NULL) {
        ret = OSSERR_NO_DEVICE_AVAILABLE;
        goto failure;
    }

    sz = sizeof(mixerhandle);
    pHandle = kmalloc(sz, GFP_KERNEL);
    if(pHandle == NULL) {
        ret = OSSERR_OUT_OF_MEMORY;
        goto failure;
    }
    memset(pHandle, 0, sizeof(mixerhandle));

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //setup pointers in file structure (used internally by ALSA)
    pHandle->file.f_dentry          = &pHandle->d_entry;
    pHandle->file.f_dentry->d_inode = &pHandle->inode;

    pHandle->file.f_mode  = FMODE_WRITE;
    pHandle->inode.i_rdev = SNDRV_MINOR(deviceid, SNDRV_MINOR_CONTROL);

    ret = alsa_fops->open(&pHandle->inode, &pHandle->file);
    if(ret) {
        goto failure;
    }
    //retrieve mixer information
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_CARD_INFO, 
                                    (ULONG)&pHandle->info);
    if(ret) {
        goto failure;
    }
    //get the number of mixer elements
    pHandle->list.offset = 0;
    pHandle->list.space  = 0;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_ELEM_LIST, 
                                    (ULONG)&pHandle->list);
    if(ret) {
        goto failure;
    }
    //allocate memory for all mixer elements
    pHandle->pids = (struct snd_ctl_elem_id *)kmalloc(sizeof(struct snd_ctl_elem_id)*pHandle->list.count, GFP_KERNEL);
    if(pHandle->pids == NULL) {
        goto failure;
    }
    //and retrieve all mixer elements
    pHandle->list.offset = 0;
    pHandle->list.space  = pHandle->list.count;
    pHandle->list.pids   = pHandle->pids;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_ELEM_LIST, 
                                    (ULONG)&pHandle->list);
    if(ret) {
        goto failure;
    }

#if 0
    dprintf(("Mixer name: %s", pHandle->info.mixername));
    dprintf(("List of mixer elements:"));
    for(i=0;i<pHandle->list.count;i++) {
        dprintf(("index %d name %s id %d device %d subdevice %d", pHandle->pids[i].index, pHandle->pids[i].name, pHandle->pids[i].numid, pHandle->pids[i].device, pHandle->pids[i].subdevice));
    }
#endif

    //Extract standard mixer controls from array with control names
    for(j=0;j<OSS_MIXER_NRDEVICES;j++) 
    {
        int namelen = strlen(ossid[j].name);

        pHandle->controls[j].idxVolume        = -1;
        pHandle->controls[j].idxMute          = -1;
        pHandle->controls[j].idxCustom        = -1;
        pHandle->controls[j].idxCaptureSwitch = -1;

        for(i=0;i<pHandle->list.count;i++) 
        {
            if (pHandle->pids[i].index == ossid[j].index &&
                strncmp(pHandle->pids[i].name, ossid[j].name, namelen) == 0)
            {
                int controlnamelen = strlen(pHandle->pids[i].name);

                if(namelen == controlnamelen)
                {//control names are identical; found exact match
                    pHandle->controls[j].idxVolume = i;
                    break;
                }
                else 
                {//first part of the control name is correct; now find out what 
                 //is it exactly
                    char *nextword = &pHandle->pids[i].name[namelen];
                    while(*nextword && *nextword == ' ') nextword++;

                    if(strncmp(nextword, MIXER_PLAYBACKVOLUME, sizeof(MIXER_PLAYBACKVOLUME)-1) == 0 ||
                       strncmp(nextword, MIXER_VOLUME, sizeof(MIXER_VOLUME)-1) == 0) 
                    {//volume control
                        pHandle->controls[j].idxVolume = i;
                    }
                    else
                    if(strncmp(nextword, MIXER_PLAYBACKSWITCH, sizeof(MIXER_PLAYBACKSWITCH)-1) == 0 ||
                       strncmp(nextword, MIXER_SWITCH, sizeof(MIXER_SWITCH)-1) == 0) 
                    {//mute control
                        pHandle->controls[j].idxMute = i;
                    }
                    else
                    if(strncmp(nextword, MIXER_SOURCE, sizeof(MIXER_SOURCE)-1) == 0) 
                    {//source control (e.g. recording source)
                        pHandle->controls[j].idxCustom = i;
                    }
                    else
                    if(strncmp(nextword, MIXER_CAPTUREROUTE, sizeof(MIXER_CAPTUREROUTE)-1) == 0 ||
                       strncmp(nextword, MIXER_CAPTURESWITCH, sizeof(MIXER_CAPTURESWITCH)-1) == 0) 
                    {//source control for recording (per input)
                        pHandle->controls[j].idxCaptureSwitch = i;
                    }
                    else
                    if(i == OSS_MIXER_MIC) {
                        if(strncmp(nextword, MIXER_BOOST, sizeof(MIXER_BOOST)-1) == 0) 
                        {//mic boost switch
                            pHandle->controls[j].idxCustom = i;
                        }
                    }
                }
            }
        }
    }

    pHandle->reccaps = 0;
    for(j=0;j<OSS32_MIX_RECSRC_MAX;j++) {
        pHandle->idxRecCaps[j] = -1;
    }

    //request information about available capture sources
    if(pHandle->controls[OSS_MIXER_IGAIN].idxCustom != -1) 
    {
        struct snd_ctl_elem_info  *pElemInfo = NULL;  
        int                   idx, j;

        idx = pHandle->controls[OSS_MIXER_IGAIN].idxCustom;

        //set operation to non-blocking
        pHandle->file.f_flags = O_NONBLOCK;
        
        pHandle->rectype = RECTYPE_SELECTOR;

        //too big to put on the stack
        pElemInfo = (struct snd_ctl_elem_info *)kmalloc(sizeof(struct snd_ctl_elem_info), GFP_KERNEL);
        if(pElemInfo == NULL) {
            DebugInt3();
            goto failure;
        }

        pElemInfo->value.enumerated.items = 1;
        for(i=0;i<pElemInfo->value.enumerated.items;i++) 
        {
            pElemInfo->value.enumerated.item = i;
            pElemInfo->id.numid = pHandle->pids[idx].numid;
            ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_INFO, (ULONG)pElemInfo);
            if(ret) {
                DebugInt3();
                break;
            }
            if(pElemInfo->type != SNDRV_CTL_ELEM_TYPE_ENUMERATED) {
                DebugInt3();
                break;
            }
            for(j=0;j<OSS32_MIX_RECSRC_MAX;j++) {
                if(!strcmp(pElemInfo->value.enumerated.name, szRecSources[j])) {
                    pHandle->reccaps      |= OSS32_MIX_FLAG(j);
                    pHandle->idxRecCaps[j] = i; //save alsa index
                    break;
                }
            }
        }
        kfree(pElemInfo);
    }
    else
    {//This card has no record source selection, but probably route switches for
     //each input source (SB mixers (also ALS4000), CMedia)
        pHandle->rectype = RECTYPE_SWITCH;
        for(j=0;j<OSS32_MIX_RECSRC_MAX;j++) {
            pHandle->idxRecCaps[j] = -1;
        }
        for(j=0;j<OSS_MIXER_NRDEVICES;j++) 
        {
            if(pHandle->controls[j].idxCaptureSwitch != -1) {
                pHandle->reccaps                    |= OSS32_MIX_FLAG(ossid[j].recsrc);
                pHandle->idxRecCaps[ossid[j].recsrc] = pHandle->controls[j].idxCaptureSwitch; //save alsa index
            }
        }
    }

    pHandle->magic = MAGIC_MIXER_ALSA32;
    *pStreamId = (ULONG)pHandle;
    return OSSERR_SUCCESS;

failure:
    if(pHandle) {
        if(pHandle->pids) kfree(pHandle->pids);
        kfree(pHandle);
    }
    DebugInt3();
    return OSSERR_OUT_OF_MEMORY;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MixClose(OSSSTREAMID streamid)
{
    mixerhandle *pHandle = (mixerhandle *)streamid;
    int          ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_MIXER_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;
    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
    kfree(pHandle->pids);  //free mixer element array
    kfree(pHandle);        //free handle data

    if(ret) {
        DebugInt3();
        return UNIXToOSSError(ret);
    }
    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MixGetVolume(OSSSTREAMID streamid, ULONG line, ULONG *pVolume)
{
    mixerhandle *pHandle = (mixerhandle *)streamid;
    int          ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_MIXER_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;
 
    return OSSERR_NOT_SUPPORTED;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MixSetVolume(OSSSTREAMID streamid, ULONG line, ULONG volume)
{
    mixerhandle          *pHandle = (mixerhandle *)streamid;
    struct snd_ctl_elem_value *pElem = NULL;
    struct snd_ctl_elem_info  *pElemInfo;  
    int                   ret, idx, lVol, rVol = 0, idxMute, cnt;

    if(pHandle == NULL || pHandle->magic != MAGIC_MIXER_ALSA32) {
        printk("Invalid handle in OSS32_MixSetVolume\n");
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //too big to put on the stack
    pElem = (struct snd_ctl_elem_value *)kmalloc(sizeof(struct snd_ctl_elem_value) + sizeof(struct snd_ctl_elem_info), GFP_KERNEL);
    if(pElem == NULL) {
        printk("Out of memory in OSS32_MixSetVolume\n");
        DebugInt3();
        return OSSERR_OUT_OF_MEMORY;
    }
    pElemInfo = (struct snd_ctl_elem_info *)(pElem+1);

    switch(line) {
    case OSS32_MIX_VOLUME_MASTER_FRONT:
        idx = pHandle->controls[OSS_MIXER_VOLUME].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_VOLUME].idxMute;
        if (idx == -1)
        {
            /* HDA codecs workaround */
            idx = pHandle->controls[OSS_MIXER_FRONT].idxVolume;
            idxMute = pHandle->controls[OSS_MIXER_FRONT].idxMute;
        }
        break;
    case OSS32_MIX_VOLUME_MASTER_REAR: //TODO:
        idx = pHandle->controls[OSS_MIXER_VOLUME].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_VOLUME].idxMute;
        break;
    case OSS32_MIX_VOLUME_PCM:
        idx = pHandle->controls[OSS_MIXER_PCM].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_PCM].idxMute;
        if (idx == -1)
        {
            /* HDA codecs workaround */
            idx = pHandle->controls[OSS_MIXER_FRONT].idxVolume;
            idxMute = pHandle->controls[OSS_MIXER_FRONT].idxMute;
        }
        break;
    case OSS32_MIX_VOLUME_MIDI:
        idx = pHandle->controls[OSS_MIXER_SYNTH].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_SYNTH].idxMute;
        break;
    case OSS32_MIX_VOLUME_LINEIN:
        idx = pHandle->controls[OSS_MIXER_LINE].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_LINE].idxMute;
        break;
    case OSS32_MIX_VOLUME_MIC:
        idx = pHandle->controls[OSS_MIXER_MIC].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_MIC].idxMute;
        break;
    case OSS32_MIX_VOLUME_CD:
        idx = pHandle->controls[OSS_MIXER_CD].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_CD].idxMute;
        break;
    case OSS32_MIX_VOLUME_SPDIF:
        idx = pHandle->controls[OSS_MIXER_DIGITAL1].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_DIGITAL1].idxMute;
        break;
    case OSS32_MIX_VOLUME_VIDEO:
        idx = pHandle->controls[OSS_MIXER_VIDEO].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_VIDEO].idxMute;
        break;
    case OSS32_MIX_VOLUME_SPEAKER:
        idx = pHandle->controls[OSS_MIXER_SPEAKER].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_SPEAKER].idxMute;
        break;
    case OSS32_MIX_VOLUME_PHONE:
        idx = pHandle->controls[OSS_MIXER_PHONEOUT].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_PHONEOUT].idxMute;
        break;
    case OSS32_MIX_VOLUME_HEADPHONE: //TODO:
        idx = pHandle->controls[OSS_MIXER_PHONEOUT].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_PHONEOUT].idxMute;
        break;
    case OSS32_MIX_VOLUME_AUX:
        idx = pHandle->controls[OSS_MIXER_LINE1].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_LINE1].idxMute;
        break;
    case OSS32_MIX_VOLUME_CAPTURE:
        idx = pHandle->controls[OSS_MIXER_IGAIN].idxVolume;
        idxMute = pHandle->controls[OSS_MIXER_IGAIN].idxMute;
        break;

    default:
        DebugInt3();
        ret = OSSERR_INVALID_PARAMETER;
        goto fail;
    }
    if(idx == -1) {
        dprintf(("Unknown control %d", line));
        ret = OSSERR_INVALID_PARAMETER;
        goto fail;
    }

    if(idxMute != -1 && volume != 0) {
        //disable mute
        pElem->id.numid = pHandle->pids[idxMute].numid;
        pElem->indirect = 0;

        pElem->value.integer.value[0] = TRUE;  //switch, not mute control (inversed)
        pElem->value.integer.value[1] = TRUE;
        ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_WRITE, (ULONG)pElem);
    }
    //request information about mixer control
    pElemInfo->id.numid = pHandle->pids[idx].numid;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_INFO, (ULONG)pElemInfo);
    if(ret) {
        ret = UNIXToOSSError(ret);
        DebugInt3();
        goto fail;
    }
    if(pElemInfo->type != SNDRV_CTL_ELEM_TYPE_INTEGER) {
        ret = OSSERR_INVALID_PARAMETER;
        DebugInt3();
        goto fail;
    }
    pElem->id.numid = pHandle->pids[idx].numid;
    pElem->indirect = 0;

    lVol = ConvertVolume(GET_VOLUME_L(volume), pElemInfo->value.integer.max);
    pElem->value.integer.value[0] = lVol;

    if(pElemInfo->count > 1) { //stereo
        rVol = ConvertVolume(GET_VOLUME_R(volume), pElemInfo->value.integer.max);
        pElem->value.integer.value[1] = rVol;
    }

    printk("OSS32_MixSetVolume of %s streamid %X to (%d,%d)(%d,%d) caps %d\n",
           pHandle->pids[idx].name,
           (ULONG)pHandle, GET_VOLUME_L(volume),
           GET_VOLUME_R(volume), lVol, rVol, pElemInfo->value.integer.max);

    // looking for more, then one opened streams to prevent of muting active stream
    cnt = 0;
    for (idx=0; idx < 8*256; idx++)
        if (opened_handles[idx].handle != 0)
            cnt++;

    //    if (((cnt == 1 && (lVol==0 && rVol==0)) || (lVol>0 && rVol>0)) ||
    if (cnt == 1 || line != OSS32_MIX_VOLUME_PCM)
    {
        ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_WRITE, (ULONG)pElem);

        if(idxMute != -1 && volume == 0) {
            //enable mute
            pElem->id.numid = pHandle->pids[idxMute].numid;
            pElem->indirect = 0;

            pElem->value.integer.value[0] = FALSE;  //switch, not mute control (inversed)
            pElem->value.integer.value[1] = FALSE;
            ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_WRITE, (ULONG)pElem);
        }
    }

    kfree(pElem);
    pElem = NULL;
    if(ret) {
        printk("ret = %i\n", ret);
        DebugInt3();
        return UNIXToOSSError(ret);
    }
    return OSSERR_SUCCESS;

fail:
    printk("OSS32_MixSetVolume failed: %i\n", ret);
    if(pElem) kfree(pElem);
    return ret;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MixSetProperty(OSSSTREAMID streamid, ULONG ulLine, ULONG ulValue)
{
    mixerhandle          *pHandle = (mixerhandle *)streamid;
    struct snd_ctl_elem_value *pElem = NULL;
    struct snd_ctl_elem_info  *pElemInfo;  
    int                   ret, idx = -1, lVol, rVol = 0, j, i;

    if(pHandle == NULL || pHandle->magic != MAGIC_MIXER_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;
 
    //too big to put on the stack
    pElem = (struct snd_ctl_elem_value *)kmalloc(sizeof(struct snd_ctl_elem_value) + sizeof(struct snd_ctl_elem_info), GFP_KERNEL);
    if(pElem == NULL) {
        DebugInt3();
        return OSSERR_OUT_OF_MEMORY;
    }
    pElemInfo = (struct snd_ctl_elem_info *)(pElem+1);

    switch(ulLine) {
    case OSS32_MIX_INPUTSRC:
        idx = pHandle->controls[OSS_MIXER_IGAIN].idxCustom;
        //is this capture source supported by the hardware??
        if(!(pHandle->reccaps & OSS32_MIX_FLAG(ulValue))) {
            DebugInt3();
            ret = OSSERR_INVALID_PARAMETER;
            goto fail;
        }
        if(pHandle->rectype == RECTYPE_SELECTOR) {//input source selector
            //set left and right capture source
            pElem->value.enumerated.item[0] = pHandle->idxRecCaps[ulValue];
            pElem->value.enumerated.item[1] = pHandle->idxRecCaps[ulValue];
        }
        else {//capture switch for each input source
            //first turn off all capture switches...
            for(j=0;j<OSS32_MIX_RECSRC_MAX;j++) 
            {
                if(pHandle->idxRecCaps[j] != -1) {
                    idx                           = pHandle->idxRecCaps[j];

                    //request information about mixer control
                    pElemInfo->id.numid           = pHandle->pids[idx].numid;
                    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_INFO, (ULONG)pElemInfo);
                    if(ret) {
                        ret = UNIXToOSSError(ret);
                        DebugInt3();
                        goto fail;
                    }
                    if(pElemInfo->type != SNDRV_CTL_ELEM_TYPE_BOOLEAN) {
                        ret = OSSERR_INVALID_PARAMETER;
                        DebugInt3();
                        goto fail;
                    }

                    pElem->id.numid               = pHandle->pids[idx].numid;
                    pElem->indirect               = 0;
                    for(i=0;i<pElemInfo->count;i++) {
                        pElem->value.integer.value[i] = 0;
                    }

                    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_WRITE, (ULONG)pElem);
                    if(ret) {
                        ret = UNIXToOSSError(ret);
                        DebugInt3();
                        goto fail;
                    }
                }
            }
            //request information about mixer control
            pElemInfo->id.numid = pHandle->pids[idx].numid;
            ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_INFO, (ULONG)pElemInfo);
            if(ret) {
                ret = UNIXToOSSError(ret);
                DebugInt3();
                goto fail;
            }

            //and enable the capture switch for the selected input source
            idx = pHandle->idxRecCaps[ulValue];
            for(i=0;i<pElemInfo->count;i++) {
                pElem->value.integer.value[i] = 1;
            }
        }

        break;
    
    case OSS32_MIX_SWITCH_MICBOOST:
        idx = pHandle->controls[OSS_MIXER_MIC].idxCustom;
        if(idx == -1) {
            DebugInt3();
            ret = OSSERR_INVALID_PARAMETER;
            goto fail;
        }
        //set mic switch value (on/off)
        pElem->value.integer.value[0] = ulValue;
        break;

    case OSS32_MIX_LEVEL_BASS:
        idx = pHandle->controls[OSS_MIXER_BASS].idxVolume;
        goto levelcontinue;
    case OSS32_MIX_LEVEL_TREBLE:
        idx = pHandle->controls[OSS_MIXER_TREBLE].idxVolume;
        goto levelcontinue;
    case OSS32_MIX_LEVEL_3DCENTER:
        idx = pHandle->controls[OSS_MIXER_3DCENTER].idxVolume;
        goto levelcontinue;
    case OSS32_MIX_LEVEL_3DDEPTH:
        idx = pHandle->controls[OSS_MIXER_3DDEPTH].idxVolume;
levelcontinue:

        if(idx == -1) {//supported?
            DebugInt3();
            ret = OSSERR_INVALID_PARAMETER;
            goto fail;
        }
        //request information about mixer control
        pElemInfo->id.numid = pHandle->pids[idx].numid;
        ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_INFO, (ULONG)pElemInfo);
        if(ret) {
            ret = UNIXToOSSError(ret);
            DebugInt3();
            goto fail;
        }
        if(pElemInfo->type != SNDRV_CTL_ELEM_TYPE_INTEGER) {
            ret = OSSERR_INVALID_PARAMETER;
            DebugInt3();
            goto fail;
        }
        lVol = ConvertVolume(GET_VOLUME_L(ulValue), pElemInfo->value.integer.max);
        pElem->value.integer.value[0] = lVol;

        if(pElemInfo->count > 1) { //stereo
            rVol = ConvertVolume(GET_VOLUME_R(ulValue), pElemInfo->value.integer.max);
            pElem->value.integer.value[1] = rVol;
        }
        break;

    default:
        DebugInt3();
        ret = OSSERR_INVALID_PARAMETER;
        goto fail;
    }
    pElem->id.numid = pHandle->pids[idx].numid;
    pElem->indirect = 0;

    dprintf(("OSS32_MixSetProperty of %s to %x", pHandle->pids[idx].name, ulValue));
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_WRITE, (ULONG)pElem);

    kfree(pElem);
    pElem = NULL;
    if(ret) {
        DebugInt3();
        return UNIXToOSSError(ret);
    }
    return OSSERR_SUCCESS;

fail:
    if(pElem) kfree(pElem);
    return ret;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MixGetProperty(OSSSTREAMID streamid, ULONG line, ULONG *pValue)
{
    mixerhandle *pHandle = (mixerhandle *)streamid;
    int          ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_MIXER_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;
 
    return OSSERR_NOT_SUPPORTED;
}
//******************************************************************************
//******************************************************************************
ULONG OSSToALSAVolume(ULONG OSSVolIdx)
{
    switch(OSSVolIdx) {
    case OSS_MIXER_VOLUME:
        return OSS32_MIX_VOLUME_MASTER_FRONT;
#if 0
    case OSS_MIXER_VOLUME: //TODO:
        return OSS32_MIX_VOLUME_MASTER_REAR;
#endif
    case OSS_MIXER_PCM:
        return OSS32_MIX_VOLUME_PCM;
    case OSS_MIXER_SYNTH:
        return OSS32_MIX_VOLUME_MIDI;
    case OSS_MIXER_LINE:
        return OSS32_MIX_VOLUME_LINEIN;
    case OSS_MIXER_MIC:
        return OSS32_MIX_VOLUME_MIC;
    case OSS_MIXER_CD:
        return OSS32_MIX_VOLUME_CD;
    case OSS_MIXER_DIGITAL1:
        return OSS32_MIX_VOLUME_SPDIF;
    case OSS_MIXER_VIDEO:
        return OSS32_MIX_VOLUME_VIDEO;
    case OSS_MIXER_SPEAKER:
        return OSS32_MIX_VOLUME_SPEAKER;
    case OSS_MIXER_PHONEOUT:
        return OSS32_MIX_VOLUME_PHONE;
    case OSS_MIXER_IGAIN:
        return OSS32_MIX_VOLUME_CAPTURE;
    case OSS_MIXER_TREBLE:
        return OSS32_MIX_LEVEL_TREBLE;
    case OSS_MIXER_BASS:
        return OSS32_MIX_LEVEL_BASS;
#if 0
    case OSS_MIXER_PHONEOUT: //TODO:
        return OSS32_MIX_VOLUME_HEADPHONE;
#endif
    case OSS_MIXER_LINE1:
        return OSS32_MIX_VOLUME_AUX;
    }
    return -1;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MixQueryCaps(OSSSTREAMID streamid, POSS32_MIXCAPS pCaps)
{
    mixerhandle *pHandle = (mixerhandle *)streamid;
    int i;

    if(pHandle == NULL || pHandle->magic != MAGIC_MIXER_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }

    strncpy(pCaps->name, pHandle->info.mixername, sizeof(pCaps->name));
    pCaps->fuCtrlCaps = 0;
    pCaps->fuRecCaps  = 0;

    for(i=0;i<OSS_MIXER_NRDEVICES;i++) 
    {
        if(pHandle->controls[i].idxVolume != -1) {
            ULONG volidx = OSSToALSAVolume(i);
            if(volidx != -1) 
                pCaps->fuCtrlCaps |= OSS32_MIX_FLAG(volidx);
        }
    }

    //if it has a capture source control or the card has capture route switches,
    //then we support intput source selection
    if(pHandle->controls[OSS_MIXER_IGAIN].idxCustom != -1 ||
       pHandle->rectype == RECTYPE_SWITCH) 
    {        
        pCaps->fuCtrlCaps |= OSS32_MIX_FLAG(OSS32_MIX_INPUTSRC);
        pCaps->fuRecCaps   = pHandle->reccaps;
    }
    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MixQueryName(ULONG deviceid, char *pszMixerName, ULONG cbMixerName)
{
    mixerhandle *pHandle = NULL;
    int          ret, i, j;

    if(alsa_fops == NULL)
        return OSSERR_NO_DEVICE_AVAILABLE;

    pHandle = kmalloc(sizeof(mixerhandle), GFP_KERNEL);
    if(pHandle == NULL) {
        ret = OSSERR_OUT_OF_MEMORY;
        goto failure;
    }
    memset(pHandle, 0, sizeof(mixerhandle));

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //setup pointers in file structure (used internally by ALSA)
    pHandle->file.f_dentry          = &pHandle->d_entry;
    pHandle->file.f_dentry->d_inode = &pHandle->inode;

    pHandle->file.f_mode  = FMODE_WRITE;
    pHandle->inode.i_rdev = SNDRV_MINOR(deviceid, SNDRV_MINOR_CONTROL);

    ret = alsa_fops->open(&pHandle->inode, &pHandle->file);
    if(ret) {
        goto failure;
    }
    //retrieve mixer information
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_CARD_INFO, 
                                    (ULONG)&pHandle->info);
    if(ret) {
        goto failure;
    }

    strncpy(pszMixerName, pHandle->info.mixername, cbMixerName);

    pHandle->file.f_flags = O_NONBLOCK;
    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
    if(ret) {
        goto failure;
    }
    kfree(pHandle);
    return OSSERR_SUCCESS;

failure:
    if(pHandle) {
        kfree(pHandle);
    }
    DebugInt3();
    return OSSERR_OUT_OF_MEMORY;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_QueryNames(ULONG deviceid, char *pszDeviceName, ULONG cbDeviceName,
                        char *pszMixerName, ULONG cbMixerName, BOOL fLongName)
{
    mixerhandle *pHandle = NULL;
    int          ret, i, j;

    if(alsa_fops == NULL) {
        ret = OSSERR_NO_DEVICE_AVAILABLE;
        printk("ret = OSSERR_NO_DEVICE_AVAILABLE\n");
        goto failure;
    }

    pHandle = kmalloc(sizeof(mixerhandle), GFP_KERNEL);
    if(pHandle == NULL) {
        ret = OSSERR_OUT_OF_MEMORY;
        goto failure;
    }
    memset(pHandle, 0, sizeof(mixerhandle));

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //setup pointers in file structure (used internally by ALSA)
    pHandle->file.f_dentry          = &pHandle->d_entry;
    pHandle->file.f_dentry->d_inode = &pHandle->inode;

    pHandle->file.f_mode  = FMODE_WRITE;
    pHandle->inode.i_rdev = SNDRV_MINOR(deviceid, SNDRV_MINOR_CONTROL);

    ret = alsa_fops->open(&pHandle->inode, &pHandle->file);
    if(ret) {
        printk("open ret = %i\n", ret);
        goto failure;
    }
    //retrieve mixer information
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_CARD_INFO, 
                                    (ULONG)&pHandle->info);
    if(ret) {
        printk("ioctl ret = %i\n", ret);
        goto failure;
    }

    if(pszDeviceName) {
        if(fLongName == TRUE) {
             strncpy(pszDeviceName, pHandle->info.longname, cbDeviceName);
        }
        else strncpy(pszDeviceName, pHandle->info.name, cbDeviceName);
    }
    if(pszMixerName) {
        strncpy(pszMixerName, pHandle->info.mixername, cbMixerName);
    }

//    printk("Card: %s with mixer %s\n",pszDeviceName, pszMixerName);

    pHandle->file.f_flags = O_NONBLOCK;
    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
    if(ret) {
        printk("release ret = %i\n", ret);
        goto failure;
    }
    kfree(pHandle);
    return OSSERR_SUCCESS;

failure:
    if(pHandle) {
        kfree(pHandle);
    }
    DebugInt3();
    printk("OSS32_QueryNames() ret = %i\n", ret);
    return OSSERR_OUT_OF_MEMORY;
}
//******************************************************************************
//******************************************************************************

