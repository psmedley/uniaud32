; $Id: devhlp.asm,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ 
;*
;* 32bit DevHelp implementations. Usually thunk down to the well known
;* 16bit OS/2 DevHelp calls
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

        include bseerr.inc
        include devhlp.inc
	include segments.inc

DosTable2 struc
        d2_ErrMap24                     dd ?
        d2_MsgMap24                     dd ?
        d2_Err_Table_24                 dd ?
        d2_CDSAddr                      dd ?
        d2_GDT_RDR1                     dd ?
        d2_InterruptLevel               dd ?
        d2__cInDos                      dd ?
        d2_zero_1                       dd ?
        d2_zero_2                       dd ?
        d2_FlatCS                       dd ?
        d2_FlatDS                       dd ?
        d2__TKSSBase                    dd ?
        d2_intSwitchStack               dd ?
        d2_privateStack                 dd ?
        d2_PhysDiskTablePtr             dd ?
        d2_forceEMHandler               dd ?
        d2_ReserveVM                    dd ?
        d2_pgpPageDir                   dd ?
        d2_unknown                      dd ?
DosTable2 ends

DATA32 segment
        public DevHelp32
        public intSwitchStack

DevHelp32	dd 0
intSwitchStack  dd 0

	public _TKSSBase
_TKSSBase	dd 0

DATA32 ends


CODE32 segment
ASSUME CS:FLAT, DS:FLAT, ES:FLAT

	public iodelay32_

        extrn  DevHlp : near
        extrn   DOSIODELAYCNT : ABS

	ALIGN 	4

	public GetTKSSBase

GetTKSSBase proc near
        push ebp
        mov  ebp, esp
        push es
        push ebx
 	push ecx
	push edx

        ;
        ; Gets the TKSSBase pointer from DosTable. TKSSBase is used by
        ; __StackToFlat() to convert a stack based address to a FLAT address
        ; without the overhead of DevHlp_VirtToLin
        ;
        ; DosTable is obtained through GetDOSVar with undocumented index 9
        ; The layout is :
        ;     byte      count of following dword (n)
        ;     dword 1   -+
        ;       .        |
        ;       .        | this is DosTable1
        ;       .        |
        ;     dword n   -+
        ;     byte      count of following dword (p)
        ;     dword 1   -+
        ;       .        |
        ;       .        | this is DosTable2
        ;       .        |
        ;     dword p   -+
        ;
        ; Flat CS  is dword number 10 in DosTable2
        ; Flat DS  is dword number 11 in DosTable2
        ; TKSSBase is dword number 12 in DosTable2
        ;
        mov 	eax, 9                       ; undocumented DosVar : DosTable pointer
        xor 	ecx, ecx
        mov 	edx, DevHlp_GetDOSVar
	call 	DevHlp
        jc 	short GetTKSSBase_Err
        mov 	es, ax                       ; es:bx points to DosTable
        movzx 	ebx, bx
        movzx 	ecx, byte ptr es:[ebx]     ; count of dword in DosTable1
        mov 	eax, es:[ebx + 4 * ecx + 2].d2__TKSSBase
        mov 	_TKSSBase, eax

        mov 	eax, es:[ebx + 4 * ecx + 2].d2_intSwitchStack
        mov     intSwitchStack, eax

        xor 	eax, eax
GetTKSSBase_Err:
	pop edx
	pop ecx
        pop ebx
        pop es
        leave
        ret
GetTKSSBase endp

	ALIGN 4

iodelay32_ proc near
	mov   eax, DOSIODELAYCNT
        align 4
@@:     dec   eax
        jnz   @b
        loop  iodelay32_
	ret
iodelay32_ endp

;iodelay32_ proc near
;	mov ax,DOSIODELAYCNT
;	top:
;	dec ax
;	jnz top
;iodelay32_ ENDP

CODE32 	ends

	end
