; $Id: startup.inc,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ 
;*
;* Assembly macro and constant definitions
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


; MACRO'S
;*******************************************************************************
;enter a 16:32 far function
;*******************************************************************************
enter32         MACRO
		push	bp
		mov	bp, sp
		push	ds
		mov	ax, DATA16
		mov	ds, ax
		ENDM
;*******************************************************************************
;return to 16:32 far caller
;*******************************************************************************
ret32           MACRO   
		pop	ds
		pop	bp
	        db      66h                             ; Force next instruction 32-bit
        	ret
                ENDM

enter16         MACRO
		push	bp
		mov	bp, sp
		ENDM
ret16           MACRO
		pop	bp
		ret
		ENDM

;pushfd/popfd trashed by fucking BUGY KernThunkStackTo32
;fix idea by zuko

;;******************************************************************************
;;Need to preserve fs:ebx!! (all other registers must be saved by the caller)
;;******************************************************************************
DevThunkStackTo32 MACRO
	push	stacksel
	push	stackbase
	push	edx
	mov	edx, ss
	mov	stacksel, edx
        pushfd
        call	KernThunkStackTo32
        popfd
	mov	stackbase, edx
	pop	edx      ;trashed by KernThunkStackTo32
        ENDM

;;******************************************************************************
;;******************************************************************************
DevThunkStackTo32_Int MACRO
	push	eax
	push	edx
        pushfd
        call	KernThunkStackTo32
        popfd
	pop	edx	;trashed by KernThunkStackTo32
	pop	eax	;trashed by KernThunkStackTo32
        ENDM

;;******************************************************************************
;;******************************************************************************
DevThunkStackTo16 MACRO
        push    edx
	push	eax
        pushfd
        call	KernThunkStackTo16
        popfd
	pop	eax	;trashed by KernThunkStackTo16
        pop     edx     ;trashed by KernThunkStackTo16 when called in interrupt context
	pop	stackbase
	pop	stacksel
        ENDM
        
;;******************************************************************************
;;******************************************************************************
DevThunkStackTo16_Int MACRO
        push    edx
	push	eax
        pushfd
        call	KernThunkStackTo16
        popfd
	pop	eax	;trashed by KernThunkStackTo16
        pop     edx     ;trashed by KernThunkStackTo16 when called in interrupt context
        ENDM

;*******************************************************************************
;enter a function that needs to switch to a 16 bits stack
;*******************************************************************************
enterKEERM      MACRO
		push	ebp
		mov	ebp, esp
                push    ds
                push    es
                push    ebx
                push    ecx
                push    edx
                push    esi
                push    edi
                ;to access the parameters on the 32 bits stack, once we've switched
                mov     edi, ebp
	        DevThunkStackTo16_Int
                ENDM

;*******************************************************************************
;switch stack from 16 bits to 32 bits and return
;*******************************************************************************
retKEERM        MACRO
                DevThunkStackTo32_Int
                pop     edi
                pop     esi
                pop     edx
                pop     ecx
                pop     ebx
                pop     es
                pop     ds
		pop	ebp
		ret
                ENDM

;;******************************************************************************
;;******************************************************************************


;Constants and structure definitions

; Status word masks
STERR	EQU	8000H		; Bit 15 - Error
STINTER EQU	0400H		; Bit 10 - Interim character
STBUI	EQU	0200H		; Bit  9 - Busy
STDON	EQU	0100H		; Bit  8 - Done
STECODE EQU	00FFH		; Error code

; Definition of the request packet header.

reqPacket       struc
reqLenght       db ?
reqUnit         db ?
reqCommand      db ?
reqStatus       dw ?
reqFlags        db ?
                db 3 dup (?)    ; Reserved field
reqLink         dd ?
reqPacket       ends

rpInitIn        struc
i_rph           db size reqPacket dup (?)
i_unit          db ?
i_devHelp       dd ?
i_initArgs      dd ?
i_driveNum      db ?
rpInitIn        ends

rpInitOut       struc
o_rph           db size reqPacket dup (?)
o_unit          db ?
o_codeend	dw ?
o_dataend	dw ?
o_bpbarray	dd ?
o_status	dw ?
rpInitOut       ends
