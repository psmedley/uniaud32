/*

 IOCTL funcs
 Written by Vlad Stelmahovsky with help from Stepan Kazakov

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
#include <stacktoflat.h>
#include <stdlib.h>
#include "soundoss.h"

POSS32_DEVCAPS pcmcaps[8] = {0,0,0,0,0,0,0,0};
extern int pcm_device;
extern OpenedHandles opened_handles[8*256];

/*
 * Control ID changed by an external
 * set by snd_ctl_notify()
 */
snd_pcm_substream_t *substream_int[8*256] = {0}; // interrupted substream
int control_id_changed = 0;
int card_id_changed = 0;
int unlock_all = 0;
int GetUniaudPcmCaps(ULONG deviceid, void *caps);
void FillCaps(ULONG deviceid);
int pcm_instances(int card_id);

int WaitForControlChange(int card_id, int timeout)
{
    int ctl_id;
    ULONG blkid = 0;
    int i = 0;

    while (1)
    {
        if (control_id_changed != 0 && card_id_changed == card_id)
        {
            ctl_id = control_id_changed;
            control_id_changed = 0;
            break;
        }
        i++;

        if (i > timeout)
            return -ETIME;

        MyDevBlock(blkid, 1, 0);
        if (unlock_all)
        {
            unlock_all = 0;
            break;
        }
    }
    return ctl_id;
}

int WaitForPCMInterrupt(void *handle, int timeout)
{
    ULONG blkid = 0;
    int i = 0;
    int j = 0;
    snd_pcm_file_t *pcm_file = NULL;
    snd_pcm_substream_t *substream;
    snd_pcm_runtime_t *runtime;
    soundhandle        *pHandle = (soundhandle *)handle;
    struct file *pfile;

    if (handle == NULL)
        return -ENXIO;

    pfile = &pHandle->file;

    if (pfile == NULL)
        return -ENXIO;

    pcm_file = (snd_pcm_file_t *)pfile->private_data;

    if (pcm_file == NULL)
        return -ENXIO;

    substream = pcm_file->substream;

    if (substream == NULL)
        return -ENXIO;

    runtime = substream->runtime;

    if ((pfile->f_mode == FMODE_WRITE) &&
        (runtime->status->state != SNDRV_PCM_STATE_RUNNING))
        return -EBADFD;

    //printk("wait for %x. tout %i\n",substream, timeout);

    while (1)
    {
        for (i=0; i < 8*256; i++)
        {
            if (substream_int[i] == substream)
            {
//                printk("found %x at %i\n",substream_int[i], i);
                substream_int[i] = 0;
                //printk("j =%i\n",j);
                return j; /* milliseconds */
            }
        }

        if (j++ > timeout)
        {
            printk("j with tout =%i\n",j);
            return -ETIME;
        }

        MyDevBlock(blkid, 1, 0);
        if (unlock_all)
        {
            unlock_all = 0;
            break;
        }
    }
    //printk("j at exit =%i\n",j);
    return j;
}

/*
 returns number of registered cards
 */
int GetNumberOfCards(void)
{
    return nrCardsDetected;
}

/*
 returns number of registered pcm instances
 */
int GetNumberOfPcm(int card_id)
{
    return pcm_instances(card_id);
}

int SetPCMInstance(int card_id, int pcm)
{
    if (pcm>=0 && pcm <= pcm_instances(card_id))
    {
        pcm_device = pcm;
        return pcm;
    } else
        return pcm_device;
}

int GetPcmForChannels(ULONG deviceid, int type, int channels)
{
    POSS32_DEVCAPS pcaps = NULL;
    WAVE_CAPS *wc;
    int i;
    int sel_pcm = -1;

    if (!pcmcaps[deviceid])
    {
        FillCaps(deviceid);
        if (!pcmcaps[deviceid])
        {
            printk("Error querying caps for device: %i\n", deviceid);
            return -1;
        }
    }

    pcaps = pcmcaps[deviceid];

    for (i=0; i<pcm_instances(deviceid);i++)
    {
        switch(type)
        {
        case OSS32_CAPS_WAVE_PLAYBACK: // play
            wc = &pcaps->waveOutCaps;
            break;
        case OSS32_CAPS_WAVE_CAPTURE: // record
            wc = &pcaps->waveInCaps;
            break;
        }
        if (wc->ulMaxChannels == channels)
        {
            sel_pcm = i;
            break;
        }
        pcaps++;
    }

    return sel_pcm;
}

int GetMaxChannels(ULONG deviceid, int type)
{
    POSS32_DEVCAPS pcaps = NULL;
    WAVE_CAPS *wc;
    int i;
    int sel_pcm = -1;
    int max_ch = 0;

    if (!pcmcaps[deviceid])
    {
        FillCaps(deviceid);
        if (!pcmcaps[deviceid])
        {
            printk("Error querying caps for device: %i\n", deviceid);
            return -1;
        }
    }

    pcaps = pcmcaps[deviceid];

    for (i=0; i<pcm_instances(deviceid);i++)
    {
        switch(type)
        {
        case OSS32_CAPS_WAVE_PLAYBACK: // play
            wc = &pcaps->waveOutCaps;
            break;
        case OSS32_CAPS_WAVE_CAPTURE: // record
            wc = &pcaps->waveInCaps;
            break;
        }
        if (wc->ulMaxChannels > max_ch)
            max_ch = wc->ulMaxChannels;
        pcaps++;
    }
    return max_ch;
}

void FillCaps(ULONG deviceid)
{
    int pcms = 0;

    pcms = pcm_instances(deviceid);

//    printk("pcms = %i\n", pcms);
    if (!pcmcaps[deviceid])
    {
        pcmcaps[deviceid] = (POSS32_DEVCAPS)kmalloc(sizeof(OSS32_DEVCAPS)*pcms, GFP_KERNEL);
        if (pcmcaps[deviceid])
        {
            memset(pcmcaps[deviceid], 0, sizeof(OSS32_DEVCAPS)*pcms);
            GetUniaudPcmCaps1(deviceid, (void *)pcmcaps[deviceid]);
        }
    }
    return;
}

int GetUniaudPcmCaps(ULONG deviceid, void *caps)
{
    int pcms = 0;

    pcms = pcm_instances(deviceid);

//    printk("pcms = %i\n", pcms);
    if (pcmcaps[deviceid])
    {
        memcpy((unsigned char*)caps,(unsigned char*)pcmcaps[deviceid],sizeof(OSS32_DEVCAPS)*pcms);
        return 0;
    }
    else
    {
        return -1;
    }
}

/*
 returns pcm caps
 */
int GetUniaudPcmCaps1(ULONG deviceid, void *caps)
{
    POSS32_DEVCAPS pcaps = (POSS32_DEVCAPS)caps;
    int i;
    OSSSTREAMID          streamid = 0;
    soundhandle         *pHandle;
    snd_pcm_info_t      *pcminfo = NULL;
    snd_pcm_hw_params_t *params;
    int                  ret, fmt, j;
    ULONG                format_mask;
    snd_mask_t           *mask;
    int pcms = 0;

    pcms = pcm_instances(deviceid);

    if (!pcaps || !pcms) return -1;

    
    //these structures are too big to put on the stack
    pcminfo = (snd_pcm_info_t *)kmalloc(sizeof(snd_pcm_info_t)+sizeof(snd_pcm_hw_params_t), GFP_KERNEL);
    if(pcminfo == NULL) {
        DebugInt3();
        printk("GetUniaudPcmCaps: out of memory\n");
        return OSSERR_OUT_OF_MEMORY;
    }
    params = (snd_pcm_hw_params_t *)(pcminfo+1);

    for (i=0; i<pcms;i++)
    {
        pcaps->nrDevices  = nrCardsDetected;
        pcaps->ulCaps     = OSS32_CAPS_WAVE_PLAYBACK | OSS32_CAPS_WAVE_CAPTURE;

        //query wave in & out caps
        for(j=0;j<2;j++)
        {
            PWAVE_CAPS pWaveCaps = (j == 0) ? &pcaps->waveOutCaps : &pcaps->waveInCaps;

            ret = OSS32_WaveOpen(deviceid, (j == 0) ? OSS32_STREAM_WAVEOUT : OSS32_STREAM_WAVEIN, &streamid, i, 0);
            if(ret != OSSERR_SUCCESS)
            {
                printk("GetUniaudPcmCaps: wave open error %i %s at pcm %i\n", ret,
                       (j == 0) ?"PLAY":"REC", i);
                continue;
//                goto fail;
            }
            pHandle = (soundhandle *)streamid;
            if(pHandle == NULL || pHandle->magic != MAGIC_WAVE_ALSA32) {
                printk("GetUniaudPcmCaps: invalid stream id \n");
                ret = OSSERR_INVALID_STREAMID;
//                goto fail;
            }

            //set operation to non-blocking
            pHandle->file.f_flags = O_NONBLOCK;

            ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_INFO, (ULONG)pcminfo);
            if(ret != 0) {
                printk("GetUniaudPcmCaps: SNDRV_PCM_IOCTL_INFO error %i\n", ret);
                ret = UNIXToOSSError(ret);
                continue;
            }
            if(pcminfo->name[0]) {
                strncpy(pcaps->szDeviceName, pcminfo->name, sizeof(pcaps->szDeviceName));
            }
            else strncpy(pcaps->szDeviceName, pcminfo->id, sizeof(pcaps->szDeviceName));

            if(pcminfo->subname[0]) {
                strncpy(pcaps->szMixerName, pcminfo->subname, sizeof(pcaps->szMixerName));
            }

            pWaveCaps->nrStreams = pcminfo->subdevices_count;

            //get all hardware parameters
            _snd_pcm_hw_params_any(params);
            ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)params);
            if(ret != 0) {
                printk("GetUniaudPcmCaps: SNDRV_PCM_IOCTL_HW_REFINE error %i\n", ret);
                ret = UNIXToOSSError(ret);
                //goto fail;
                continue;
            }

            pWaveCaps->ulMinChannels = hw_param_interval(params, SNDRV_PCM_HW_PARAM_CHANNELS)->min;
            pWaveCaps->ulMaxChannels = hw_param_interval(params, SNDRV_PCM_HW_PARAM_CHANNELS)->max;
            printk("chan: from %i to %i\n", pWaveCaps->ulMinChannels,pWaveCaps->ulMaxChannels);
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
        } // for j
        pcaps++; // next pcm
    } // for i

    if (pcminfo) kfree(pcminfo);
    return OSSERR_SUCCESS;
}

/*
 returns power state of given card
 */
int UniaudCtlGetPowerState(ULONG deviceid, void *state)
{
    mixerhandle *pHandle = NULL;
    int          ret, i, j;

    if(alsa_fops == NULL) {
        ret = OSSERR_NO_DEVICE_AVAILABLE;
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
        goto failure;
    }
    //retrieve mixer information
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_POWER_STATE, 
                                    (ULONG)state);

    pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);

    kfree(pHandle);

    if(ret) {
        goto failure;
    }

    return OSSERR_SUCCESS;
failure:
    DebugInt3();
    return ret;
}

/*
 sets power state for given card
 */
int UniaudCtlSetPowerState(ULONG deviceid, void *state)
{
    mixerhandle *pHandle = NULL;
    int          ret, i, j;

    if(alsa_fops == NULL) {
        ret = OSSERR_NO_DEVICE_AVAILABLE;
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
        goto failure;
    }
    //retrieve mixer information
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_POWER, 
                                    (ULONG)state);

    pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);

    kfree(pHandle);

    if(ret) {
        goto failure;
    }

    return OSSERR_SUCCESS;
failure:
    DebugInt3();
    return ret;
}

/*
 returns card info
 */
int GetUniaudCardInfo(ULONG deviceid, void *info)
{
    mixerhandle *pHandle = NULL;
    int          ret, i, j;
//    snd_ctl_card_info_t *pinfo;

    if(alsa_fops == NULL) {
        ret = OSSERR_NO_DEVICE_AVAILABLE;
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
        goto failure;
    }
    //retrieve mixer information
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_CARD_INFO, 
                                    (ULONG)(snd_ctl_card_info_t *)info);
    if(ret) {
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        goto failure;
    }
    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);

    if(pHandle) {
        kfree(pHandle);
    }
    return OSSERR_SUCCESS;
failure:
    if(pHandle) {
        kfree(pHandle);
    }
    DebugInt3();
    return OSSERR_OUT_OF_MEMORY;
}

int GetUniaudControlNum(ULONG deviceid)
{
    mixerhandle *pHandle = NULL;
    int          ret, i, j, sz;

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
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        goto failure;
    }
    //get the number of mixer elements
    pHandle->list.offset = 0;
    pHandle->list.space  = 0;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_ELEM_LIST, 
                                    (ULONG)&pHandle->list);
    if(ret) {
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        goto failure;
    }
    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
    if(pHandle) {
        if(pHandle->pids) kfree(pHandle->pids);
        kfree(pHandle);
    }
    return pHandle->list.count;
failure:
    if(pHandle) {
        if(pHandle->pids) kfree(pHandle->pids);
        kfree(pHandle);
    }
    DebugInt3();
    return OSSERR_OUT_OF_MEMORY;
}

int GetUniaudControls(ULONG deviceid, void *pids)
{
    mixerhandle *pHandle = NULL;
    int          ret, i, j, sz;

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
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        goto failure;
    }
    //get the number of mixer elements
    pHandle->list.offset = 0;
    pHandle->list.space  = 0;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, 
                                    SNDRV_CTL_IOCTL_ELEM_LIST, 
                                    (ULONG)&pHandle->list);
    if(ret) {
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        goto failure;
    }

    //allocate memory for all mixer elements
    pHandle->pids = (snd_ctl_elem_id_t *)pids;
        //kmalloc(sizeof(snd_ctl_elem_id_t)*pHandle->list.count, GFP_KERNEL);
    if(pHandle->pids == NULL) {
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
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
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        goto failure;
    }
//    if (pids)
//        memcpy(pids, pHandle->pids,
//               sizeof(snd_ctl_elem_id_t)*pHandle->list.count);

    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
    if(pHandle) {
//        if(pHandle->pids) kfree(pHandle->pids);
        kfree(pHandle);
    }
    return pHandle->list.count;
failure:
    if(pHandle) {
//        if(pHandle->pids) kfree(pHandle->pids);
        kfree(pHandle);
    }
    DebugInt3();
    return OSSERR_OUT_OF_MEMORY;
}

int GetUniaudControlInfo(ULONG deviceid, ULONG id, void *info)
{
    snd_ctl_elem_value_t *pElem = NULL;
    snd_ctl_elem_info_t  *pElemInfo = NULL;
    mixerhandle *pHandle = NULL;
    int          ret, i, j, sz;

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

    //allocate memory for info element
    pElemInfo = (snd_ctl_elem_info_t *)info;
    if(pElemInfo == NULL) {
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        goto failure;
    }

//    printk("sizeof elem_info %i\n",sizeof(snd_ctl_elem_info_t));

    pElemInfo->id.numid = id;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_INFO, (ULONG)pElemInfo);
    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
    if(pHandle) kfree(pHandle);
//    printk("elem type: %i, id: %i, card: %i\n", pElemInfo->type, pElemInfo->id.numid, deviceid);
    return OSSERR_SUCCESS;

failure:
    if(pHandle) {
        kfree(pHandle);
    }
    DebugInt3();
    return OSSERR_OUT_OF_MEMORY;
}

int GetUniaudControlValueGet(ULONG deviceid, ULONG id, void *value)
{
    snd_ctl_elem_value_t *pElem = NULL;
    snd_ctl_elem_info_t  *pElemInfo = NULL;
    mixerhandle *pHandle = NULL;
    int          ret, i, j, sz;

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

    pElem = (snd_ctl_elem_value_t *)value;
    if(pElem == NULL) {
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        goto failure;
    }

//    printk("sizeof elem_info %i\n",sizeof(snd_ctl_elem_info_t));

    pElem->id.numid = id;
    pElem->indirect = 0;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_READ, (ULONG)pElem);
    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
    if(pHandle) kfree(pHandle);

    return OSSERR_SUCCESS;
failure:
    if(pHandle) {
        kfree(pHandle);
    }
    DebugInt3();
    return OSSERR_OUT_OF_MEMORY;
}

int GetUniaudControlValuePut(ULONG deviceid, ULONG id, void *value)
{
    snd_ctl_elem_value_t *pElem = NULL;
    snd_ctl_elem_info_t  *pElemInfo = NULL;
    mixerhandle *pHandle = NULL;
    int          ret, i, j, sz;

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

    pElem = (snd_ctl_elem_value_t *)value;
    if(pElem == NULL) {
        ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
        goto failure;
    }

//    printk("sizeof elem_info %i\n",sizeof(snd_ctl_elem_info_t));

    pElem->id.numid = id;
    pElem->indirect = 0;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_CTL_IOCTL_ELEM_WRITE, (ULONG)pElem);
    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
    if(pHandle) kfree(pHandle);

    return OSSERR_SUCCESS;

failure:
    if(pHandle) {
        kfree(pHandle);
    }
    DebugInt3();
    return OSSERR_OUT_OF_MEMORY;
}

int UniaudIoctlHWRefine(OSSSTREAMID streamid, void *pHwParams)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;
    snd_pcm_hw_params_t *params = NULL;

    params = (snd_pcm_hw_params_t *)pHwParams;

    if (!params) return -1001;

    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_REFINE, (ULONG)params);
    return ret;
}

int UniaudIoctlHWParamSet(OSSSTREAMID streamid, void *pHwParams)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;
    snd_pcm_hw_params_t *params = NULL;

    params = (snd_pcm_hw_params_t *)pHwParams;

    if (!params) return -1001;
    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;
    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_HW_PARAMS, (ULONG)params);
    return ret;
}

int UniaudIoctlSWParamSet(OSSSTREAMID streamid, void *pSwParams)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;
    snd_pcm_sw_params_t *params = NULL;

    params = (snd_pcm_sw_params_t *)pSwParams;

    if (!params) return -1001;
    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_SW_PARAMS, (ULONG)params);
    return ret;
}

int UniaudIoctlPCMStatus(OSSSTREAMID streamid, void *pstatus)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;
    snd_pcm_status_t   *status = (snd_pcm_status_t *)pstatus;

    if (!status) return -1001;
    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_STATUS, (ULONG)status);
    return ret;
}

int UniaudIoctlPCMWrite(OSSSTREAMID streamid, char *buf, int size)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;

    if (!buf) return -1001;
    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->write(&pHandle->file, buf, size, &pHandle->file.f_pos);

    return ret;
}

int UniaudIoctlPCMRead(OSSSTREAMID streamid, char *buf, int size)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;

    if (!buf) return -1001;
    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->read(&pHandle->file, buf, size, &pHandle->file.f_pos);

    return ret;
}

int UniaudIoctlPCMPrepare(OSSSTREAMID streamid)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;

    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PREPARE, 0);
    return ret;
}

int UniaudIoctlPCMResume(OSSSTREAMID streamid, int pause)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;

    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_PAUSE, pause);

    return ret;
}

int UniaudIoctlPCMStart(OSSSTREAMID streamid)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;

    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_START, 0);

    return ret;
}

int UniaudIoctlPCMDrop(OSSSTREAMID streamid)
{
    int ret;
    soundhandle        *pHandle = (soundhandle *)streamid;

    if (!pHandle) return -1002;

    pHandle->file.f_flags = O_NONBLOCK;

    ret = pHandle->file.f_op->ioctl(&pHandle->inode, &pHandle->file, SNDRV_PCM_IOCTL_DROP, 0);

    return ret;
}

void UniaudCloseAll(USHORT fileid)
{
    int i;

    for (i=0; i < 8*256; i++)
    {
        if (opened_handles[i].handle != 0)
        {
            if (fileid)
            {
                if (fileid == opened_handles[i].FileId)
                {
                    opened_handles[i].reuse = 0;
                    if (OSS32_WaveClose((OSSSTREAMID)opened_handles[i].handle) == 0)
                        opened_handles[i].handle = 0;
                }
            }
            else
            {
                opened_handles[i].reuse = 0;
                if (OSS32_WaveClose((OSSSTREAMID)opened_handles[i].handle) == 0)
                    opened_handles[i].handle = 0;
            }
        }
    }
    return;
}
