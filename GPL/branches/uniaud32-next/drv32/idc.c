/* $Id: idc.c,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * IDC entry point for calls from 16bit driver
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

//DAZ extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_DOSINFOSEG
#include <os2.h>
//DAZ }
#include <devhelp.h>
#include <ossidc32.h>
#include <irqos2.h>
#include <dbgos2.h>

//16:32 address of 16 bits pdd idc handler
IDC16_HANDLER idc16_PddHandler = 0;
extern int pcm_device;
OSSRET OSS32IDC(ULONG cmd, PIDC32_PACKET pPacket);

//packet pointer must reference a structure on the stack

#pragma aux AlsaIDC "ALSA_IDC" parm reverse [ecx edx]
OSSRET AlsaIDC(ULONG cmd, ULONG packet)
{
    PIDC32_PACKET pPacket = (PIDC32_PACKET)__Stack16ToFlat(packet);
    ULONG  oldfileid;
    OSSRET rc;

    //Sets file id in current task structure
    oldfileid = OSS32_SetFileId(pPacket->fileid);
    rc = OSS32IDC(cmd & 0xFFFF, pPacket);
    OSS32_SetFileId(oldfileid);
    return rc;
}

OSSRET OSS32IDC(ULONG cmd, PIDC32_PACKET pPacket)
{
  OSSRET rc;

  switch(cmd)
  {
  case IDC32_OSS32_INIT:
       idc16_PddHandler = (IDC16_HANDLER)MAKE_FARPTR32(pPacket->init.handler16);
       return OSSERR_SUCCESS;

  case IDC32_OSS32_EXIT:
       idc16_PddHandler = 0;
       return OSSERR_SUCCESS;

  case IDC32_OSS32_QUERYDEVCAPS:
       return OSS32_QueryDevCaps(pPacket->devcaps.devicenr, (POSS32_DEVCAPS)__Stack16ToFlat(pPacket->devcaps.pDevCaps));

  case IDC32_OSS32_APM_SUSPEND:
       return OSS32_APMSuspend();

  case IDC32_OSS32_APM_RESUME:
       return OSS32_APMResume();

  case IDC32_WAVE_OPEN:
  {
       OSSSTREAMID streamId = 0;

       rc = OSS32_WaveOpen(pPacket->open.devicenr, pPacket->open.streamtype, &streamId, pcm_device, (USHORT)pPacket->fileid); /* DAZ cast added to stop compiler warning */
       pPacket->streamid = streamId;
       return rc;
  }

  case IDC32_WAVE_CLOSE:
       return OSS32_WaveClose(pPacket->streamid);

  case IDC32_WAVE_ADDBUFFER:
  {
       ULONG transferred = 0;
       rc = OSS32_WaveAddBuffer(pPacket->streamid, pPacket->buffer.buffer, pPacket->buffer.size, &transferred, pcm_device);
       pPacket->buffer.transferred = transferred;
       return rc;
  }

  case IDC32_WAVE_GETSPACE:
  {
       ULONG bytesavail = 0;
       rc = OSS32_WaveGetSpace(pPacket->streamid, &bytesavail);
       pPacket->getspace.bytesavail = bytesavail;
       return rc;
  }

  case IDC32_WAVE_PREPARE:
       return OSS32_WavePrepare(pPacket->streamid);

  case IDC32_WAVE_RESUME:
       return OSS32_WaveResume(pPacket->streamid);

  case IDC32_WAVE_PAUSE:
       return OSS32_WavePause(pPacket->streamid);

  case IDC32_WAVE_START:
       return OSS32_WaveStart(pPacket->streamid);

  case IDC32_WAVE_STOP:
		DbgPrintIrq();
       return OSS32_WaveStop(pPacket->streamid);

  case IDC32_WAVE_SETHWPARAMS:
  {
       OSS32_HWPARAMS *pHwParam = (OSS32_HWPARAMS *)__Stack16ToFlat(pPacket->sethwparams.hwparams);
       return OSS32_WaveSetHwParams(pPacket->streamid, pHwParam);
  }

  case IDC32_WAVE_GETPOS:
  {
       ULONG position = 0;
       rc = OSS32_WaveGetPosition(pPacket->streamid, &position);
       pPacket->getpos.position = position;
       return rc;
  }

  case IDC32_WAVE_GETHWPTR:
  {
       ULONG position = 0;
       rc = OSS32_WaveGetHwPtr(pPacket->streamid, &position);
       pPacket->getpos.position = position;
       return rc;
  }

  case IDC32_WAVE_GETSTATUS:
  {
       ULONG status = 0;
       rc = OSS32_WaveGetStatus(pPacket->streamid, &status);
       pPacket->status.state = status;
       return rc;
  }

  case IDC32_WAVE_SETVOLUME:
       return OSS32_WaveSetVolume(pPacket->streamid, pPacket->setwavevol.volume);

  case IDC32_MIXER_OPEN:
  {
       OSSSTREAMID streamId = 0;

       rc = OSS32_MixOpen(pPacket->open.devicenr, &streamId);
       pPacket->streamid = streamId;
       return rc;
  }

  case IDC32_MIXER_CLOSE:
       return OSS32_MixClose(pPacket->streamid);

  case IDC32_MIXER_GETVOLUME:
  {
       ULONG volume = 0;

       rc = OSS32_MixGetVolume(pPacket->streamid, pPacket->mixvol.line, &volume);
       pPacket->mixvol.volume = volume;
       return rc;
  }

  case IDC32_MIXER_SETVOLUME:
       return OSS32_MixSetVolume(pPacket->streamid, pPacket->mixvol.line, pPacket->mixvol.volume);

  case IDC32_MIXER_GETPROPERTY:
  {
       ULONG value = 0;

       rc = OSS32_MixGetProperty(pPacket->streamid, pPacket->mixprop.property, &value);
       pPacket->mixprop.value = value;
       return rc;
  }

  case IDC32_MIXER_SETPROPERTY:
       return OSS32_MixSetProperty(pPacket->streamid, pPacket->mixprop.property, pPacket->mixprop.value);

  case IDC32_MIXER_QUERYCAPS:
       return OSS32_MixQueryCaps(pPacket->streamid, (POSS32_MIXCAPS)__Stack16ToFlat(pPacket->mixcaps.pCaps));

  case IDC32_MIDI_OPEN:
  {
       OSSSTREAMID streamId = 0;

       rc = OSS32_MidiOpen(pPacket->open.devicenr, pPacket->open.streamtype, &streamId);
       pPacket->streamid = streamId;
       return rc;
  }

  case IDC32_MIDI_CLOSE:
       return OSS32_MidiClose(pPacket->streamid);

  case IDC32_MIDI_WRITE:
       return OSS32_MidiWrite(pPacket->streamid, pPacket->midiwrite.midiByte);

  case IDC32_MIDI_READ:
      {
       ULONG transferred = 0;
       rc = OSS32_MidiRead(pPacket->streamid, (char *)__Stack16ToFlat(pPacket->midiread.buffer), pPacket->midiread.bufsize, &transferred);
       pPacket->buffer.transferred = transferred;
       return rc;
      }

  case IDC32_MIDI_QUERYCAPS:
       return OSS32_MidiQueryCaps(pPacket->streamid, (POSS32_MIDICAPS)__Stack16ToFlat(pPacket->midicaps.pCaps));

  case IDC32_DEBUG_STRING:
       return OSS32_DebugString((char *)pPacket->buffer.buffer, pPacket->buffer.size);

  case IDC32_MIDI_NOTEON:
  case IDC32_MIDI_NOTEOFF:
  case IDC32_MIDI_KEY_PRESSURE:
  case IDC32_MIDI_CONTROL_CHANGE:
  case IDC32_MIDI_PROGRAM_CHANGE:
  case IDC32_MIDI_CHANNEL_PRESSURE:
  case IDC32_MIDI_PITCH_BEND:
       return OSS32_MidiCommand(pPacket->streamid, cmd, pPacket->midicmd.channel, pPacket->midicmd.param1, pPacket->midicmd.param2);
  }

  return OSSERR_INVALID_PARAMETER;
}
