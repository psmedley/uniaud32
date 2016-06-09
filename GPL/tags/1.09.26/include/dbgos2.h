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

typedef struct {
	unsigned long ulState;
	unsigned long ulIntServiced[4];
	unsigned long ulIntUnserviced[4];
} DBGINT;

#ifdef __cplusplus
extern "C" {
#endif
extern int DebugLevel;
extern int wrOffset;
extern char *szprintBuf;
int _cdecl printk(const char * fmt, ...);
extern void DbgPrintIrq(void);
#ifdef __cplusplus
}
#endif


/* rprintf always prints to the log buffer, and to SIO if enabled */
#define rprintf(a) printk a

/* the dprintf functions only print if DEBUG is defined */
#ifdef DEBUG
#define DBG_MAX_BUF_SIZE 0x100000
#define dprintf(a)	printk a
#define dprintf1(a)	if(DebugLevel > 0) printk a
#define dprintf2(a)	if(DebugLevel > 1) printk a
#define dprintf3(a)	if(DebugLevel > 2) printk a
#define DebugInt3()	; //_asm int 3
//#define DebInt3()	_asm int 3;
#else
// not DEBUG
#define DBG_MAX_BUF_SIZE 0x10000
#define dprintf(a)
#define dprintf1(a)
#define dprintf2(a)
#define dprintf3(a)
#define DebugInt3()
#endif

#endif //__COMMDBG_H__

