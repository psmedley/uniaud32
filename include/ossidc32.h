/* $Id: ossidc32.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * Header for IDC definitions & declarations
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

#ifndef __OSSIDC32_H__
#define __OSSIDC32_H__

#include <ossidc.h>

#ifdef __cplusplus
extern "C" {
#endif

int MyDevBlock(ULONG id, ULONG tout, char flag);

//32 bits IDC procedures
//16 bits pdd calls this during init. OSS_InitDriver calls init_module
OSSRET OSS32_Initialize();
OSSRET OSS32_Shutdown();

int OSS32_CloseUNI16(void);	// 12 Jun 07 SHL

OSSRET OSS32_QueryDevCaps(ULONG deviceid, POSS32_DEVCAPS pDevCaps);
OSSRET OSS32_QueryNames(ULONG deviceid, char *pszDeviceName, ULONG cbDeviceName,
                        char *pszMixerName, ULONG cbMixerName, BOOL fLongName);
OSSRET OSS32_APMResume();
OSSRET OSS32_APMSuspend();

OSSRET OSS32_WaveOpen(ULONG deviceid, ULONG streamtype, OSSSTREAMID *pStreamId, int pcm, USHORT fileid);
OSSRET OSS32_WaveClose(OSSSTREAMID streamid);
OSSRET OSS32_WaveStart(OSSSTREAMID streamid);
OSSRET OSS32_WaveStop(OSSSTREAMID streamid);
OSSRET OSS32_WavePause(OSSSTREAMID streamid);
OSSRET OSS32_WaveResume(OSSSTREAMID streamid);
OSSRET OSS32_WavePrepare(OSSSTREAMID streamid);
OSSRET OSS32_WaveSetHwParams(OSSSTREAMID streamid, OSS32_HWPARAMS *pFormatInfo);
OSSRET OSS32_WaveAddBuffer(OSSSTREAMID streamid, ULONG buffer, ULONG size, ULONG *pTransferred, int pcm);
OSSRET OSS32_WaveGetPosition(OSSSTREAMID streamid, ULONG *pPosition);
OSSRET OSS32_WaveGetSpace(OSSSTREAMID streamid, ULONG *pBytesAvail);
OSSRET OSS32_WaveSetVolume(OSSSTREAMID streamid, ULONG volume);

OSSRET OSS32_MixOpen(ULONG deviceid, OSSSTREAMID *pStreamId);
OSSRET OSS32_MixClose(OSSSTREAMID streamid);
OSSRET OSS32_MixSetVolume(OSSSTREAMID streamid, ULONG line, ULONG volume);
OSSRET OSS32_MixGetVolume(OSSSTREAMID streamid, ULONG line, ULONG *pVolume);
OSSRET OSS32_MixSetProperty(OSSSTREAMID streamid, ULONG line, ULONG value);
OSSRET OSS32_MixGetProperty(OSSSTREAMID streamid, ULONG line, ULONG *pValue);
OSSRET OSS32_MixQueryCaps(OSSSTREAMID streamid, POSS32_MIXCAPS pCaps);
OSSRET OSS32_MixQueryName(ULONG deviceid, char *pszMixerName, ULONG cbMixerName);

OSSRET OSS32_MidiOpen(ULONG deviceid, ULONG streamtype, OSSSTREAMID *pStreamId);
OSSRET OSS32_MidiClose(OSSSTREAMID streamid);
OSSRET OSS32_MidiWrite(OSSSTREAMID streamid, ULONG midiByte);
OSSRET OSS32_MidiRead(OSSSTREAMID streamid, char *buffer, ULONG bufsize, ULONG *pTransferred);
OSSRET OSS32_MidiQueryCaps(OSSSTREAMID streamid, POSS32_MIDICAPS pCaps);
OSSRET OSS32_MidiCommand(OSSSTREAMID streamid, ULONG Cmd, BYTE channel, BYTE param1, BYTE param2);

int OSS32_ProcessMIDIIRQ(int bytes);	// 12 Jun 07 SHL

//Sets file id in current task structure
ULONG OSS32_SetFileId(ULONG fileid);

//Notifies the 16 bits MMPM/2 driver that an irq has occured. This means
//it has to query the status of all active streams.
int OSS32_ProcessIRQ();
extern IDC16_HANDLER idc16_PddHandler;

ULONG _cdecl CallPDD16(IDC16_HANDLER pHandler, ULONG cmd, ULONG param1, ULONG param2);

#ifdef __cplusplus
}
#endif

#endif
