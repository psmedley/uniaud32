; $Id: end.asm,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ 
;*
;* Labels to mark end of code & data segments
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

	.386p

	include segments.inc

;Label to mark end of 32 bits code section
LASTCODE32 segment
       public __OffFinalCS32
__OffFinalCS32 label byte
LASTCODE32 ends

;Label to mark end of 32 bits data section

LASTDATA32 segment
       public __OffFinalDS32
    __OffFinalDS32 dd 0
LASTDATA32 ends

	end
