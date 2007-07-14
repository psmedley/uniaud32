/* $Id: dbgos2.h,v 1.2 2003/08/08 15:09:03 vladest Exp $ */
/*
 * Header for debug functions
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

#ifndef __COMMDBG_H__
#define __COMMDBG_H__

//#ifdef DEBUG
#if 1
#ifdef __cplusplus
extern "C" {
#endif
extern int DebugLevel;

void _cdecl DPE(char *x, ...) ; /* not debugging: nothing */
void _cdecl DPD(int level, char *x, ...) ; /* not debugging: nothing */
#ifdef __cplusplus
}
#endif
#define dprintf(a)	if(DebugLevel > 0) DPE a
#define dprintf2(a)	if(DebugLevel > 1) DPE a
#define dprintf3(a)	if(DebugLevel > 2) DPE a
#define DebugInt3()	; //_asm int 3;
#else // #ifndef DEBUG
#define dprintf(a)
#define dprintf2(a)
#define dprintf3(a)
#define DebugInt3()     ;
#endif // if DEBUG

#endif //__COMMDBG_H__
