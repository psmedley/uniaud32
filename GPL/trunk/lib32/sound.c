/* $Id: sound.c,v 1.2 2003/08/08 15:09:03 vladest Exp $ */
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

#include <sound/driver.h>
#include <sound/control.h>
#include <sound/info.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/minors.h>
#include <linux/file.h>
#include <linux/soundcard.h>

#define LINUX
#include <ossidc32.h>
#include <osspci.h>
#include <stacktoflat.h>
#include <stdlib.h>
#include "soundoss.h"

#undef samples_to_bytes
#undef bytes_to_samples
#define samples_to_bytes(a)     ((a*pHandle->doublesamplesize)/2)
#define bytes_to_samples(a)    (pHandle->doublesamplesize ? ((a*2)/pHandle->doublesamplesize) : a)

int GetMaxChannels(ULONG deviceid, int type);

struct file_operations oss_devices[OSS32_MAX_DEVICES] = {0};
struct file_operations *alsa_fops = NULL;
int per_bytes = 0;
int prev_size = 0;
int pcm_device = 0;
int total = 0;
int pcm_instances = 0;

OpenedHandles opened_handles[8 * 256] = {0};

//OSS32 to ALSA datatype conversion table
static OSSToALSADataType[OSS32_PCM_MAX_FORMATS] = {
/* OSS32_PCM_FORMAT_S8     */ SNDRV_PCM_FORMAT_S8,          //signed 8 bits sample
/* OSS32_PCM_FORMAT_U8     */ SNDRV_PCM_FORMAT_U8,          //unsigned 8 bits sample
/* OSS32_PCM_FORMAT_S16_LE */ SNDRV_PCM_FORMAT_S16_LE,      //signed 16 bits sample (little endian/Intel)
/* OSS32_PCM_FORMAT_S16_BE */ SNDRV_PCM_FORMAT_S16_BE,      //signed 16 bits sample (big endian/Motorola)
/* OSS32_PCM_FORMAT_U16_LE */ SNDRV_PCM_FORMAT_U16_LE,      //unsigned 16 bits sample (little endian/Intel)
/* OSS32_PCM_FORMAT_U16_BE */ SNDRV_PCM_FORMAT_U16_BE,      //unsigned 16 bits sample (big endian/Motorola)
/* OSS32_PCM_FORMAT_S24_LE */ SNDRV_PCM_FORMAT_S24_LE,      //signed 24 bits sample (little endian/Intel)
/* OSS32_PCM_FORMAT_S24_BE */ SNDRV_PCM_FORMAT_S24_BE,      //signed 24 bits sample (big endian/Motorola)
/* OSS32_PCM_FORMAT_U24_LE */ SNDRV_PCM_FORMAT_U24_LE,      //unsigned 24 bits sample (little endian/Intel)
/* OSS32_PCM_FORMAT_U24_BE */ SNDRV_PCM_FORMAT_U24_BE,      //unsigned 24 bits sample (big endian/Motorola)
/* OSS32_PCM_FORMAT_S32_LE */ SNDRV_PCM_FORMAT_S32_LE,      //signed 32 bits sample (little endian/Intel)
/* OSS32_PCM_FORMAT_S32_BE */ SNDRV_PCM_FORMAT_S32_BE,      //signed 32 bits sample (big endian/Motorola)
/* OSS32_PCM_FORMAT_U32_LE */ SNDRV_PCM_FORMAT_U32_LE,      //unsigned 32 bits sample (little endian/Intel)
/* OSS32_PCM_FORMAT_U32_BE */ SNDRV_PCM_FORMAT_U32_BE,      //unsigned 32 bits sample (big endian/Motorola)
/* OSS32_PCM_FORMAT_MULAW  */ SNDRV_PCM_FORMAT_MU_LAW,      //8 bps (compressed 16 bits sample)
/* OSS32_PCM_FORMAT_ALAW   */ SNDRV_PCM_FORMAT_A_LAW,       //8 bps (compressed 16 bits sample)
/* OSS32_PCM_FORMAT_ADPCM  */ SNDRV_PCM_FORMAT_IMA_ADPCM,   //4 bps (compressed 16 bits sample)
/* OSS32_PCM_FORMAT_MPEG   */ SNDRV_PCM_FORMAT_MPEG,        //AC3?
};

//******************************************************************************
//******************************************************************************
int register_chrdev(unsigned int version, const char *name, struct file_operations *fsop)
{
   if(!strcmp(name, "alsa")) {
       alsa_fops = fsop;
   }
   return 0;
}
//******************************************************************************
//******************************************************************************
int unregister_chrdev(unsigned int version, const char *name)
{
   if(!strcmp(name, "alsa")) {
       alsa_fops = NULL;
   }
   return 0;
}
//******************************************************************************
//******************************************************************************
int register_sound_special(struct file_operations *fops, int unit)
{
    if(fops == NULL) return -1;

    memcpy(&oss_devices[OSS32_SPECIALID], fops, sizeof(struct file_operations));
    return OSS32_SPECIALID;
}
//******************************************************************************
//******************************************************************************
int register_sound_mixer(struct file_operations *fops, int dev)
{
    if(fops == NULL) return -1;

    memcpy(&oss_devices[OSS32_MIXERID], fops, sizeof(struct file_operations));
    return OSS32_MIXERID;
}
//******************************************************************************
//******************************************************************************
int register_sound_midi(struct file_operations *fops, int dev)
{
    if(fops == NULL) return -1;

    memcpy(&oss_devices[OSS32_MIDIID], fops, sizeof(struct file_operations));
    return OSS32_MIDIID;
}
//******************************************************************************
//******************************************************************************
int register_sound_dsp(struct file_operations *fops, int dev)
{
    if(fops == NULL) return -1;

    memcpy(&oss_devices[OSS32_DSPID], fops, sizeof(struct file_operations));
    return OSS32_DSPID;
}
//******************************************************************************
//******************************************************************************
int register_sound_synth(struct file_operations *fops, int dev)
{
    if(fops == NULL) return -1;

    memcpy(&oss_devices[OSS32_SYNTHID], fops, sizeof(struct file_operations));
    return OSS32_SYNTHID;
}
//******************************************************************************
//******************************************************************************
void unregister_sound_special(int unit)
{
    memset(&oss_devices[OSS32_SPECIALID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
void unregister_sound_mixer(int unit)
{
    memset(&oss_devices[OSS32_MIXERID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
void unregister_sound_midi(int unit)
{
    memset(&oss_devices[OSS32_MIDIID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
void unregister_sound_dsp(int unit)
{
    memset(&oss_devices[OSS32_DSPID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
void unregister_sound_synth(int unit)
{
    memset(&oss_devices[OSS32_SYNTHID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
OSSRET UNIXToOSSError(int unixerror)
{
    switch(unixerror) {
    case 0:
        return OSSERR_SUCCESS;
    case -ENOMEM:
        return OSSERR_OUT_OF_MEMORY;
    case -ENODEV:
        return OSSERR_NO_DEVICE_AVAILABLE;
    case -ENOTTY:
    case -EINVAL:
        return OSSERR_INVALID_PARAMETER;
    case -EAGAIN:
        return OSSERR_AGAIN;    //????
    case -ENXIO:
        return OSSERR_IO_ERROR;
    case -EBUSY:
        return OSSERR_BUSY;
    case -EPERM:
        return OSSERR_ACCESS_DENIED; //??
    case -EPIPE:
    case -EBADFD:
        return OSSERR_ACCESS_DENIED; //??
    default:
        dprintf(("Unknown error %d", (unixerror > 0) ? unixerror : -unixerror));
        return OSSERR_UNKNOWN;
    }
}
//******************************************************************************
//ALSA to OSS32 datatype conversion
//******************************************************************************
int ALSAToOSSDataType(ULONG ALSADataType)
{
    switch(ALSADataType) 
    {
    case SNDRV_PCM_FORMAT_S8:
        return OSS32_CAPS_PCM_FORMAT_S8;          //signed 8 bits sample
    case SNDRV_PCM_FORMAT_U8:
        return OSS32_CAPS_PCM_FORMAT_U8;          //unsigned 8 bits sample
    case SNDRV_PCM_FORMAT_S16_LE:
        return OSS32_CAPS_PCM_FORMAT_S16_LE;      //signed 16 bits sample (little endian/Intel)
    case SNDRV_PCM_FORMAT_S16_BE:
        return OSS32_CAPS_PCM_FORMAT_S16_BE;      //signed 16 bits sample (big endian/Motorola)
    case SNDRV_PCM_FORMAT_U16_LE:
        return OSS32_CAPS_PCM_FORMAT_U16_LE;      //unsigned 16 bits sample (little endian/Intel)
    case SNDRV_PCM_FORMAT_U16_BE:
        return OSS32_CAPS_PCM_FORMAT_U16_BE;      //unsigned 16 bits sample (big endian/Motorola)
    case SNDRV_PCM_FORMAT_S24_LE:
        return OSS32_CAPS_PCM_FORMAT_S24_LE;      //signed 24 bits sample (little endian/Intel)
    case SNDRV_PCM_FORMAT_S24_BE:
        return OSS32_CAPS_PCM_FORMAT_S24_BE;      //signed 24 bits sample (big endian/Motorola)
    case SNDRV_PCM_FORMAT_U24_LE:
        return OSS32_CAPS_PCM_FORMAT_U24_LE;      //unsigned 24 bits sample (little endian/Intel)
    case SNDRV_PCM_FORMAT_U24_BE:
        return OSS32_CAPS_PCM_FORMAT_U24_BE;      //unsigned 16 bits sample (big endian/Motorola)
    case SNDRV_PCM_FORMAT_S32_LE:
        return OSS32_CAPS_PCM_FORMAT_S32_LE;      //signed 32 bits sample (little endian/Intel)
    case SNDRV_PCM_FORMAT_S32_BE:
        return OSS32_CAPS_PCM_FORMAT_S32_BE;      //signed 32 bits sample (big endian/Motorola)
    case SNDRV_PCM_FORMAT_U32_LE:
        return OSS32_CAPS_PCM_FORMAT_U32_LE;      //unsigned 32 bits sample (little endian/Intel)
    case SNDRV_PCM_FORMAT_U32_BE:
        return OSS32_CAPS_PCM_FORMAT_U32_BE;      //unsigned 32 bits sample (big endian/Motorola)
    case SNDRV_PCM_FORMAT_MU_LAW:
        return OSS32_CAPS_PCM_FORMAT_MULAW;       //8 bps (compressed 16 bits sample)
    case SNDRV_PCM_FORMAT_A_LAW:
        return OSS32_CAPS_PCM_FORMAT_ALAW;        //8 bps (compressed 16 bits sample)
    case SNDRV_PCM_FORMAT_IMA_ADPCM:
        return OSS32_CAPS_PCM_FORMAT_ADPCM;       //4 bps (compressed 16 bits sample)
    case SNDRV_PCM_FORMAT_MPEG:
        return OSS32_CAPS_PCM_FORMAT_MPEG;        //AC3?
    default:
        DebugInt3();
        return -1;
    }
}
//******************************************************************************
//******************************************************************************
ULONG ALSAToOSSRateFlags(ULONG fuRates)
{
    ULONG fuOSSRates = 0;

    if(fuRates & SNDRV_PCM_RATE_5512) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_5512;
    }
    if(fuRates & SNDRV_PCM_RATE_8000) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_8000;
    }
    if(fuRates & SNDRV_PCM_RATE_11025) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_11025;
    }
    if(fuRates & SNDRV_PCM_RATE_16000) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_16000;
    }
    if(fuRates & SNDRV_PCM_RATE_22050) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_22050;
    }
    if(fuRates & SNDRV_PCM_RATE_32000) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_32000;
    }
    if(fuRates & SNDRV_PCM_RATE_44100) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_44100;
    }
    if(fuRates & SNDRV_PCM_RATE_48000) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_48000;
    }
#if 0
    if(fuRates & SNDRV_PCM_RATE_64000) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_64000;
    }
    if(fuRates & SNDRV_PCM_RATE_88200) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_88200;
    }
    if(fuRates & SNDRV_PCM_RATE_96000) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_96000;
    }
    if(fuRates & SNDRV_PCM_RATE_176400) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_176400;
    }
    if(fuRates & SNDRV_PCM_RATE_192000) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_192000;
    }
#endif
    if(fuRates & SNDRV_PCM_RATE_CONTINUOUS) {
        fuOSSRates |= OSS32_CAPS_PCM_RATE_CONTINUOUS;
    }

    //TODO:
    if(fuRates & SNDRV_PCM_RATE_KNOT) {
        DebugInt3();
    }
//#define OSS32_CAPS_PCM_RATE_KNOT		(1<<31)		/* supports more non-continuos rates */

    return fuOSSRates;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_QueryDevCaps(ULONG deviceid, POSS32_DEVCAPS pDevCaps)
{
    OSSSTREAMID          streamid = 0;
    soundhandle         *pHandle;
    snd_pcm_info_t      *pcminfo = NULL;
    snd_pcm_hw_params_t *params;
    int                  ret, fmt, i;
    ULONG                format_mask;
    snd_mask_t           *mask;
    int max_ch;

#ifdef DEBUG
    dprintf(("OSS32_QueryDevCaps"));
#endif
//    max_ch = GetMaxChannels(deviceid, OSS32_CAPS_WAVE_PLAYBACK);

    //these structures are too big to put on the stack
    pcminfo = (snd_pcm_info_t *)kmalloc(sizeof(snd_pcm_info_t)+sizeof(snd_pcm_hw_params_t), GFP_KERNEL);
    if(pcminfo == NULL) {
        DebugInt3();
        printk("OSS32_QueryDevCaps: out of memory\n");
        return OSSERR_OUT_OF_MEMORY;
    }
    params = (snd_pcm_hw_params_t *)(pcminfo+1);

//    printk("Number of cards: %i\n",nrCardsDetected);
//    printk("dev id: %i\n",deviceid);
    pDevCaps->nrDevices  = 1;//nrCardsDetected;
    pDevCaps->ulCaps     = OSS32_CAPS_WAVE_PLAYBACK | OSS32_CAPS_WAVE_CAPTURE;

    //query wave in & out caps
    for(i=0;i<2;i++) 
    {
        PWAVE_CAPS pWaveCaps = (i == 0) ? &pDevCaps->waveOutCaps : &pDevCaps->waveInCaps;

        ret = OSS32_WaveOpen(deviceid, (i == 0) ? OSS32_STREAM_WAVEOUT : OSS32_STREAM_WAVEIN, &streamid, 0, 0);
        if(ret != OSSERR_SUCCESS) 
        {
            printk("OSS32_QueryDevCaps: wave open error %i\n", ret);
            DebugInt3();
            goto fail;
        }
        pHandle = (soundhandle *)streamid;
        if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
            printk("OSS32_QueryDevCaps: invalid stream id \n");
            ret = OSSERR_INVALID_STREAMID;
            goto fail;
        }
        //set operation to non-blocking
        pHandle->file.f_flags = O_NONBLOCK;
    
        ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_INFO, (ULONG)pcminfo);
        if(ret != 0) {
            printk("OSS32_QueryDevCaps: SNDRV_PCM_IOCTL_INFO error %i\n", ret);
            ret = UNIXToOSSError(ret);
            goto fail;
        }
        if(i == 0) {//only need to do this once
            if(pcminfo->name[0]) {
                 strncpy(pDevCaps->szDeviceName, pcminfo->name, sizeof(pDevCaps->szDeviceName));
            }
            else strncpy(pDevCaps->szDeviceName, pcminfo->id, sizeof(pDevCaps->szDeviceName));
        }
        printk("Device name: %s\n", pDevCaps->szDeviceName);
        pWaveCaps->nrStreams = pcminfo->subdevices_count;
    
        //get all hardware parameters
        _snd_pcm_hw_params_any(params);
        ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)params);
        if(ret != 0) {
            printk("OSS32_QueryDevCaps: SNDRV_PCM_IOCTL_HW_REFINE error %i\n", ret);
            ret = UNIXToOSSError(ret);
            goto fail;
        }

        pWaveCaps->ulMinChannels = hw_param_interval(params, SNDRV_PCM_HW_PARAM_CHANNELS)->min;
        pWaveCaps->ulMaxChannels = hw_param_interval(params, SNDRV_PCM_HW_PARAM_CHANNELS)->max;
        pWaveCaps->ulChanFlags   = 0;
        if(pWaveCaps->ulMinChannels == 1) {
            pWaveCaps->ulChanFlags |= OSS32_CAPS_PCM_CHAN_MONO;
        }
        if(pWaveCaps->ulMaxChannels >= 2) {
            pWaveCaps->ulChanFlags |= OSS32_CAPS_PCM_CHAN_STEREO;
        }
        if(pWaveCaps->ulMaxChannels >= 4) {
            pWaveCaps->ulChanFlags |= OSS32_CAPS_PCM_CHAN_QUAD;
        }
        if(pWaveCaps->ulMaxChannels >= 6) {
            pWaveCaps->ulChanFlags |= OSS32_CAPS_PCM_CHAN_5_1;
        }

        pWaveCaps->ulMinRate     = hw_param_interval(params, SNDRV_PCM_HW_PARAM_RATE)->min;
        pWaveCaps->ulMaxRate     = hw_param_interval(params, SNDRV_PCM_HW_PARAM_RATE)->max;

        mask = hw_param_mask(params, SNDRV_PCM_HW_PARAM_RATE_MASK);
        pWaveCaps->ulRateFlags   = mask->bits[0];

        pWaveCaps->ulRateFlags   = ALSAToOSSRateFlags(pWaveCaps->ulRateFlags);

        printk("rate flags: %X\n", pWaveCaps->ulRateFlags);

        pWaveCaps->ulDataFormats = 0;

        mask = hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT);
        format_mask = mask->bits[0];
        for(fmt=0;fmt<32;fmt++) 
        {
            if(format_mask & (1 << fmt)) 
            {
                int f = ALSAToOSSDataType(fmt);
                if (f >= 0)
                    pWaveCaps->ulDataFormats |= f;
            }
        }

        OSS32_WaveClose(streamid);
        streamid = 0;

    }
    //Check support for MPU401, FM & Wavetable MIDI
    if(OSS32_MidiOpen(deviceid, OSS32_STREAM_MPU401_MIDIOUT, &streamid) == OSSERR_SUCCESS) 
    {
        pDevCaps->ulCaps |= OSS32_CAPS_MPU401_PLAYBACK;
        OSS32_MidiClose(streamid);
        streamid = 0;
    }
    if(OSS32_MidiOpen(deviceid, OSS32_STREAM_MPU401_MIDIIN, &streamid) == OSSERR_SUCCESS) 
    {
        pDevCaps->ulCaps |= OSS32_CAPS_MPU401_CAPTURE;
        OSS32_MidiClose(streamid);
        streamid = 0;
    }
    if(OSS32_MidiOpen(deviceid, OSS32_STREAM_FM_MIDIOUT, &streamid) == OSSERR_SUCCESS) 
    {
        pDevCaps->ulCaps |= OSS32_CAPS_FMSYNTH_PLAYBACK;
        OSS32_MidiClose(streamid);
        streamid = 0;
    }
    if(OSS32_MidiOpen(deviceid, OSS32_STREAM_WAVETABLE_MIDIOUT, &streamid) == OSSERR_SUCCESS) 
    {
        pDevCaps->ulCaps |= OSS32_CAPS_WAVETABLE_PLAYBACK;
        OSS32_MidiClose(streamid);
        streamid = 0;
    }
    if(OSS32_MixQueryName(deviceid, &pDevCaps->szMixerName, sizeof(pDevCaps->szMixerName)) != OSSERR_SUCCESS) {
        DebugInt3();
        printk("OSS32_QueryDevCaps: OSS32_MixQueryName error\n");
        goto fail;
    }
//    printk("OSS32_QueryDevCaps: devname: [%s]\n", pDevCaps->szDeviceName);
    kfree(pcminfo);
    streamid = 0;


    return OSSERR_SUCCESS;

fail:
    printk("OSS32_QueryDevCaps failed\n");
    DebugInt3();
    if(streamid)   OSS32_WaveClose(streamid);
    if(pcminfo)    kfree(pcminfo);

    return ret;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveOpen(ULONG deviceid, ULONG streamtype, OSSSTREAMID *pStreamId, int pcm, USHORT fileid)
{
    soundhandle *pHandle;
    int          ret,i;

    if (pStreamId)
        *pStreamId = 0;
    else
    {
        printk("ERROR: invalid stream id pointer passed\n");
        return OSSERR_OUT_OF_MEMORY;
    }

    if(alsa_fops == NULL) {
        DebugInt3();
#ifdef DEBUG
        dprintf(("OSS32_WaveOpen: no devices"));
#endif
        printk("OSS32_WaveOpen: no devices\n");

        return OSSERR_NO_DEVICE_AVAILABLE;
    }

//    printk("dev id: %i\n",deviceid);

    pHandle = kmalloc(sizeof(soundhandle), GFP_KERNEL);
    if(pHandle == NULL) {
        DebugInt3();
        printk("OSS32_WaveOpen: out of memory\n");
        return OSSERR_OUT_OF_MEMORY;
    }
    memset(pHandle, 0, sizeof(soundhandle));

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //setup pointers in file structure (used internally by ALSA)
    pHandle->file.f_dentry          = &pHandle->d_entry;
    pHandle->file.f_dentry->d_inode = &pHandle->inode;

    switch(streamtype) {
    case OSS32_STREAM_WAVEOUT:
        pHandle->file.f_mode  = FMODE_WRITE;
        pHandle->inode.i_rdev = SNDRV_MINOR(deviceid, SNDRV_MINOR_PCM_PLAYBACK) + pcm;
        break;
    case OSS32_STREAM_WAVEIN:
        pHandle->file.f_mode  = FMODE_READ;
        pHandle->inode.i_rdev = SNDRV_MINOR(deviceid, SNDRV_MINOR_PCM_CAPTURE) + pcm;
        break;
    default:
        DebugInt3();
        kfree(pHandle);
        printk("OSS32_WaveOpen: invalid parameter\n");
        return OSSERR_INVALID_PARAMETER;
    }

    printk("OSS32_WaveOpen: cp1\n");
    ret = alsa_fops->open(&pHandle->inode, &pHandle->file);
    printk("OSS32_WaveOpen. ret: %i\n", ret);
    /* check if PCM already opened (stupid uniaud16.sys doesnt closes it) */
    if (ret == -16)
    {
        for (i=0; i < 8*256; i++)
        {
            if (opened_handles[i].handle != 0)
            {
                if (pStreamId)
                    *pStreamId = (ULONG)opened_handles[i].handle;
                opened_handles[i].reuse = 0; /* prevent from reusing */
                if (OSS32_WaveClose((OSSSTREAMID)opened_handles[i].handle) == 0)
                {
                    OSS32_CloseUNI16(); /* say to UNIAUD16 that we closing now */
                    opened_handles[i].handle = 0;
                    ret = alsa_fops->open(&pHandle->inode, &pHandle->file);
                    printk("OSS32_WaveOpen. Reopen ret: %i\n", ret);
                    break;
                }
            }
        }
    }
    printk("OSS32_WaveOpen: cp3\n");

    if(ret) {
        kfree(pHandle);
        DebugInt3();
        printk("OSS32_WaveOpen: open error: %i\n",ret);
        return UNIXToOSSError(ret);
    }
    pHandle->magic = MAGIC_WAVE_ALSA32;
    printk("OSS32_WaveOpen: cp4\n");

    if (pStreamId)
        *pStreamId = (ULONG)pHandle;
    // filling opened handles table
    for (i=0; i < 8*256; i++)
    {
        if (opened_handles[i].handle == 0)
        {
            opened_handles[i].handle = pHandle;
            opened_handles[i].FileId = fileid;
            break;
        }
    }
    printk("OSS32_WaveOpen. streamid %X\n",(ULONG)pHandle);
    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveClose(OSSSTREAMID streamid)
{
    soundhandle *pHandle = (soundhandle *)streamid;
    int          ret = 0, i;

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
        DebugInt3();
        printk("OSS32_WaveClose. invalid streamid %X\n",(ULONG)pHandle);
        return OSSERR_INVALID_STREAMID;
    }

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    for (i=0; i < 8*256; i++)
    {
        if (opened_handles[i].handle == pHandle)
        {
            printk("Found phandle for closing: %x reuse flag: %i\n", pHandle, opened_handles[i].reuse);
            if (!opened_handles[i].reuse)
            {
                ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
                opened_handles[i].handle = 0;
                kfree(pHandle);   //free handle data
            } else
            {
                /* prepare for reuse */
                pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PREPARE, 0);
            }
            break;
        }
    }

    if (i >= 8*256)
    {
        //all already closed
        printk("phandle %x not found\n", pHandle);
//        return OSSERR_SUCCESS;
    }


    if(ret) {
        printk("Error closing wave. rc = %i\n", ret);
        DebugInt3();
        return UNIXToOSSError(ret);
    }
    printk("OSS32_WaveClose. streamid %X\n",(ULONG)pHandle);
    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WavePrepare(OSSSTREAMID streamid)
{
    soundhandle    *pHandle = (soundhandle *)streamid;
    int             ret;

#ifdef DEBUG
    dprintf(("OSS32_WavePrepare"));
#endif

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
        DebugInt3();
//#ifdef DEBUG
        printk("vladest: OSS32_WavePrepare: invalid streamID\n");
//#endif

          return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PREPARE, 0);
    if (ret)
        printk("Wave prepare ret = %i, streamid %X\n",ret,(ULONG)pHandle);

    return UNIXToOSSError(ret);;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveStart(OSSSTREAMID streamid)
{
    soundhandle    *pHandle = (soundhandle *)streamid;
    int             ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
          DebugInt3();
          return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_START, 0);
    if (ret)
        printk("Wave start ret = %i, streamid %X\n",ret,(ULONG)pHandle);

    return UNIXToOSSError(ret);;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveStop(OSSSTREAMID streamid)
{
    soundhandle *pHandle = (soundhandle *)streamid;
    int          ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
          DebugInt3();
          return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_DROP, 0);
    if (ret)
        printk("Wave stop ret = %i. streamid %X\n",ret,(ULONG)pHandle);

    return UNIXToOSSError(ret);;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WavePause(OSSSTREAMID streamid)
{
    soundhandle *pHandle = (soundhandle *)streamid;
    int          ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
          DebugInt3();
          return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PAUSE, 0);
    if (ret)
        printk("Wave pause ret = %i, streamid %X\n",ret,(ULONG)pHandle);

    return UNIXToOSSError(ret);;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveResume(OSSSTREAMID streamid)
{
    soundhandle *pHandle = (soundhandle *)streamid;
    int          ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
          DebugInt3();
          return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PAUSE, 1);
    if (ret)
        printk("Wave resume ret = %i, streamid %X\n",ret,(ULONG)pHandle);

    return UNIXToOSSError(ret);;
}

static unsigned rates[] = {
    /* ATTENTION: these values depend on the definition in pcm.h! */
    5512, 8000, 11025, 16000, 22050, 32000, 44100, 48000,
    64000, 88200, 96000, 176400, 192000
};

//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveSetHwParams(OSSSTREAMID streamid, OSS32_HWPARAMS *pHwParams)
{
    soundhandle        *pHandle = (soundhandle *)streamid;
    snd_pcm_hw_params_t params;
    snd_pcm_status_t    status;
    snd_pcm_sw_params_t swparams;
    int                 ret, ret1, nrperiods, minnrperiods, maxnrperiods, samplesize, i;
    ULONG               bufsize, periodsize, minperiodsize, maxperiodsize;
    ULONG               periodbytes, minperiodbytes, maxperiodbytes;
    BOOL                fTryAgain = FALSE;
    ULONG ulMinRate, ulMaxRate;

#ifdef DEBUG
    dprintf(("OSS32_WaveSetHwParams"));
#endif
    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
        printk("OSS32_WaveSetHwParams error. Invalid handle\n");
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    if(pHwParams == NULL) {
        printk("OSS32_WaveSetHwParams error. params = NULL\n");
        DebugInt3();
        return OSSERR_INVALID_PARAMETER;
    }
    if(pHwParams->ulDataType >= OSS32_PCM_MAX_FORMATS) {
        printk("OSS32_WaveSetHwParams error. Too high PCM format\n");
        DebugInt3();
        return OSSERR_INVALID_PARAMETER;
    }
    if ((int)pHwParams->ulNumChannels <= 0) {
        printk("OSS32_WaveSetHwParams error. Invalid number of channels: %i\n", pHwParams->ulNumChannels);
        DebugInt3();
        return OSSERR_INVALID_PARAMETER;
    }
tryagain:
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;
    //size of two samples (adpcm sample can be as small as 4 bits (mono), so take two)
    samplesize = snd_pcm_format_size(OSSToALSADataType[pHwParams->ulDataType], 1);
    pHandle->doublesamplesize  = samplesize * 2;
    pHandle->doublesamplesize *= pHwParams->ulNumChannels;
    periodbytes = pHwParams->ulPeriodSize;
    periodsize  = bytes_to_samples(periodbytes);
    // checking number of channels

    printk("channels: %i, period size: %i\n",pHwParams->ulNumChannels, periodbytes);
    _snd_pcm_hw_params_any(&params);
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)__Stack32ToFlat(&params));
    ulMinRate     = hw_param_interval(&params, SNDRV_PCM_HW_PARAM_RATE)->min;
    ulMaxRate     = hw_param_interval(&params, SNDRV_PCM_HW_PARAM_RATE)->max;

    _snd_pcm_hw_params_any(&params);
    do {
        _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_ACCESS,
                              SNDRV_PCM_ACCESS_RW_INTERLEAVED, 0);
        _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_CHANNELS,
                              pHwParams->ulNumChannels, 0);
        ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)__Stack32ToFlat(&params));
        if (ret == 0) break;
        pHwParams->ulNumChannels--;
    } while(ret < 0 && pHwParams->ulNumChannels > 1);

    printk("channels selected: %i\n",pHwParams->ulNumChannels);

    //get all hardware parameters
    _snd_pcm_hw_params_any(&params);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_ACCESS,
                          SNDRV_PCM_ACCESS_RW_INTERLEAVED, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_FORMAT,
                          OSSToALSADataType[pHwParams->ulDataType], 0);
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)__Stack32ToFlat(&params));
    if(ret != 0) {
        printk("invalid format %i\n", OSSToALSADataType[pHwParams->ulDataType]);
        return UNIXToOSSError(ret);
    }
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
                          pHwParams->ulBitsPerSample, 0);
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)__Stack32ToFlat(&params));
    if(ret != 0) {
        printk("invalid number of sample bits %i\n", pHwParams->ulBitsPerSample);
        return UNIXToOSSError(ret);
    }
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
                          pHwParams->ulBitsPerSample*pHwParams->ulNumChannels, 0);
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)__Stack32ToFlat(&params));
    if(ret != 0) {
        printk("invalid number of frame bits %i\n", pHwParams->ulBitsPerSample*pHwParams->ulNumChannels);
        return UNIXToOSSError(ret);
    }

    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
                          pHwParams->ulBitsPerSample, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
                          pHwParams->ulBitsPerSample*pHwParams->ulNumChannels, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_FORMAT,
                          OSSToALSADataType[pHwParams->ulDataType], 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_CHANNELS,
                           pHwParams->ulNumChannels, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_RATE,
                          pHwParams->ulSampleRate, 0);

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)__Stack32ToFlat(&params));
    if(ret != 0) {
        printk("OSS32_WaveSetHwParams (first pass) error %i\n", ret);
        printk("bps: %i\n",pHwParams->ulBitsPerSample);
        printk("format: %i\n",OSSToALSADataType[pHwParams->ulDataType]);
        printk("channels: %i\n",pHwParams->ulNumChannels);
        printk("sample rate: %i\n",pHwParams->ulSampleRate);

//        printk("OSS32_WaveSetHwParams invalid sample rate %i\n", pHwParams->ulSampleRate);
        //        printk("will set to nearest\n");
        _snd_pcm_hw_params_any(&params);
        for (i=0; i<(sizeof(rates)/sizeof(unsigned))-1;i++)
        {
            if (pHwParams->ulSampleRate >= rates[i] &&
                pHwParams->ulSampleRate <= rates[i+1])
            {
                pHwParams->ulSampleRate = rates[i];
                _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_RATE,
                                      pHwParams->ulSampleRate, 0);
                ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)__Stack32ToFlat(&params));
                if(ret == 0)
                {
                    _snd_pcm_hw_params_any(&params);
                    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
                                          pHwParams->ulBitsPerSample, 0);
                    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
                                          pHwParams->ulBitsPerSample*pHwParams->ulNumChannels, 0);
                    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_FORMAT,
                                          OSSToALSADataType[pHwParams->ulDataType], 0);
                    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_CHANNELS,
                                          pHwParams->ulNumChannels, 0);
                    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_RATE,
                                          pHwParams->ulSampleRate, 0);
                    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)__Stack32ToFlat(&params));
                    goto __next;
                }
            }
        }

//        ret1 = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        DebugInt3();
        return UNIXToOSSError(ret);
    }
__next:

    printk("sample rate: %i\n",pHwParams->ulSampleRate);

    //check period size against lower and upper boundaries
    minperiodbytes = hw_param_interval((&params), SNDRV_PCM_HW_PARAM_PERIOD_BYTES)->min;
    maxperiodbytes = hw_param_interval((&params), SNDRV_PCM_HW_PARAM_PERIOD_BYTES)->max;
    if(periodbytes < minperiodbytes) {
        periodbytes = minperiodbytes;
    }
    else
    if(periodbytes > maxperiodbytes) {
        periodbytes = maxperiodbytes;
    }

    minperiodsize = hw_param_interval((&params), SNDRV_PCM_HW_PARAM_PERIOD_SIZE)->min;
    maxperiodsize = hw_param_interval((&params), SNDRV_PCM_HW_PARAM_PERIOD_SIZE)->max;
    if(periodsize < minperiodsize) {
        periodsize = minperiodsize;
    }
    else
    if(periodsize > maxperiodsize) {
        periodsize = maxperiodsize;
    }

    if(samples_to_bytes(periodsize) < periodbytes) {
        periodbytes = samples_to_bytes(periodsize);
    }
    else
    if(bytes_to_samples(periodbytes) < periodsize) {
        periodsize = bytes_to_samples(periodbytes);
    }

    //make sure period size is a whole fraction of the buffer size
    bufsize = hw_param_interval((&params), SNDRV_PCM_HW_PARAM_BUFFER_BYTES)->max;
    if(periodsize) {
        nrperiods = bufsize/periodbytes;
    }
    else {
        printk("OSS32_WaveSetHwParams error. Invalid periodsize (=0). closing file\n");
//        ret1 = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        DebugInt3();
        return OSSERR_INVALID_PARAMETER;
    }
    //check nr of periods against lower and upper boundaries
    minnrperiods = hw_param_interval((&params), SNDRV_PCM_HW_PARAM_PERIODS)->min;
    maxnrperiods = hw_param_interval((&params), SNDRV_PCM_HW_PARAM_PERIODS)->max;
    if(nrperiods < minnrperiods) {
        nrperiods = minnrperiods;
    }
    else
    if(nrperiods > maxnrperiods) {
        nrperiods = maxnrperiods;
    }
    //an odd nr of periods is not always a good thing (CMedia -> clicks during 8 bps playback),
    //so we make sure it's an even number.
    if(nrperiods == 1) {
        DebugInt3();
        printk("OSS32_WaveSetHwParams error. Invalid number of periods(=1). closing file\n");
//        ret1 = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        return OSSERR_INVALID_PARAMETER;
    }
    nrperiods &= ~1;

    //initialize parameter block & set sample rate, nr of channels and sample format, nr of periods,
    //period size and buffer size
//tryagain:
    _snd_pcm_hw_params_any(&params);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_ACCESS,
                          SNDRV_PCM_ACCESS_RW_INTERLEAVED, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
                          pHwParams->ulBitsPerSample, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
                          pHwParams->ulBitsPerSample*pHwParams->ulNumChannels, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_FORMAT,
                          OSSToALSADataType[pHwParams->ulDataType], 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_CHANNELS,
                           pHwParams->ulNumChannels, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_RATE,
                          pHwParams->ulSampleRate, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, 
                          periodsize, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, 
                          periodbytes, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_PERIODS, 
                          nrperiods, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_BUFFER_SIZE, 
                          periodsize*nrperiods, 0);
    _snd_pcm_hw_param_set(&params, SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 
                          periodbytes*nrperiods, 0);
//#ifdef DEBUG_PK
    printk("Hardware parameters: sample rate %i, data type %i, channels %i, period size %i, periods %i\n",
             pHwParams->ulSampleRate, pHwParams->ulDataType, pHwParams->ulNumChannels, periodsize, nrperiods);
//#endif
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_PARAMS, (ULONG)__Stack32ToFlat(&params));
    if (ret == -77 && fTryAgain == FALSE)
    {
        ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PREPARE, 0);
        fTryAgain = TRUE;
        goto tryagain;
    }
    if(ret) {
        if(fTryAgain == FALSE) {
            minperiodsize = hw_param_interval((&params), SNDRV_PCM_HW_PARAM_PERIOD_SIZE)->min;
            maxperiodsize = hw_param_interval((&params), SNDRV_PCM_HW_PARAM_PERIOD_SIZE)->max;
            if(minperiodsize > maxperiodsize) {
                //ALSA doesn't like the period size; try suggested one
                periodsize  = maxperiodsize;
                periodbytes = samples_to_bytes(periodsize);
                fTryAgain = TRUE;
                goto tryagain;
            }
        }
        printk("OSS32_WaveSetHwParams (second pass) error %i\n", ret);
        printk("bps: %i\n",pHwParams->ulBitsPerSample);
        printk("format: %i\n",OSSToALSADataType[pHwParams->ulDataType]);
        printk("channels: %i\n",pHwParams->ulNumChannels);
        printk("sample rate: %i\n",pHwParams->ulSampleRate);
        printk("periodsize: %i\n",periodsize);
        printk("periodbytes: %i\n",periodbytes);
        printk("periods: %i\n",nrperiods);
//        ret1 = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        return UNIXToOSSError(ret);
    }

    //set silence threshold (all sizes in frames) (only needed for playback)
    if(pHandle->file.f_mode == FMODE_WRITE) 
    {
        swparams.avail_min         = periodsize;
        swparams.period_step       = 1;
        if(nrperiods <= 2) {
            swparams.silence_size  = (periodsize/2);
        }
        else {
            swparams.silence_size  = periodsize;
        }
        swparams.silence_threshold = swparams.silence_size;
        swparams.sleep_min         = 0;
        swparams.start_threshold   = 1;
        swparams.stop_threshold    = periodsize*nrperiods;
        swparams.tstamp_mode       = SNDRV_PCM_TSTAMP_NONE;
        swparams.xfer_align        = periodsize;

        ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_SW_PARAMS, (ULONG)__Stack32ToFlat(&swparams));
    }
    total = 0;
    per_bytes = periodbytes;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_STATUS, (ULONG)__Stack32ToFlat(&status));
    if ( ((status.state != SNDRV_PCM_STATE_PREPARED) &&
          (status.state != SNDRV_PCM_STATE_RUNNING) &&
          (status.state != SNDRV_PCM_STATE_DRAINING))) {
        printk("Device is not in proper state: %i. Calling prepare\n", status.state);
        ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PREPARE, 0);
    }
    printk("OSS32_WaveSetHwParams return %d after SNDRV_PCM_IOCTL_SW_PARAMS ioctl, streamid %X", ret,(ULONG)pHandle);
    return UNIXToOSSError(ret);
}

//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveAddBuffer(OSSSTREAMID streamid, ULONG buffer, ULONG size, ULONG *pTransferred, int pcm)
{
    soundhandle        *pHandle = (soundhandle *)streamid;
    snd_pcm_status_t    status;
    int                 ret, align, size1, ret1;
    LONG                transferred;
    ULONG               position;
    char                *buf;
    int toret = 0;

//    return OSSERR_SUCCESS;

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    if(pTransferred == NULL || buffer == 0 || size == 0) {
        DebugInt3();
        return OSSERR_INVALID_PARAMETER;
    }

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //first check how much room is left in the circular dma buffer
    //this is done to make sure we don't block inside ALSA while trying to write
    //more data than fits in the internal dma buffer.
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_STATUS, (ULONG)__Stack32ToFlat(&status));

    if(ret) {
        DebugInt3();
        return UNIXToOSSError(ret);
    }

    size1 = size;

    size = min(size, samples_to_bytes(status.avail));
    if (size1 != size)
        printk("requested size [%i] less then available [%s]\n", size1, size);
#if 0
    if (size < per_bytes)
    { /*HACK!!!*/
        printk("HACK avoid buffer overrun: requested buffer [%i] less than period [%i]. prev size: %i\n", size, per_bytes, prev_size);
        *pTransferred = size;
        prev_size = size;
        ret1 = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PREPARE, 0);
        return OSSERR_SUCCESS;
    }
#endif
//    size = per_bytes * (size / per_bytes);

#ifdef DEBUG_PK
    printk("OSS32_WaveAddBuffer hw %i app %i avail %i, orig size: %i, size %i align %i\n", samples_to_bytes(status.hw_ptr), samples_to_bytes(status.appl_ptr), samples_to_bytes(status.avail), size1, size, per_bytes);
#endif

    if(size == 0) {
#ifdef DEBUG_PK
        printk("OSS32_WaveAddBuffer: no room left in hardware buffer!!\n");
        printk("state = %i\n",status.state);
        printk("avail = %i\n",status.avail);
        printk("size req = %i\n",size1);
#endif
        *pTransferred = 0;
        return OSSERR_BUFFER_FULL;
    }

    transferred = 0;
    switch(SNDRV_MINOR_DEVICE(MINOR(pHandle->inode.i_rdev-pcm)))
    {
    case SNDRV_MINOR_PCM_PLAYBACK:
        // size should be aligned to channels number * samplesize

        while(TRUE) {
        again:
            ret = pHandle->file.f_op->write(&pHandle->file, (char *)buffer, size, &pHandle->file.f_pos);
            if(ret < 0) {
                // check EAGAIN
                if (ret == -EPIPE)
                {
                    ret1 = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PREPARE, 0);
//                    if (ret1 == 0) continue;
//                    printk("prepare rc %i\n",ret);
                }

                if(transferred > 0) {
                    printk("OSS32_WaveAddBuffer failed on partial transfer %x %i; ret = %i\n", buffer, size, ret);
                    *pTransferred = transferred;
                    if (toret)
                        return OSSERR_SUCCESS; /* avoid infinite loop */
                    toret = 1;
                    goto again;
                }

                ret1 = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_STATUS, (ULONG)__Stack32ToFlat(&status));

                if ( ((status.state != SNDRV_PCM_STATE_PREPARED) &&
                      (status.state != SNDRV_PCM_STATE_RUNNING) &&
                      (status.state != SNDRV_PCM_STATE_DRAINING)) || ret == -32 ) {
                    ret1 = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PREPARE, 0);
                    printk("OSS32_WaveAddBuffer buffer overrun: size %i, ret %i, trans %i, prev sz %i per sz %i total %i\n", size, ret, transferred, prev_size, per_bytes, total);
                    //ret = size;
                    //transferred = size;
                    if (toret) break; /* avoid infinite loop */
                    toret = 1;
                    goto again;
                }
                else {
                    printk("OSS32_WaveAddBuffer failed when SNDRV_MINOR_PCM_PLAYBACK. rc = %i\n",ret);
                    printk("hw %i app %i avail %i, size %i\n", samples_to_bytes(status.hw_ptr), samples_to_bytes(status.appl_ptr), samples_to_bytes(status.avail), size);
                    printk("total = %i\n",total);
                    printk("state = %i\n",status.state);
                    printk("avail = %i\n",status.avail);
                    printk("size req = %i\n",size1);
                    DebugInt3();
                    return UNIXToOSSError(transferred);
                }
            }
            if(ret == 0) {
                break;
            }
            toret = 0;
            transferred += ret;
//            printk("written: now: %i, buffer: %i, total: %i\n", ret, transferred, total);
            buffer += ret;
            size   -= ret;
            if((int)size <= 0) {
                break;
            }
        }
        break;
    case SNDRV_MINOR_PCM_CAPTURE:
        transferred = pHandle->file.f_op->read(&pHandle->file, (char *)buffer, size, &pHandle->file.f_pos);
//        printk("readed %i bytes\n", transferred);
        break;
    default:
        DebugInt3();
        return OSSERR_INVALID_PARAMETER;
    }
    if(ret < 0) {
        dprintf(("OSS32_WaveAddBuffer failed"));
        DebugInt3();
        return UNIXToOSSError(transferred);
    }

    total+=transferred;
    *pTransferred = transferred;
    prev_size = transferred;
    if (transferred < size)
    {
        printk("warning: transferred [%i] less than requested [%i]\n", transferred, size);
        ret1 = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PREPARE, 0);
    }

    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveGetPosition(ULONG streamid, ULONG *pPosition)
{
    soundhandle        *pHandle = (soundhandle *)streamid;
    snd_pcm_status_t    status;
    int                 ret;
    ULONG               delta;

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    if(pPosition == NULL) {
        DebugInt3();
        return OSSERR_INVALID_PARAMETER;
    }

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //Get the status of the stream
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_STATUS, (ULONG)__Stack32ToFlat(&status));

    if(ret) {
        DebugInt3();
        return UNIXToOSSError(ret);
    }

    dprintf2(("OSS32_WaveGetPosition: hardware %x application %x", samples_to_bytes(status.hw_ptr), samples_to_bytes(status.appl_ptr)));
    *pPosition = samples_to_bytes(status.hw_ptr);  //return new hardware position
    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveGetSpace(ULONG streamid, ULONG *pBytesAvail)
{
    soundhandle        *pHandle = (soundhandle *)streamid;
    snd_pcm_status_t    status;
    int                 ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    if(pBytesAvail == NULL) {
        DebugInt3();
        return OSSERR_INVALID_PARAMETER;
    }

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //Get the nr of bytes left in the audio buffer
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_STATUS, (ULONG)__Stack32ToFlat(&status));

    if(ret) {
        DebugInt3();
        return UNIXToOSSError(ret);
    }
    *pBytesAvail = samples_to_bytes(status.avail);
    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_WaveSetVolume(OSSSTREAMID streamid, ULONG volume)
{
    soundhandle *pHandle = (soundhandle *)streamid;
    int          ret;
    int          leftvol, rightvol;
    struct snd_pcm_volume pcm_volume;

    if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
          DebugInt3();
          return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    leftvol  = GET_VOLUME_L(volume);
    rightvol = GET_VOLUME_R(volume);

    pcm_volume.nrchannels = 4;
    pcm_volume.volume[SNDRV_PCM_VOL_FRONT_LEFT]  = leftvol;
    pcm_volume.volume[SNDRV_PCM_VOL_FRONT_RIGHT] = rightvol;
    pcm_volume.volume[SNDRV_PCM_VOL_REAR_LEFT]   = leftvol;
    pcm_volume.volume[SNDRV_PCM_VOL_REAR_RIGHT]  = rightvol;

    dprintf(("OSS32_WaveSetVolume %x to (%d,%d)", streamid, leftvol, rightvol));
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_SETVOLUME, (ULONG)__Stack32ToFlat(&pcm_volume));
    return UNIXToOSSError(ret);
}
//******************************************************************************
//******************************************************************************

