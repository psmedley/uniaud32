/* $Id: stacktoflat.h,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ */
/*
 * Header for SS / DS conversion routines
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

#ifndef __STACKTOFLAT_H__
#define __STACKTOFLAT_H__

extern ULONG TKSSBase;
#pragma aux TKSSBase "_TKSSBase"

extern ULONG GetTKSSBase();
#pragma aux GetTKSSBase "GetTKSSBase" \
  value [eax];

#ifdef KEE
extern ULONG stacksel;		//16 bits stack selector
#pragma aux stacksel "stacksel"

extern ULONG stackbase;		//32 bits stackbase
#pragma aux stackbase "stackbase"
#endif

#ifdef FLATSTACK

#ifdef KEE
//Convert 16:16 stack based address to 0:32 flat addresss
#define __Stack16ToFlat(addr)	(LINEAR)((ULONG)(addr&0xffff) + stackbase)
#else
//Convert 16:16 stack based address to 0:32 flat addresss
#define __Stack16ToFlat(addr)	(LINEAR)((((ULONG)addr)&0xffff) + *(ULONG *)TKSSBase)
#endif

//stack is already flat
#define __Stack32ToFlat(addr)	(LINEAR)addr

#else
//Convert 16:16 stack based address to 0:32 flat addresss
#define __Stack16ToFlat(addr)	(LINEAR)((((ULONG)addr)&0xffff) + *(ULONG *)TKSSBase)

//Convert 16:16 stack based address to 0:32 flat addresss
#define __Stack32ToFlat(addr)	(LINEAR)((((ULONG)addr)&0xffff) + *(ULONG *)TKSSBase)
#endif

// Convert 16:16 pointer to 16:32
char FAR48 *MAKE_FARPTR32(FARPTR16 addr1616);
#pragma aux MAKE_FARPTR32 =          \
  "movzx edx, ax"   \
  "shr   eax, 16"   \
  "mov   fs, ax"    \
  parm [eax]        \
  value [fs edx];

//Only valid for pointer previously constructed by __Make48Pointer!!
//(upper 16 bits of 32 bits offset must be 0)
FARPTR16 MAKE_FARPTR16(void FAR48 *addr1632);
#pragma aux MAKE_FARPTR16 = \
  "mov   ax, gs"    \
  "shl   eax, 16"   \
  "mov   ax, dx"    \
  parm [gs edx]     \
  value [eax];

FARPTR16 MAKE_FARPTR16_STACK(LINEAR offset);
#pragma aux MAKE_FARPTR16_STACK = \
  "mov   ax, ss"    \
  "shl   eax, 16"   \
  "mov   ax, dx"    \
  parm [edx]     \
  value [eax];

char FAR48 *MAKE_FARPTR32_STACK(LINEAR offset);
#pragma aux MAKE_FARPTR32_STACK =          \
  "push  ss" \
  "pop   fs" \
  parm [eax]        \
  value [fs eax];

#define MAKE_FP16(sel, offset)	((sel << 16) | (offset & 0xffff) )

USHORT GETFARSEL(char FAR48 *addr1632);
#pragma aux GETFARSEL = \
  "mov   ax, gs"    \
  parm [gs edx]     \
  value [ax];

ULONG GETFAROFFSET(char FAR48 *addr1632);
#pragma aux GETFAROFFSET = \
  "mov   eax, edx"  \
  parm [gs edx]     \
  value [eax];

//SvL: Only works for DS & SS ptrs!
ULONG GETFLATPTR(char FAR48 *ptr);

#define FLATPTR(a)	GETFLATPTR((char FAR48 *)a)

#ifdef KEE
#define FlatToSel(addr32)	((stacksel << 16) | (((ULONG)addr32 - stackbase) & 0xffff))
#else
//Only valid for stack based pointer!!
ULONG FlatToSel(ULONG addr32);
#endif

#endif
