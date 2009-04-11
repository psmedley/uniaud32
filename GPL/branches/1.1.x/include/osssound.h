/* $Id: osssound.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * Header for OSS constants
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

#ifndef __OSSSOUND_H__
#define __OSSSOUND_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OSS32_DSPID			0
#define OSS32_SYNTHID			1
#define OSS32_MIDIID			2
#define OSS32_MIXERID			3
#define OSS32_SPECIALID			4
#define OSS32_MAX_DEVICES 		5

#define OSS32_STREAM_WAVEOUT		0
#define OSS32_STREAM_WAVEIN    	 	1
#define OSS32_STREAM_MPU401_MIDIOUT	2
#define OSS32_STREAM_MPU401_MIDIIN	3
#define OSS32_STREAM_WAVETABLE_MIDIOUT	4
#define OSS32_STREAM_FM_MIDIOUT		5

extern struct file_operations oss_devices[OSS32_MAX_DEVICES];

#ifdef __cplusplus
}
#endif

#endif
