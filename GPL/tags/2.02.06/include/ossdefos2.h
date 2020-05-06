/* $Id: ossdefos2.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * Type definitions
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

#ifndef __OSSDEFOS2_H__
#define __OSSDEFOS2_H__

#define FAR48 __far
#ifndef FAR
#define FAR   __far
#endif

#ifdef LINUX
#define NEAR __near

typedef unsigned long ULONG;
typedef unsigned long *PULONG;
typedef unsigned short USHORT;
typedef unsigned short *PUSHORT;
typedef signed long LONG;
typedef signed short SHORT;
typedef signed short *PSHORT;
typedef unsigned long BOOL;
typedef ULONG FARPTR16;
typedef char NEAR *LINEAR;
typedef unsigned long APIRET;
typedef void VOID;
typedef void *PVOID;
typedef signed int INT;
typedef unsigned char UCHAR;
typedef signed char CHAR;
typedef CHAR *PCHAR;
typedef unsigned char BYTE;
typedef __int64 QWORD;
typedef ULONG HFILE;

#define APIENTRY    _System

#endif

#define OSS_MIXER_VOLUME	0
#define OSS_MIXER_BASS		1
#define OSS_MIXER_TREBLE	2
#define OSS_MIXER_SYNTH		3
#define OSS_MIXER_PCM		4
#define OSS_MIXER_PCSPEAKER	5
#define OSS_MIXER_LINE		6
#define OSS_MIXER_MIC		7
#define OSS_MIXER_CD		8
#define OSS_MIXER_IMIX		9	/*  Recording monitor  */
#define OSS_MIXER_ALTPCM	10
#define OSS_MIXER_RECLEV	11	/* Recording level */
#define OSS_MIXER_IGAIN		12	/* Input gain */
#define OSS_MIXER_OGAIN		13	/* Output gain */
#define OSS_MIXER_LINE1		14	/* Input source 1  (aux1) */
#define OSS_MIXER_LINE2		15	/* Input source 2  (aux2) */
#define OSS_MIXER_LINE3		16	/* Input source 3  (line) */
#define OSS_MIXER_DIGITAL1	17	/* Digital (input) 1 */
#define OSS_MIXER_DIGITAL2	18	/* Digital (input) 2 */
#define OSS_MIXER_DIGITAL3	19	/* Digital (input) 3 */
#define OSS_MIXER_PHONEIN	20	/* Phone input */
#define OSS_MIXER_PHONEOUT	21	/* Phone output */
#define OSS_MIXER_VIDEO		22	/* Video/TV (audio) in */
#define OSS_MIXER_RADIO		23	/* Radio in */
#define OSS_MIXER_MONITOR	24	/* Monitor (usually mic) volume */
#define OSS_MIXER_3DDEPTH       25
#define OSS_MIXER_3DCENTER      26
#define OSS_MIXER_FRONT         27
#define OSS_MIXER_SPEAKER       28
#define OSS_MIXER_HEADPHONE 	29
#define OSS_MIXER_NRDEVICES	30

#endif
