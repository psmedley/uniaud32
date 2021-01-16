/* $Id: soundmidi.c,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */
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
#include <sound/asequencer.h>
#include <sound/seqmid.h>
#include <sound/seq_kernel.h>
#include <linux/file.h>
#include <linux/soundcard.h>

#define LINUX
#include <ossidc32.h>
#include <stacktoflat.h>
#include <stdlib.h>
#include "soundoss.h"

int midi_port = 0;

//******************************************************************************
//******************************************************************************
OSSRET OSS32_MidiOpen(ULONG deviceid, ULONG streamtype, OSSSTREAMID *pStreamId)
{
    midihandle                      *pHandle;
    int                              ret, i;
    struct snd_seq_port_subscribe	subs;
    struct snd_seq_client_info     clientinfo;
    struct snd_seq_port_info       portinfo;

    *pStreamId = 0;

    if(alsa_fops == NULL) {
        DebugInt3();
        return OSSERR_NO_DEVICE_AVAILABLE;
    }

    pHandle = kmalloc(sizeof(midihandle), GFP_KERNEL);
    if(pHandle == NULL) {
        DebugInt3();
        return OSSERR_OUT_OF_MEMORY;
    }
    memset(pHandle, 0, sizeof(midihandle));

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    //setup pointers in file structure (used internally by ALSA)
    pHandle->file.f_dentry          = &pHandle->d_entry;
    pHandle->file.f_dentry->d_inode = &pHandle->inode;

    switch(streamtype) {
    case OSS32_STREAM_MPU401_MIDIOUT:
        dprintf(("OSS32_STREAM_MPU401_MIDIOUT"));
        pHandle->file.f_mode  = FMODE_WRITE;
        pHandle->inode.i_rdev = SNDRV_MINOR(deviceid, SNDRV_MINOR_RAWMIDI);
        pHandle->inode.i_rdev |= (CONFIG_SND_MAJOR << 8);
        break;
    case OSS32_STREAM_MPU401_MIDIIN:
        dprintf(("OSS32_STREAM_MPU401_MIDIIN"));
        pHandle->file.f_mode  = FMODE_READ;
        pHandle->inode.i_rdev = SNDRV_MINOR(deviceid, SNDRV_MINOR_RAWMIDI);
        pHandle->inode.i_rdev |= (CONFIG_SND_MAJOR << 8);
        break;
    case OSS32_STREAM_FM_MIDIOUT:
        dprintf(("OSS32_STREAM_FM_MIDIOUT"));
        pHandle->file.f_mode  = FMODE_WRITE;
        pHandle->inode.i_rdev = SNDRV_MINOR(deviceid, SNDRV_MINOR_SEQUENCER);
        break;
    case OSS32_STREAM_WAVETABLE_MIDIOUT:
        dprintf(("OSS32_STREAM_WAVETABLE_MIDIOUT: TODO"));
        pHandle->file.f_mode  = FMODE_WRITE;
        pHandle->inode.i_rdev = SNDRV_MINOR(deviceid, SNDRV_MINOR_SEQUENCER);
        kfree(pHandle);
        return OSSERR_INVALID_PARAMETER;
        break;
    default:
        DebugInt3();
        kfree(pHandle);
        return OSSERR_INVALID_PARAMETER;
    }
#if 1

    ret = alsa_fops->open(&pHandle->inode, &pHandle->file);
    if(ret) {
        rprintf(("Midi Open %d failed with error %d", streamtype, ret));
        kfree(pHandle);
        return UNIXToOSSError(ret);
    }
    pHandle->magic = MAGIC_MIDI_ALSA32;
    *pStreamId = (ULONG)pHandle;

    pHandle->state |= MIDISTATE_OPENED;
    dprintf(("Opened MIDI %x\n",(ULONG)pHandle));
    if(streamtype == OSS32_STREAM_FM_MIDIOUT)
    {
        //get the client id
        ret = pHandle->file.f_op->unlocked_ioctl(&pHandle->file, SNDRV_SEQ_IOCTL_CLIENT_ID, (ULONG)&pHandle->clientid);
        if(ret) {
            rprintf(("Get client id failed with error %d", ret));
            kfree(pHandle);
            return UNIXToOSSError(ret);
        }

        //find the FM device
        for(i=64;i<64+((deviceid+1)<<3);i++)
        {
            memset((PVOID)__Stack32ToFlat(&clientinfo), 0, sizeof(clientinfo));
            clientinfo.client = i;
            clientinfo.type   = KERNEL_CLIENT;
            ret = pHandle->file.f_op->unlocked_ioctl(&pHandle->file, SNDRV_SEQ_IOCTL_GET_CLIENT_INFO, (ULONG)__Stack32ToFlat(&clientinfo));
            if(ret) {
                continue;
            }
            if(!strcmp(clientinfo.name, "OPL3 FM synth")) {
                //found the FM synth device
                break;
            }
        }
        if(i == 64+((deviceid+1)<<3)) {
            rprintf(("Couldn't find OPL3 device"));
            ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
            kfree(pHandle);
            *pStreamId = 0;
            return OSSERR_NO_DEVICE_AVAILABLE;
        }
        pHandle->destclient = i;
        pHandle->destport   = 0;

        //create client port
        memset(&portinfo, 0, sizeof(portinfo));
        strcpy(portinfo.name, "Unamed port");
        portinfo.addr.client = pHandle->clientid;
        portinfo.flags       = SNDRV_SEQ_PORT_TYPE_APPLICATION;
        ret = pHandle->file.f_op->unlocked_ioctl(&pHandle->file, SNDRV_SEQ_IOCTL_CREATE_PORT, (ULONG)__Stack32ToFlat(&portinfo));
        if(ret) {
            rprintf(("subscribe error %d", ret));
            kfree(pHandle);
            return UNIXToOSSError(ret);
        }
        pHandle->clientport = portinfo.addr.port;
        pHandle->state     |= MIDISTATE_PORTCREATED;

        //subscribe to FM device port
        memset(&subs, 0, sizeof(subs));
        subs.dest.client   = pHandle->destclient;
        subs.dest.port     = pHandle->destport;
        subs.sender.client = pHandle->clientid;
        subs.sender.port   = pHandle->clientport;
        ret = pHandle->file.f_op->unlocked_ioctl(&pHandle->file, SNDRV_SEQ_IOCTL_SUBSCRIBE_PORT, (ULONG)__Stack32ToFlat(&subs));
        if(ret) {
            rprintf(("subscribe error %d", ret));
            kfree(pHandle);
            return UNIXToOSSError(ret);
        }
        pHandle->state |= MIDISTATE_SUBSCRIBED;

#if 0
        //Load FM instruments (only done once)
        OSS32_FMMidiLoadInstruments((ULONG)pHandle);
#endif

    }//if FM
#endif
    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MidiClose(OSSSTREAMID streamid)
{
    midihandle *pHandle = (midihandle *)streamid;
    int                              ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_MIDI_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    dprintf(("Closing MIDI %x\n",(ULONG)streamid));

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;
    //unsubscribe
    if(pHandle->state & MIDISTATE_SUBSCRIBED) {
        struct snd_seq_port_subscribe  subs;
        memset(&subs, 0, sizeof(subs));
        subs.dest.client   = pHandle->destclient;
        subs.dest.port     = pHandle->destport;
        subs.sender.client = pHandle->clientid;
        subs.sender.port   = pHandle->clientport;
        ret = pHandle->file.f_op->unlocked_ioctl(&pHandle->file, SNDRV_SEQ_IOCTL_UNSUBSCRIBE_PORT, (ULONG)__Stack32ToFlat(&subs));
        if(ret) {
            rprintf(("unsubscribe error %d", ret));
            DebugInt3();
        }
    }

    //delete port
    if(pHandle->state & MIDISTATE_SUBSCRIBED) {
        struct snd_seq_port_info       portinfo;

        memset(&portinfo, 0, sizeof(portinfo));
        strcpy(portinfo.name, "Unamed port");
        portinfo.addr.client = pHandle->clientid;
        portinfo.addr.port   = pHandle->clientport;
        ret = pHandle->file.f_op->unlocked_ioctl(&pHandle->file, SNDRV_SEQ_IOCTL_DELETE_PORT, (ULONG)__Stack32ToFlat(&portinfo));
        if(ret) {
            dprintf(("delete port error %d", ret));
            DebugInt3();
        }
    }
    //and release device
    ret = pHandle->file.f_op->release(&pHandle->inode, &pHandle->file);
    kfree(pHandle);   //free handle data

    if(ret) {
        DebugInt3();
        return UNIXToOSSError(ret);
    }

    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MidiWrite(OSSSTREAMID streamid, ULONG midiByte)
{
    dprintf(("MIDI write %x\n",(ULONG)streamid));
    return OSSERR_INVALID_STREAMID;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MidiRead(OSSSTREAMID streamid, char *buffer, ULONG bufsize, ULONG *pTransferred)
{
    midihandle *pHandle = (midihandle *)streamid;
    int                              ret;

    if(pHandle == NULL || pHandle->magic != MAGIC_MIDI_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;
    //and release device
    ret = pHandle->file.f_op->read(&pHandle->file,(char*)buffer, bufsize, &pHandle->file.f_pos);
    kfree(pHandle);   //free handle data
    if (ret > 0)
    {
        dprintf(("MIDI read %i bytes from %x. first byte is: %x\n",ret, (ULONG)streamid, (char*)buffer[0]));
        *pTransferred = ret;
        return OSSERR_SUCCESS;
    }
    return UNIXToOSSError(ret);
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MidiQueryCaps(OSSSTREAMID streamid, POSS32_MIDICAPS pCaps)
{
    dprintf(("MIDI query caps %x\n",(ULONG)streamid));
    return OSSERR_INVALID_STREAMID;
}
//******************************************************************************
//******************************************************************************
OSSRET OSS32_MidiCommand(OSSSTREAMID streamid, ULONG Cmd, BYTE channel, BYTE param1, BYTE param2)
{
    midihandle  *pHandle = (midihandle *)streamid;
    //int          ret;
    LONG         transferred;
    struct snd_seq_event fmevent;

    if(pHandle == NULL || pHandle->magic != MAGIC_MIDI_ALSA32) {
        DebugInt3();
        return OSSERR_INVALID_STREAMID;
    }
    dprintf(("MIDI command %x\n",(ULONG)streamid));

    //set operation to non-blocking
    pHandle->file.f_flags = O_NONBLOCK;

    memset((PVOID)__Stack32ToFlat(&fmevent), 0, sizeof(fmevent));
    switch(Cmd) {
    case IDC32_MIDI_NOTEON:
        snd_seq_ev_set_noteon((&fmevent), channel, param1, param2);
        break;
    case IDC32_MIDI_NOTEOFF:
        snd_seq_ev_set_noteoff((&fmevent), channel, param1, param2);
        break;
    case IDC32_MIDI_KEY_PRESSURE:
        snd_seq_ev_set_keypress((&fmevent), channel, param1, param2);
        break;
    case IDC32_MIDI_CONTROL_CHANGE:
        snd_seq_ev_set_controller((&fmevent), channel, param1, param2);
        break;
    case IDC32_MIDI_PROGRAM_CHANGE:
        snd_seq_ev_set_pgmchange((&fmevent), channel, param1);
        break;
    case IDC32_MIDI_CHANNEL_PRESSURE:
        snd_seq_ev_set_chanpress((&fmevent), channel, param1);
        break;
    case IDC32_MIDI_PITCH_BEND:
        snd_seq_ev_set_pitchbend((&fmevent), channel, param1);
        break;
    }
    fmevent.tag           = 0;
    fmevent.queue         = SNDRV_SEQ_QUEUE_DIRECT;
    fmevent.source.client = pHandle->clientid;
    fmevent.source.port   = pHandle->clientport;
    fmevent.dest.client   = pHandle->destclient;
    fmevent.dest.port     = pHandle->destport;

    transferred = pHandle->file.f_op->write(&pHandle->file, (char *)__Stack32ToFlat(&fmevent), sizeof(fmevent), &pHandle->file.f_pos);

    if(transferred < 0) {
        rprintf(("OSS32_MidiNoteOn failed!!"));
        DebugInt3();
        return UNIXToOSSError(transferred);
    }
    return OSSERR_SUCCESS;
}
//******************************************************************************
//******************************************************************************

