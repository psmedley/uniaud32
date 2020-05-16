/* $Id: ossidc.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
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

#ifndef __OSSIDC_H__
#define __OSSIDC_H__

#include <ossdefos2.h>
#include <osssound.h>
#include <osserror.h>

#pragma pack(4)

typedef BOOL (FAR48 __cdecl *IDC16_HANDLER)(ULONG cmd, ULONG param);

//OSS types
typedef ULONG OSSSTREAMID;	//stream id
typedef ULONG OSSRET;		//OSS error code

//IDC communication packet
typedef struct
{
	OSSSTREAMID streamid;	//IN:  stream id
	ULONG       fileid;
	union {
		struct {
			ULONG	handler16;	//IN: 16:16 address of pdd idc handler
		} init;
		struct {
			ULONG   devicenr;       //IN:  nr of detected device
			ULONG   streamtype;     //IN:  stream type
		} open;
		struct {
			ULONG   devicenr;       //IN:  nr of detected device
			ULONG   pDevCaps;       //IN:  16:16 stack based pointer to OSS32_DEVCAPS structure
		} devcaps;
		struct {
			ULONG   position; 	//OUT: stream position
		} getpos;
		struct {
			ULONG   state;	//OUT: stream state
		} status;
		struct {
			ULONG   hwparams;       //IN:  16:16 pointer to OSS32_HWPARAMS structure
		} sethwparams;
		struct {
			ULONG   bytesavail;     //OUT: bytes available in dma buffer
		} getspace;
		struct {
			ULONG   volume;         //IN:  wave stream volume
		} setwavevol;
		struct {
			ULONG 	buffer;		//IN:  physical address of mmpm/2 buffer
			ULONG 	size;		//IN:  size of mmpm/2 buffer
			ULONG   transferred;	//OUT: nr of bytes transferred
		} buffer;
		struct {
			ULONG   midiByte;
		} midiwrite;
		struct {
			ULONG   buffer;         //IN: data buffer
			ULONG   bufsize;        //IN: size of data buffer
			ULONG   transferred;	//OUT: nr of bytes transferred
		} midiread;
		struct {
			ULONG   pCaps;          //IN: 16:16 stack based pointer to OSSMIDICAPS structure
		} midicaps;
		struct {
			BYTE    channel;        //IN: channel nr
			BYTE    param1;         //IN: midi command parameter 1
			BYTE    param2;         //IN: midi command parameter 2
		} midicmd;
		struct {
			ULONG   line;           //IN: mixer line
			ULONG   volume;         //IN/OUT: volume (set/get)
		} mixvol;
		struct {
			ULONG   property;       //IN: mixer property
			ULONG   value;          //IN/OUT: property value (set/get)
		} mixprop;
		struct {
			ULONG   pCaps;          //IN: 16:16 stack based pointer to OSSMIXCAPS structure
		} mixcaps;
		struct {
			ULONG 	param1;
			ULONG 	param2;
			ULONG 	param3;
		};
	};
} IDC32_PACKET, NEAR *PIDC32_PACKET;

//IOCTL commands
#define CAT_IOCTL_OSS32			0x92
#define IOCTL_OSS32_ATTACH		0x40
#define IOCTL_OSS32_VERSION		0x41
#define IOCTL_OSS32_GET_PCM_NUM         0x42
#define IOCTL_OSS32_PCM_CAPS            0x43
#define IOCTL_OSS32_SET_PCM             0x44
#define IOCTL_OSS32_GET_PCM             0x45
#define IOCTL_OSS32_CARDS_NUM           0x48
#define IOCTL_OSS32_CARD_INFO           0x49
#define IOCTL_OSS32_GET_POWER_STATE     0x4A
#define IOCTL_OSS32_SET_POWER_STATE     0x4B
#define IOCTL_OSS32_GET_CNTRLS_NUM      0x50
#define IOCTL_OSS32_GET_CNTRLS          0x51
#define IOCTL_OSS32_CNTRL_INFO          0x52
#define IOCTL_OSS32_CNTRL_GET           0x53
#define IOCTL_OSS32_CNTRL_PUT           0x54
#define IOCTL_OSS32_CNTRL_WAIT          0x55

#define IOCTL_OSS32_PCM_OPEN            0x60
#define IOCTL_OSS32_PCM_CLOSE           0x61
#define IOCTL_OSS32_PCM_READ            0x62
#define IOCTL_OSS32_PCM_WRITE           0x63
#define IOCTL_OSS32_PCM_PAUSE           0x64
#define IOCTL_OSS32_PCM_RESUME          0x65
#define IOCTL_OSS32_PCM_DROP            0x66
#define IOCTL_OSS32_PCM_DRAIN           0x67
#define IOCTL_OSS32_PCM_STATUS          0x68
#define IOCTL_OSS32_PCM_REFINEHWPARAMS  0x69
#define IOCTL_OSS32_PCM_SETHWPARAMS     0x70
#define IOCTL_OSS32_PCM_SETSWPARAMS     0x71
#define IOCTL_OSS32_PCM_PREPARE         0x72
#define IOCTL_OSS32_PCM_WAIT_INT        0x73
#define IOCTL_OSS32_PCM_START           0x74

#define IOCTL_OSS32_PCM_CLOSE_ALL       0x1F
#define IOCTL_OSS32_PCM_CLOSE_16        0x1F+1
//IDC commands (16->32 bits)
#define IDC32_OSS32_INIT		0x00
#define IDC32_OSS32_EXIT		0x01
#define IDC32_OSS32_QUERYDEVCAPS	0x02
#define IDC32_OSS32_APM_SUSPEND         0x03
#define IDC32_OSS32_APM_RESUME          0x04

#define IDC32_WAVE_OPEN               	0x10
#define IDC32_WAVE_CLOSE              	0x11
#define IDC32_WAVE_ADDBUFFER		0x12
#define IDC32_WAVE_PREPARE		0x13
#define IDC32_WAVE_START		0x14
#define IDC32_WAVE_STOP			0x15
#define IDC32_WAVE_RESET		0x16
#define IDC32_WAVE_RESUME              	0x17
#define IDC32_WAVE_PAUSE		0x18
#define IDC32_WAVE_GETSPACE		0x19
#define IDC32_WAVE_SETHWPARAMS        	0x1A
#define IDC32_WAVE_GETPOS             	0x1B
#define IDC32_WAVE_SETVOLUME           	0x1C
#define IDC32_WAVE_GETHWPTR             0x1D
#define IDC32_WAVE_GETSTATUS             0x1E

#define IDC32_MIXER_OPEN                0x20
#define IDC32_MIXER_SETVOLUME		0x21
#define IDC32_MIXER_GETVOLUME		0x22
#define IDC32_MIXER_SETPROPERTY		0x23
#define IDC32_MIXER_GETPROPERTY		0x24
#define IDC32_MIXER_QUERYCAPS		0x25
#define IDC32_MIXER_CLOSE               0x26

#define IDC32_MIDI_OPEN                 0x30
#define IDC32_MIDI_WRITE                0x31
#define IDC32_MIDI_READ                 0x32
#define IDC32_MIDI_QUERYCAPS            0x33
#define IDC32_MIDI_NOTEON               0x34
#define IDC32_MIDI_NOTEOFF              0x35
#define IDC32_MIDI_KEY_PRESSURE         0x36
#define IDC32_MIDI_CONTROL_CHANGE       0x37
#define IDC32_MIDI_PROGRAM_CHANGE       0x38
#define IDC32_MIDI_CHANNEL_PRESSURE     0x39
#define IDC32_MIDI_PITCH_BEND           0x3A
#define IDC32_MIDI_CLOSE                0x3B
#define IDC32_DEBUG_STRING              0x3C

#define IDC16_INIT			0
#define IDC16_EXIT			1
#define IDC16_WAVEOUT_BUFFER_DONE	2
#define IDC16_WAVEIN_BUFFER_DONE	3
#define IDC16_PROCESS                   4

#define IDC16_WAVEIN_IRQ		0
#define IDC16_WAVEOUT_IRQ		1
#define IDC16_MIDI_IRQ			2
#define IDC16_CLOSE_UNI16               3

// Default device nr (first detected audio device)
#define OSS32_DEFAULT_DEVICE            0
#define OSS32_DEFAULT_WAVE_DEVICE       OSS32_DEFAULT_DEVICE
#define OSS32_DEFAULT_MIXER_DEVICE      OSS32_DEFAULT_DEVICE
#define OSS32_DEFAULT_FMSYNTH_DEVICE    OSS32_DEFAULT_DEVICE
#define OSS32_DEFAULT_WAVETABLE_DEVICE  OSS32_DEFAULT_DEVICE
#define OSS32_DEFAULT_MPU401_DEVICE     OSS32_DEFAULT_DEVICE
#define OSS32_MAX_AUDIOCARDS		8


// PCM data types: (DO NOT CHANGE without changing conversion table in sound.c!)
#define	OSS32_PCM_FORMAT_S8		0	//signed 8 bits sample
#define	OSS32_PCM_FORMAT_U8		1	//unsigned 8 bits sample
#define	OSS32_PCM_FORMAT_S16_LE		2	//signed 16 bits sample (little endian/Intel)
#define	OSS32_PCM_FORMAT_S16_BE		3	//signed 16 bits sample (big endian/Motorola)
#define	OSS32_PCM_FORMAT_U16_LE		4	//unsigned 16 bits sample (little endian/Intel)
#define	OSS32_PCM_FORMAT_U16_BE		5	//unsigned 16 bits sample (big endian/Motorola)
#define OSS32_PCM_FORMAT_S24_LE		6       //signed 24 bits sample (little endian/Intel)
#define OSS32_PCM_FORMAT_S24_BE		7       //signed 24 bits sample (big endian/Motorola)
#define OSS32_PCM_FORMAT_U24_LE		8       //unsigned 24 bits sample (little endian/Intel)
#define OSS32_PCM_FORMAT_U24_BE		9       //unsigned 24 bits sample (big endian/Motorola)
#define OSS32_PCM_FORMAT_S32_LE		10      //signed 32 bits sample (little endian/Intel)
#define OSS32_PCM_FORMAT_S32_BE		11      //signed 32 bits sample (big endian/Motorola)
#define OSS32_PCM_FORMAT_U32_LE		12      //unsigned 32 bits sample (little endian/Intel)
#define OSS32_PCM_FORMAT_U32_BE		13      //unsigned 32 bits sample (big endian/Motorola)
#define	OSS32_PCM_FORMAT_MULAW          14	//8 bps (compressed 16 bits sample)
#define	OSS32_PCM_FORMAT_ALAW           15	//8 bps (compressed 16 bits sample)
#define	OSS32_PCM_FORMAT_ADPCM          16      //4 bps (compressed 16 bits sample)
#define OSS32_PCM_FORMAT_MPEG           17      //AC3?
#define OSS32_PCM_MAX_FORMATS           18

#define OSS32_MIX_VOLUME_MASTER_FRONT	0	//front DAC volume
#define OSS32_MIX_VOLUME_MASTER_REAR	1	//rear DAC volume
#define OSS32_MIX_VOLUME_PCM		2	//PCM volume (affects all wave streams)
#define OSS32_MIX_VOLUME_MIDI		3	//MIDI volume
#define OSS32_MIX_VOLUME_LINEIN		4	//Line-In volume
#define OSS32_MIX_VOLUME_MIC		5	//Microphone volume
#define OSS32_MIX_VOLUME_CD		6	//CD volume
#define OSS32_MIX_VOLUME_SPDIF		7	//SPDIF volume
#define OSS32_MIX_VOLUME_VIDEO		8	//Video volume
#define OSS32_MIX_VOLUME_PCSPEAKER	9	//PC speaker volume
#define OSS32_MIX_VOLUME_PHONE		10	//Phone volume
#define OSS32_MIX_VOLUME_HEADPHONE	11	//Headphone volume
#define OSS32_MIX_VOLUME_AUX           	12	//Auxiliary volume
#define OSS32_MIX_VOLUME_CAPTURE	13	//Recording volume
#define OSS32_MIX_VOLUME_SPEAKER	14	//Speaker volume
//DAZ not used #define OSS32_MIX_VOLUME_MAX            OSS32_MIX_VOLUME_SPEAKER

#define OSS32_MIX_FIRST_PROPERTY        20
#define OSS32_MIX_INPUTSRC		20	//Recording source
#define OSS32_MIX_LEVEL_BASS           	21	//Bass level
#define OSS32_MIX_LEVEL_TREBLE         	22	//Treble level
#define OSS32_MIX_LEVEL_3DCENTER       	23	//3D center level
#define OSS32_MIX_LEVEL_3DDEPTH       	24	//3D depth level
#define OSS32_MIX_SWITCH_MICBOOST       25      //Microphone boost switch
#define OSS32_MIX_MAX_PROPERTY          OSS32_MIX_SWITCH_MICBOOST
#define OSS32_MIX_MAX_CONTROLS          26

#define MAKE_VOLUME_LR(l, r)		((r << 8) | l)
#define OSS32_MAX_VOLUME                100UL
#define GET_VOLUME_R(vol)               (vol >> 8)
#define GET_VOLUME_L(vol)               (vol & 0xff)

//Do NOT change these definitions without changing string array in gpl\lib32\soundmixer.c!
#define OSS32_MIX_RECSRC_MIC		0
#define OSS32_MIX_RECSRC_CD		1
#define OSS32_MIX_RECSRC_LINE		2
#define OSS32_MIX_RECSRC_VIDEO		3
#define OSS32_MIX_RECSRC_AUX	 	4
#define OSS32_MIX_RECSRC_MIXER	 	5
#define OSS32_MIX_RECSRC_MIXER_MONO     6
#define OSS32_MIX_RECSRC_PHONE          7
#define OSS32_MIX_RECSRC_SYNTH          8
#define OSS32_MIX_RECSRC_MAX            9

#define OSS32_SETBIT(a)                 (1UL<<a)
#define OSS32_MIX_FLAG(a)		OSS32_SETBIT(a)

#define SIZE_DEVICE_NAME            32


typedef struct
{
    char  name[SIZE_DEVICE_NAME];
    ULONG fuCtrlCaps;
    ULONG fuRecCaps;
} OSS32_MIXCAPS, *POSS32_MIXCAPS, FAR *LPOSS32_MIXCAPS;


#define SIZE_DEVICE_NAME            32
typedef struct
{
    ULONG nrchannels;
    ULONG devicetype;
    char  name[SIZE_DEVICE_NAME];
    //TODO:
} OSS32_MIDICAPS, *POSS32_MIDICAPS, FAR *LPOSS32_MIDICAPS;

#define OSS32_CAPS_WAVE_PLAYBACK	1
#define OSS32_CAPS_WAVE_CAPTURE		2
#define OSS32_CAPS_FMSYNTH_PLAYBACK	4
#define OSS32_CAPS_WAVETABLE_PLAYBACK	8
#define OSS32_CAPS_MPU401_PLAYBACK	16
#define OSS32_CAPS_MPU401_CAPTURE	32

#define	OSS32_CAPS_PCM_FORMAT_S8 		OSS32_SETBIT(OSS32_PCM_FORMAT_S8)
#define	OSS32_CAPS_PCM_FORMAT_U8		OSS32_SETBIT(OSS32_PCM_FORMAT_U8)
#define	OSS32_CAPS_PCM_FORMAT_S16_LE		OSS32_SETBIT(OSS32_PCM_FORMAT_S16_LE)
#define	OSS32_CAPS_PCM_FORMAT_S16_BE		OSS32_SETBIT(OSS32_PCM_FORMAT_S16_BE)
#define	OSS32_CAPS_PCM_FORMAT_U16_LE		OSS32_SETBIT(OSS32_PCM_FORMAT_U16_LE)
#define	OSS32_CAPS_PCM_FORMAT_U16_BE		OSS32_SETBIT(OSS32_PCM_FORMAT_U16_BE)
#define OSS32_CAPS_PCM_FORMAT_S24_LE		OSS32_SETBIT(OSS32_PCM_FORMAT_S24_LE)
#define OSS32_CAPS_PCM_FORMAT_S24_BE		OSS32_SETBIT(OSS32_PCM_FORMAT_S24_BE)
#define OSS32_CAPS_PCM_FORMAT_U24_LE		OSS32_SETBIT(OSS32_PCM_FORMAT_U24_LE)
#define OSS32_CAPS_PCM_FORMAT_U24_BE		OSS32_SETBIT(OSS32_PCM_FORMAT_U24_BE)
#define OSS32_CAPS_PCM_FORMAT_S32_LE		OSS32_SETBIT(OSS32_PCM_FORMAT_S32_LE)
#define OSS32_CAPS_PCM_FORMAT_S32_BE		OSS32_SETBIT(OSS32_PCM_FORMAT_S32_BE)
#define OSS32_CAPS_PCM_FORMAT_U32_LE		OSS32_SETBIT(OSS32_PCM_FORMAT_U32_LE)
#define OSS32_CAPS_PCM_FORMAT_U32_BE		OSS32_SETBIT(OSS32_PCM_FORMAT_U32_BE)
#define	OSS32_CAPS_PCM_FORMAT_MULAW		OSS32_SETBIT(OSS32_PCM_FORMAT_MULAW)
#define	OSS32_CAPS_PCM_FORMAT_ALAW		OSS32_SETBIT(OSS32_PCM_FORMAT_ALAW)
#define	OSS32_CAPS_PCM_FORMAT_ADPCM		OSS32_SETBIT(OSS32_PCM_FORMAT_ADPCM)
#define OSS32_CAPS_PCM_FORMAT_MPEG		OSS32_SETBIT(OSS32_PCM_FORMAT_MPEG)

#define OSS32_CAPS_PCM_FORMAT_8BPS              (OSS32_CAPS_PCM_FORMAT_S8|OSS32_CAPS_PCM_FORMAT_U8)
#define OSS32_CAPS_PCM_FORMAT_16BPS             (OSS32_CAPS_PCM_FORMAT_S16_LE|OSS32_CAPS_PCM_FORMAT_S16_BE|OSS32_CAPS_PCM_FORMAT_U16_LE|OSS32_CAPS_PCM_FORMAT_U16_BE)
#define OSS32_CAPS_PCM_FORMAT_16BPS_SIGNED      (OSS32_CAPS_PCM_FORMAT_S16_LE|OSS32_CAPS_PCM_FORMAT_S16_BE)
#define OSS32_CAPS_PCM_FORMAT_16BPS_UNSIGNED    (OSS32_CAPS_PCM_FORMAT_U16_LE|OSS32_CAPS_PCM_FORMAT_U16_BE)
#define OSS32_CAPS_PCM_FORMAT_16BPS_LE          (OSS32_CAPS_PCM_FORMAT_S16_LE|OSS32_CAPS_PCM_FORMAT_U16_LE)
#define OSS32_CAPS_PCM_FORMAT_16BPS_BE          (OSS32_CAPS_PCM_FORMAT_S16_BE|OSS32_CAPS_PCM_FORMAT_U16_BE)
#define OSS32_CAPS_PCM_FORMAT_24BPS             (OSS32_CAPS_PCM_FORMAT_S24_LE|OSS32_CAPS_PCM_FORMAT_S24_BE|OSS32_CAPS_PCM_FORMAT_U24_LE|OSS32_CAPS_PCM_FORMAT_U24_BE)
#define OSS32_CAPS_PCM_FORMAT_24BPS_SIGNED      (OSS32_CAPS_PCM_FORMAT_S24_LE|OSS32_CAPS_PCM_FORMAT_S24_BE)
#define OSS32_CAPS_PCM_FORMAT_24BPS_UNSIGNED    (OSS32_CAPS_PCM_FORMAT_U24_LE|OSS32_CAPS_PCM_FORMAT_U24_BE)
#define OSS32_CAPS_PCM_FORMAT_24BPS_LE          (OSS32_CAPS_PCM_FORMAT_S24_LE|OSS32_CAPS_PCM_FORMAT_U24_LE)
#define OSS32_CAPS_PCM_FORMAT_24BPS_BE          (OSS32_CAPS_PCM_FORMAT_S24_BE|OSS32_CAPS_PCM_FORMAT_U24_BE)
#define OSS32_CAPS_PCM_FORMAT_32BPS             (OSS32_CAPS_PCM_FORMAT_S32_LE|OSS32_CAPS_PCM_FORMAT_S32_BE|OSS32_CAPS_PCM_FORMAT_U32_LE|OSS32_CAPS_PCM_FORMAT_U32_BE)
#define OSS32_CAPS_PCM_FORMAT_32BPS_SIGNED      (OSS32_CAPS_PCM_FORMAT_S32_LE|OSS32_CAPS_PCM_FORMAT_S32_BE)
#define OSS32_CAPS_PCM_FORMAT_32BPS_UNSIGNED    (OSS32_CAPS_PCM_FORMAT_U32_LE|OSS32_CAPS_PCM_FORMAT_U32_BE)
#define OSS32_CAPS_PCM_FORMAT_32BPS_LE          (OSS32_CAPS_PCM_FORMAT_S32_LE|OSS32_CAPS_PCM_FORMAT_U32_LE)
#define OSS32_CAPS_PCM_FORMAT_32BPS_BE          (OSS32_CAPS_PCM_FORMAT_S32_BE|OSS32_CAPS_PCM_FORMAT_U32_BE)

#define OSS32_CAPS_PCM_RATE_5512		OSS32_SETBIT(0)		/* 5512Hz */
#define OSS32_CAPS_PCM_RATE_8000		OSS32_SETBIT(1)		/* 8000Hz */
#define OSS32_CAPS_PCM_RATE_11025		OSS32_SETBIT(2)		/* 11025Hz */
#define OSS32_CAPS_PCM_RATE_16000		OSS32_SETBIT(3)		/* 16000Hz */
#define OSS32_CAPS_PCM_RATE_22050		OSS32_SETBIT(4)		/* 22050Hz */
#define OSS32_CAPS_PCM_RATE_32000		OSS32_SETBIT(5)		/* 32000Hz */
#define OSS32_CAPS_PCM_RATE_44100		OSS32_SETBIT(6)		/* 44100Hz */
#define OSS32_CAPS_PCM_RATE_48000		OSS32_SETBIT(7)		/* 48000Hz */
#define OSS32_CAPS_PCM_RATE_64000		OSS32_SETBIT(8)		/* 64000Hz */
#define OSS32_CAPS_PCM_RATE_88200		OSS32_SETBIT(9)		/* 88200Hz */
#define OSS32_CAPS_PCM_RATE_96000		OSS32_SETBIT(10)		/* 96000Hz */
#define OSS32_CAPS_PCM_RATE_176400		OSS32_SETBIT(11)		/* 176400Hz */
#define OSS32_CAPS_PCM_RATE_192000		OSS32_SETBIT(12)		/* 192000Hz */

#define OSS32_CAPS_PCM_RATE_HALFDUPLEX          OSS32_SETBIT(29)        /* playback & record sample rates must be identical if both active */
#define OSS32_CAPS_PCM_RATE_CONTINUOUS	        OSS32_SETBIT(30)	/* continuous range */
#define OSS32_CAPS_PCM_RATE_KNOT		OSS32_SETBIT(31)	/* supports more non-continuos rates */

// Channel flags
#define OSS32_CAPS_PCM_CHAN_MONO                OSS32_SETBIT(0)		// Mono
#define OSS32_CAPS_PCM_CHAN_STEREO              OSS32_SETBIT(1)		// Stereo
#define OSS32_CAPS_PCM_CHAN_QUAD                OSS32_SETBIT(2)		// 4 channels
#define OSS32_CAPS_PCM_CHAN_5_1                 OSS32_SETBIT(3)		// 5.1 channels

#define OSS32_MAX_WAVE_PLAYBACK_STREAMS         32
#define OSS32_MAX_WAVE_CAPTURE_STREAMS          1

typedef struct {
    ULONG  nrStreams;            //nr of activate wave streams supported
    ULONG  ulMinChannels;        //min nr of channels
    ULONG  ulMaxChannels;        //max nr of channels
    ULONG  ulChanFlags;          //channel flags
    ULONG  ulMinRate;            //min sample rate
    ULONG  ulMaxRate;            //max sample rate
    ULONG  ulRateFlags;          //sample rate flags
    ULONG  ulDataFormats;        //supported wave formats
} WAVE_CAPS, *PWAVE_CAPS, FAR *LPWAVE_CAPS;

typedef struct {
    ULONG     nrDevices;                   //total nr of audio devices
    ULONG     ulCaps; 	   	           //device caps
    char      szDeviceName[SIZE_DEVICE_NAME];
    char      szMixerName[SIZE_DEVICE_NAME];
    WAVE_CAPS waveOutCaps;
    WAVE_CAPS waveInCaps;
} OSS32_DEVCAPS, *POSS32_DEVCAPS, FAR *LPOSS32_DEVCAPS;

typedef struct {
    ULONG  ulSampleRate;
    ULONG  ulBitsPerSample;
    ULONG  ulNumChannels;
    ULONG  ulDataType;
    ULONG  ulPeriodSize;
} OSS32_HWPARAMS, *POSS32_HWPARAMS, FAR *LPOSS32_HWPARAMS;

#pragma pack()

#endif
