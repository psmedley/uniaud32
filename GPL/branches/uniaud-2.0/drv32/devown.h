/* $Id: devown.h,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ */
/*
 * Device owner flags
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


extern WORD32 deviceOwner;
extern WORD32 numOS2Opens;

#define DEV_NO_OWNER		0
#define DEV_PDD_OWNER		1       //owner is still unknown
#define DEV_PCM_OWNER		2	//PCM&MIDI
#define DEV_MIDI_OWNER		4	//MIDI&PCM
#define DEV_VDD_OWNER		8	//exclusive
