; $Id: segments.inc,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $
;*
;* Segment definition statements
;*
;* (C) 2000-2002 InnoTek Systemberatung GmbH
;* (C) 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)
;*
;* This program is free software; you can redistribute it and/or
;* modify it under the terms of the GNU General Public License as
;* published by the Free Software Foundation; either version 2 of
;* the License, or (at your option) any later version.
;*
;* This program is distributed in the hope that it will be useful,
;* but WITHOUT ANY WARRANTY; without even the implied warranty of
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;* GNU General Public License for more details.
;*
;* You should have received a copy of the GNU General Public
;* License along with this program; if not, write to the Free
;* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
;* USA.
;*


DATA16 segment word public use16 'DATA16'
DATA16 ends

CODE16 segment word public use16 'CODE'
CODE16 ends

FIRSTCODE32 segment dword public use32 'CODE'
FIRSTCODE32 ends

CODE32 segment dword public use32 'CODE'
CODE32 ends

_TEXT segment dword public use32 'CODE'
_TEXT ends

LASTCODE32 segment dword public use32 'CODE'
LASTCODE32 ends

BSS32   segment dword use32 public 'BSS'
BSS32   ends

_BSS   segment dword use32 public 'BSS'
_BSS   ends

C_COMMON segment dword use32 public 'DATA'
C_COMMON ends

c_common segment dword use32 public 'DATA'
c_common ends

CONST32 segment dword use32 public 'CONST'
CONST32 ends

DATA32  segment dword use32 public 'DATA'
DATA32  ends

_DATA  segment dword use32 public 'DATA'
_DATA  ends

CONST   SEGMENT DWORD PUBLIC USE32 'DATA'
CONST   ENDS

CONST2  SEGMENT DWORD PUBLIC USE32 'DATA'
CONST2  ENDS

LASTDATA32 SEGMENT DWORD PUBLIC USE32 'DATA'
LASTDATA32 ENDS

;order is important!
DGROUP  group BSS32, DATA32, _DATA, CONST32, C_COMMON, c_common, CONST2, CONST, _BSS, LASTDATA32

CGROUP  group FIRSTCODE32, CODE32, _TEXT, LASTCODE32
