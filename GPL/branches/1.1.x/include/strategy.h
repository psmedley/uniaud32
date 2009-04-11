/* $Id: strategy.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * OS/2 strategy handler definitions
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

#ifndef STRATEGY_INCLUDED
#define STRATEGY_INCLUDED


/* Constants relating to the Strategy Routines
*/

#define RPDONE    0x0100         // return successful, must be set
#define RPBUSY    0x0200         // device is busy (or has no data to return)
#define RPDEV     0x4000         // user-defined error
#define RPERR     0x8000         // return error

// List of error codes, from chapter 8 of PDD reference
#define RPNOTREADY  0x0002
#define RPBADCMD    0x0003
#define RPGENFAIL   0x000c
#define RPDEVINUSE  0x0014
#define RPINITFAIL  0x0015

// list of Strategy commands from PDD reference
// Note this is only the list of commands audio device drivers care about
#define STRATEGY_INIT          0x00
#define STRATEGY_OPEN          0x0D
#define STRATEGY_CLOSE         0x0E
#define STRATEGY_GENIOCTL      0x10
#define STRATEGY_DEINSTALL     0x14
#define STRATEGY_INITCOMPLETE  0x1F

#endif
