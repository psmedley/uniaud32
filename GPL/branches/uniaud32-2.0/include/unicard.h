/* $Id: unicard.h,v 1.3 2003/08/08 15:09:03 vladest Exp $ */
/*
 * Header for debug functions
 *
 * (C) 2000-2002 InnoTek Systemberatung GmbH
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

#ifndef __UNICARD_H__
#define __UNICARD_H__

//#define CS46XX_SUPPORTED

#define CARD_NONE   	-1
#define CARD_SBLIVE 	0
#define CARD_CMEDIA 	1
#define CARD_ALS4000	2
#define CARD_CS4281 	3
#define CARD_ICH    	4
#define CARD_CS46XX     5
#define CARD_VIA82XX    6
#define CARD_ESS1938    7
#define CARD_ENSONIQ    8
#define CARD_YAMAHA     9
#define CARD_MAESTRO    10
#define CARD_MAESTRO3   11
#define CARD_ALI5451    12
#define CARD_TRIDENT    13
#define CARD_NEOMAGIC   14
#define CARD_ATIIXP     15
#define CARD_FM801      16
#define CARD_AUDIGYLS   17
#define CARD_BT87X      18
#define CARD_AZX        19

#define CARD_VORTEX     20 // unsupported

#define CARDS_NUM       20

#define CARD_STRING_SBLIVE	"SBLIVE"
#define CARD_STRING_CMEDIA	"CMEDIA"
#define CARD_STRING_ALS4000	"ALS4000"
#define CARD_STRING_CS4281	"CS4281"
#define CARD_STRING_ICH     	"ICH"
#define CARD_STRING_CS46XX  	"CS46XX"
#define CARD_STRING_VIA82XX 	"VIA82XX"
#define CARD_STRING_ESS1938     "ESS1938"
#define CARD_STRING_VORTEX      "VORTEX"
#define CARD_STRING_ENSONIQ     "ENSONIQ"
#define CARD_STRING_YAMAHA      "YAMAHA"
#define CARD_STRING_MAESTRO     "MAESTRO"
#define CARD_STRING_MAESTRO3    "MAESTRO3"
#define CARD_STRING_ALI5451     "ALI5451"
#define CARD_STRING_TRIDENT     "TRIDENT"
#define CARD_STRING_NEOMAGIC    "NEO"
#define CARD_STRING_ATIIXP      "ATIIXP"
#define CARD_STRING_FM801       "FM801"
#define CARD_STRING_AUDIGYLS    "AUDIGYLS"
#define CARD_STRING_BT87X       "BT87X"
#define CARD_STRING_AZX         "AZX"

#define CARD_MAX_LEN            16

extern int ForceCard;

#endif //__UNICARD_H__
