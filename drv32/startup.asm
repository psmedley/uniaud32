; $Id: startup.asm,v 1.1.1.1 2003/07/02 13:56:56 eleph Exp $ 
;*
;* 16bit entrypoints to the PDD with thunks to the 32bit functions
;*
;* (C) 2000-2002 InnoTek Systemberatung GmbH
;* (C) 1998-2001 Sander van Leeuwen (sandervl@xs4all.nl)
;*
;* Partly based on MWDD32 (32 bits OS/2 device driver and IFS support driver)
;* Copyright (C) 1995, 1996 Matthieu WILLM
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


        INCL_DOS        equ 1
        INCL_DOSERRORS  equ 1
        include os2.inc

	include segments.inc
        include startup.inc

DATA16 segment
		extrn DOS32FLATDS : abs                ; ring 0 FLAT kernel data selector
  	        public __OffFinalDS16
		public help_header
		public uniaud_header
		public _MSG_TABLE16
		public DevHelpInit
		public fOpen	
		public InitPktSeg
		public InitPktOff
		public _MESSAGE_STR
		public pddname16
		public FileName
 	        public _RM_Help0
 	        public _RM_Help1
	        public _RM_Help3
	        public _RMFlags

;*********************************************************************************************
;************************* Device Driver Header **********************************************
;*********************************************************************************************
help_header     dw OFFSET DATA16:uniaud_header         ; Pointer to next driver
		dw SEG DATA16:uniaud_header
                dw 1000100110000000b            ; Device attributes
;                  ||||| +-+   ||||
;                  ||||| | |   |||+------------------ STDIN
;                  ||||| | |   ||+------------------- STDOUT
;                  ||||| | |   |+-------------------- NULL
;                  ||||| | |   +--------------------- CLOCK
;                  ||||| | |
;                  ||||| | +------------------------+ (001) OS/2
;                  ||||| |                          | (010) DosDevIOCtl2 + SHUTDOWN
;                  ||||| +--------------------------+ (011) Capability bit strip
;                  |||||
;                  ||||+----------------------------- OPEN/CLOSE (char) or Removable (blk)
;                  |||+------------------------------ Sharing support
;                  ||+------------------------------- IBM
;                  |+-------------------------------- IDC entry point
;                  +--------------------------------- char/block device driver

                dw offset CODE16:help_stub_strategy       ; Strategy routine entry point
                dw 0				    ; IDC routine entry point
                db 'ALSAHLP$'                   ; Device name
                db 8 dup (0)                    ; Reserved
                dw 0000000000010011b            ; Level 3 device driver capabilities
;                             |||||
;                             ||||+------------------ DosDevIOCtl2 + Shutdown
;                             |||+------------------- More than 16 MB support
;                             ||+-------------------- Parallel port driver
;                             |+--------------------- Adapter device driver
;                             +---------------------- InitComplete
                dw 0000000000000000b

uniaud_header    dd -1
                dw 1101100110000000b            ; Device attributes
;                  ||||| +-+   ||||
;                  ||||| | |   |||+------------------ STDIN
;                  ||||| | |   ||+------------------- STDOUT
;                  ||||| | |   |+-------------------- NULL
;                  ||||| | |   +--------------------- CLOCK
;                  ||||| | |
;                  ||||| | +------------------------+ (001) OS/2
;                  ||||| |                          | (010) DosDevIOCtl2 + SHUTDOWN
;                  ||||| +--------------------------+ (011) Capability bit strip
;                  |||||
;                  ||||+----------------------------- OPEN/CLOSE (char) or Removable (blk)
;                  |||+------------------------------ Sharing support
;                  ||+------------------------------- IBM
;                  |+-------------------------------- IDC entry point
;                  +--------------------------------- char/block device driver

                dw offset CODE16:uniaud_stub_strategy ; Strategy routine entry point
                dw offset CODE16:uniaud_stub_idc      ; IDC routine entry point
                db 'ALSA32$ '                   ; Device name
                db 8 dup (0)                    ; Reserved
                dw 0000000000010011b            ; Level 3 device driver capabilities
;                             |||||
;                             ||||+------------------ DosDevIOCtl2 + Shutdown
;                             |||+------------------- More than 16 MB support
;                             ||+-------------------- Parallel port driver
;                             |+--------------------- Adapter device driver
;                             +---------------------- InitComplete
                dw 0000000000000000b

DevHelpInit	dd 0
fOpen		dd 0
InitPktSeg	dw 0
InitPktOff	dw 0
;needed for rmcalls.lib
_RM_Help0       dd 0
_RM_Help1       dd 0
_RM_Help3       dd 0
_RMFlags        dd 0
_MESSAGE_STR    db 1024 dup (0)
_MSG_TABLE16    dw 0	;message length
		dw OFFSET _MESSAGE_STR	;message far pointer
		dw SEG    _MESSAGE_STR

pddname16	db 'ALSA32$'
FileName 	db "ALSAHLP$", 0
ResMgr          DB  52H,45H,53H,4dH,47H,52H,24H,20H
                DB  00H
_RMIDCTable     DB  00H,00H,00H,00H,00H,00H,00H,00H
                DB  00H,00H,00H,00H

;last byte in 16 bits data segment
__OffFinalDS16 label byte

DATA16 ends

CODE16 segment
        assume cs:CODE16, ds:DATA16

        public __OffFinalCS16

        public help_stub_strategy
        public uniaud_stub_strategy
	public uniaud_stub_idc
	public uniaud_stub_timer
	public thunk3216_devhelp
	public thunk3216_devhelp_modified_ds
        extrn DOSOPEN       : far
        extrn DOSWRITE      : far
        extrn DOSCLOSE      : far

        ALIGN 2
help_stub_strategy proc far
        movzx 	eax, byte ptr es:[bx].reqCommand
        cmp 	eax, 04h			; DosRead
        je 	uniaud_stub_strategy
        
        enter   0, 0
        and     sp, 0fffch			; align stack

	pushad  
	push	ds
	push	es
	push	fs
	push	gs

	mov	dx, DATA16
	mov	ds, dx

	cmp	eax, 0				; Init
	je	short @@help_init
	cmp	eax, 0Eh			; DosClose
	je	short @@help_close
        cmp 	eax, 0Dh			; DosOpen
        jne 	short @@help_error
;DosOpen:
	cmp 	word ptr fOpen, 0
	je	short @@help_ret_ok		; not ours
	push	ebx				; save ebx
	push	es
	mov	word ptr fOpen, 0
	mov	ax, word ptr InitPktSeg
	mov	fs, ax				; fs:ebx = req. packet
	xor	ebx, ebx
	mov     bx, word ptr InitPktOff
	call	far ptr FLAT:STRATEGY_
	pop	es
	pop	ebx				; restore ebx ptr
@@help_ret:
        mov 	word ptr es:[bx].reqStatus, ax
@@help_ret_error:
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad

        leave
	ret
		
@@help_init:
	mov	eax, dword ptr es:[bx].i_devHelp
	mov	dword ptr DevHelpInit, eax
        mov 	word ptr es:[bx].o_codeend, offset __OffFinalCS16
        mov 	word ptr es:[bx].o_dataend, offset __OffFinalDS16

@@help_ret_ok:
	mov	ax, STDON
	jmp	short @@help_ret
	
@@help_close:
	call	far ptr FLAT:HelpClose
	jmp	short @@help_ret_ok

@@help_error:
	mov	ax, STDON + STERR + ERROR_I24_BAD_COMMAND
        mov 	word ptr es:[bx].reqStatus, ax
	jmp	short @@help_ret_error

help_stub_strategy endp

        ALIGN 2
uniaud_stub_strategy proc far
        enter   0, 0
        and     sp, 0fffch			; align stack

	pushad  
	push	ds
	push	es
	push	fs
	push	gs

	mov	ax, DATA16
	mov	ds, ax

        movzx 	eax, byte ptr es:[bx].reqCommand
        cmp 	eax, 0
        jz 	short @@init

	push	ebx
	push	es
	mov	ax, bx
	xor	ebx, ebx			
	mov	bx, ax
	mov	ax, es
	mov	fs, ax				; fs:ebx = req. packet

        call 	far ptr FLAT:STRATEGY_     ; 32 bits strategy entry point

	pop	es
	pop	ebx				; oude bx ptr
        mov 	word ptr es:[bx].reqStatus, ax  ; status code

@@uniaud_ret:

	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad
        leave
        ret

@@init:
        ;
        ; DEVICE= initialization
        ;
	mov  	word ptr InitPktSeg, es
	mov	word ptr InitPktOff, bx
	inc	word ptr fOpen
	call 	device_init

        mov 	word ptr es:[bx].reqStatus, ax  ; status code (ret by device_init)
        mov 	word ptr es:[bx].o_codeend, offset __OffFinalCS16
        mov 	word ptr es:[bx].o_dataend, offset __OffFinalDS16
	jmp 	short @@uniaud_ret

init_err:
	mov 	dword ptr es:[bx].i_devHelp, 0
	jmp 	short @@uniaud_ret

uniaud_stub_strategy endp

;in: cx = cmd
;    bx = lower 16 bits of ULONG parameter
;    dx = upper 16 bits of ULONG parameter
;return value in dx:ax
        ALIGN 2
uniaud_stub_idc proc far
        enter   0, 0
        and     sp, 0fffch			; align stack

	shl	edx, 16
	mov	dx, bx
        call 	far ptr FLAT:IDC_     ; 32 bits strategy entry point

	mov	dx, ax
	shr	eax, 16
	xchg	ax, dx

        leave
	retf
uniaud_stub_idc endp

        ALIGN 2
uniaud_stub_timer proc far
        enter   0, 0
        and     sp, 0fffch			; align stack

        call 	far ptr FLAT:TIMER_             ; 32 bits timer entry point

        leave
	retf
uniaud_stub_timer endp


;;*****************************************************************************
; device_init
;
; Use DosOpen to tell the 1st driver to handle init for us. We must do it this 
; way since right now our CPL is 3 and the flat code selector has DPL 0, so
; we can't load it. In the open strategy request, CPL is 0
;;*****************************************************************************
        ALIGN 2
device_init proc near
	enter 	24, 0
	push 	ds
	push 	es
	push 	bx
	push 	si
	push 	di

        ; bp      ->  old bp
        ; bp - 2  -> FileHandle
        ; bp - 4  -> ActionTaken
        ; bp - 8  -> IOCTL parm (4 bytes)  : union mwdd32_ioctl_init_device_parm
        ; bp - 24 -> IOCTL data (16 bytes) : union mwdd32_ioctl_init_device_data

        ;
        ; Opens wathlp$
        ;
        push 	seg DATA16                 ; seg  FileName
        push 	offset FileName            ; ofs  FileName
        push 	ss                         ; seg &FileHandle
        lea 	ax, [bp - 2]
        push 	ax                         ; ofs &FileHandle
        push 	ss                         ; seg &ActionTaken
        lea 	ax, [bp - 4]
        push 	ax                         ; ofs &ActionTaken
        push 	dword ptr 0                ; file size
        push 	0                          ; file attributes
        push 	OPEN_ACTION_FAIL_IF_NEW + OPEN_ACTION_OPEN_IF_EXISTS
        push 	OPEN_SHARE_DENYNONE + OPEN_ACCESS_READONLY
        push 	dword ptr 0                ; reserved
        call 	DOSOPEN
        cmp 	ax, NO_ERROR
        jnz 	short @@error


        ;
        ; Closes wathlp$
        ;
        push 	word ptr [bp - 2]                   ; FileHandle
        call 	DOSCLOSE
        cmp 	ax, NO_ERROR
        jnz 	short @@error

@@out:
	push	eax		;gemold door doswrite

        push    0001H
        push    ds
        push    offset _MESSAGE_STR
        push    word ptr _MSG_TABLE16
	push	ss
        lea     dx, [bp - 2]
        push    dx
        call    DOSWRITE

	pop	eax

	pop 	di
	pop 	si
	pop 	bx
	pop 	es
	pop 	ds
	leave
	ret
@@error:
	mov 	ax, STDON + STERR + ERROR_I24_GEN_FAILURE
	jmp 	short @@out

device_init endp

	ALIGN   2
;use devhlp pointer stored in 16 bits code segment
thunk3216_devhelp:
	push	ds
	push	DATA16
	pop	ds
        call 	dword ptr DevHelpInit
	pop	ds

        jmp 	far ptr FLAT:thunk1632_devhelp

	ALIGN 	2
thunk3216_devhelp_modified_ds:
	push	gs
	push	DATA16
	pop	gs
        call 	dword ptr gs:DevHelpInit
	pop	gs
        jmp 	far ptr FLAT:thunk1632_devhelp_modified_ds


                ALIGN 2
                PUBLIC  _RMAllocResourceOrg
_RMAllocResourceOrg proc far
		enter16
                test    byte ptr _RMFlags, 01H
                je      short AllocL1
                lea     eax, [bp+6]
                push    ss
                push    ax
                push    0008H
                push    cs
                call    near ptr _CallRM
                mov     sp,bp
                ret16
AllocL1:        test    byte ptr _RMFlags,02H
                je      short AllocL2
		push	es
		push	bx
                les     bx,dword ptr [bp+10]
                mov     word ptr es:[bx],0ffffH
                mov     word ptr es:+2H[bx],0ffffH
                sub     ax,ax
		pop	bx
		pop	es
                ret16
AllocL2:        mov     ax,0001H
                ret16
_RMAllocResourceOrg endp

                ALIGN 2
                PUBLIC  _RMAllocResource16
_RMAllocResource16 proc far
		enter32
		xor	eax, eax
		push	dword ptr [bp+18]
		push	dword ptr [bp+14]
		push	dword ptr [bp+10]
		call	_RMAllocResourceOrg
		add	sp, 12
                ret32
_RMAllocResource16 endp

                ALIGN 2
                PUBLIC  _RMCreateAdapterOrg
_RMCreateAdapterOrg proc far
		enter16
                test    byte ptr _RMFlags,01H
                je      short CreateAdL1
                lea     eax, [bp+6]
                push    ss
                push    ax
                push    0004H
                push    cs
                call    near ptr _CallRM
                mov     sp,bp
                ret16
CreateAdL1:     test    byte ptr _RMFlags,02H
                je      short CreateAdL2
		push	es
		push	bx
                les     bx,dword ptr [bp+10]
                mov     word ptr es:[bx],0ffffH
                mov     word ptr es:+2H[bx],0ffffH
                sub     ax,ax
		pop	bx
		pop	es
                ret16
CreateAdL2:     mov     ax,0001H
                ret16
_RMCreateAdapterOrg endp

                ALIGN 2
                PUBLIC  _RMCreateAdapter16
_RMCreateAdapter16 proc far
		enter32
		xor	eax, eax
		push	dword ptr [bp+26]
		push	dword ptr [bp+22]
		push	dword ptr [bp+18]
		push	dword ptr [bp+14]
		push	dword ptr [bp+10]
		call	_RMCreateAdapterOrg
		add	sp, 20
                ret32
_RMCreateAdapter16 endp

                ALIGN 2
                PUBLIC  _RMCreateDeviceOrg
_RMCreateDeviceOrg proc far
		enter16
                test    byte ptr _RMFlags,01H
                je      short CreateDevL1
                lea     ax, [bp+6]
                push    ss
                push    ax
                push    0006H
                push    cs
                call    near ptr _CallRM
                mov     sp,bp
                ret16
CreateDevL1:    test    byte ptr _RMFlags,02H
                je      short CreateDevL2
		push	es
		push	bx
                les     bx,dword ptr [bp+10]
                mov     word ptr es:[bx],0ffffH
                mov     word ptr es:+2H[bx],0ffffH
                sub     ax,ax
		pop	bx
		pop	es
                ret16
CreateDevL2:    mov     ax,0001H
                ret16
_RMCreateDeviceOrg endp

                ALIGN 2
                PUBLIC  _RMCreateDevice16
_RMCreateDevice16 proc far
		enter32
		xor	eax, eax
		push	dword ptr [bp+26]
		push	dword ptr [bp+22]
		push	dword ptr [bp+18]
		push	dword ptr [bp+14]
		push	dword ptr [bp+10]
		call	_RMCreateDeviceOrg
		add	sp, 20
                ret32
_RMCreateDevice16 endp

                ALIGN 2
                PUBLIC  _RMDestroyDeviceOrg
_RMDestroyDeviceOrg proc far
		enter16
                test    byte ptr _RMFlags,01H
                je      short DestroyL1
                lea     ax, [bp+6]
                push    ss
                push    ax
                push    0007H
                push    cs
                call    near ptr _CallRM
                mov     sp,bp
                ret16
DestroyL1:      test    byte ptr _RMFlags,02H
                je      short DestroyL2
                sub     ax,ax
                ret16
DestroyL2:      mov     ax,0001H
                ret16
_RMDestroyDeviceOrg endp

                ALIGN 2
                PUBLIC  _RMDestroyDevice16
_RMDestroyDevice16 proc far
		enter32
		xor	eax, eax
		push	dword ptr [bp+10]
		call	_RMDestroyDeviceOrg
		add	sp, 4
                ret32
_RMDestroyDevice16 endp

                ALIGN 2
                PUBLIC  _RMDeallocResourceOrg
_RMDeallocResourceOrg proc far
		enter16
                test    byte ptr _RMFlags,01H
                je      short DeAllocL1
                lea     ax, [bp+6]
                push    ss
                push    ax
                push    0009H
                push    cs
                call    near ptr _CallRM
                mov     sp,bp
                ret16     
DeAllocL1:      test    byte ptr _RMFlags,02H
                je      short DeAllocL2
                sub     ax,ax
                ret16     
DeAllocL2:      mov     ax,0001H
                ret16     
_RMDeallocResourceOrg endp

                ALIGN 2
                PUBLIC  _RMDeallocResource16
_RMDeallocResource16 proc far
		enter32
		xor	eax, eax
		push	dword ptr [bp+14]
		push	dword ptr [bp+10]
		call	_RMDeallocResourceOrg
		add	sp, 8
                ret32
_RMDeallocResource16 endp

                ALIGN 2
                PUBLIC  MY_DEVHELP_ATTACHDD 
MY_DEVHELP_ATTACHDD proc near
                push    bp
                mov     bp,sp
                push    di
		push	es
		mov	ax, ds
		mov	es, ax
                mov     bx,word ptr +8H[bp]
                mov     di,word ptr +6H[bp]
                mov     dl,2aH
                call 	dword ptr DevHelpInit
                jb      short L2
                sub     ax,ax
		pop	es
                pop     di
                leave   
                ret     0004H
L2:		pop	es
		pop	di
                leave   	
                ret     0004H
MY_DEVHELP_ATTACHDD endp

                ALIGN 2
                PUBLIC  _RMCreateDriverOrg
_RMCreateDriverOrg proc far
		enter16
                test    byte ptr _RMFlags,01H
                je      short CreateDrL4
CreateDrL3:     lea     ax, [bp+6]
                push    ss
                push    ax
                push    0002H
                push    cs
                call    near ptr _CallRM
                add     sp,0006H
                ret16     
CreateDrL4:     test    byte ptr _RMFlags,02H
                je      short CreateDrL6
CreateDrL5:     push	es
		push	bx
		les     bx,dword ptr [bp+10]
                mov     word ptr es:[bx],0ffffH
                mov     word ptr es:+2H[bx],0ffffH
                sub     ax,ax
		pop	bx
		pop	es
                ret16     
CreateDrL6:     mov     ax, word ptr DevHelpInit + 2H
                or      ax, word ptr DevHelpInit
                jne     short CreateDrL7
                mov     ax,0008H
                ret16    
CreateDrL7:     push    offset ResMgr
                push    offset _RMIDCTable
                push    cs
                call    near ptr MY_DEVHELP_ATTACHDD
                or      ax,ax
                je      short CreateDrL8
                or      byte ptr _RMFlags, 02H
                jmp     short CreateDrL5
CreateDrL8:     mov     ax,word ptr _RMIDCTable+4H
                mov     word ptr  _RM_Help3+2H,ax
                mov     word ptr _RM_Help3,0000H
                mov     ax,word ptr  _RMIDCTable+2H
                mov     word ptr  _RM_Help0+2H,ax
                mov     ax,word ptr  _RMIDCTable+6H
                mov     word ptr _RM_Help0,ax
                or      byte ptr _RMFlags,05H
                jmp     short CreateDrL3
_RMCreateDriverOrg endp

                ALIGN 2
                PUBLIC  _RMCreateDriver16
_RMCreateDriver16 proc far
		enter32
		xor	eax, eax
		push	dword ptr [bp+14]
		push	dword ptr [bp+10]
		call	_RMCreateDriverOrg
		add	sp, 8
                ret32
_RMCreateDriver16 endp

                ALIGN 2
                PUBLIC  _CallRM 
_CallRM proc near
	        enter   0002H,00H
                call    near ptr _GetCS
                test    al,03H
                je      short L1
                push    word ptr +0aH[bp]
                push    word ptr +8H[bp]
                push    word ptr +6H[bp]
                call    dword ptr _RM_Help3
                add     sp,0006H
                leave   
                ret
L1:             push    word ptr +6H[bp]
                call    dword ptr _RM_Help0
                leave   
                ret
_CallRM endp

                ALIGN 2
_GetCS proc near
	        enter   0002H,00H
                push    cs
                pop     word ptr -2H[bp]
                mov     ax,word ptr -2H[bp]
                leave   
                ret     
_GetCS endp

                ALIGN 2
                PUBLIC  _RMDestroyDriverOrg
_RMDestroyDriverOrg proc far
		enter16
                test    byte ptr _RMFlags,01H
                je      short DestroyDrvL1
                lea     ax, [bp+6]
                push    ss
                push    ax
                push    0003H
                push    cs
                call    near ptr _CallRM
                mov     sp,bp
                ret16
DestroyDrvL1:   test    byte ptr _RMFlags,02H
                je      short DestroyDrvL2
                sub     ax,ax
                ret16   
DestroyDrvL2:   mov     ax,0001H
                ret16
_RMDestroyDriverOrg endp

                ALIGN 2
                PUBLIC  _RMDestroyDriver16
_RMDestroyDriver16 proc far
		enter32
		xor	eax, eax
		push	dword ptr [bp+10]
		call	_RMDestroyDriverOrg
		add	sp, 4
                ret32
_RMDestroyDriver16 endp


                ALIGN 2
_RMGetNodeInfoOrg proc far
		enter16
                test    byte ptr _RMFlags,01H
                je      short GetNodeInfo_L3
                test    byte ptr _RMFlags,04H
                je      short GetNodeInfo_L1
                lea     ax,+6H[bp]
                push    ss
                push    ax
                push    001cH
                push    cs
                call    near ptr _CallRM
                mov     sp,bp
                ret16
GetNodeInfo_L1: mov     ax,0014H
                ret16
                nop     
GetNodeInfo_L2: test    byte ptr _RMFlags,02H
                je      short GetNodeInfo_L3
                mov     ax,0015H
		ret16
GetNodeInfo_L3: mov     ax,0001H
                ret16
_RMGetNodeInfoOrg endp

                ALIGN 2
                PUBLIC  _RMGetNodeInfo16
_RMGetNodeInfo16  proc far 
		enter32
		xor	eax, eax
		push	word ptr  [bp+18]	;pushed as dword by watcom	
		push	dword ptr [bp+14]	
		push	dword ptr [bp+10]
		call	_RMGetNodeInfoOrg
		add	sp, 10
                ret32
_RMGetNodeInfo16 endp

                ALIGN 2
_RMDevIDToHandleListOrg proc far
		enter16
                test    byte ptr _RMFlags,01H
                je      short RMDevIDToHandleList_L2
                test    byte ptr _RMFlags,04H
                je      short RMDevIDToHandleList_L1
                lea     ax,+6H[bp]
                push    ss
                push    ax
                push    0020H
                push    cs
                call    near ptr _CallRM
                mov     sp,bp
		ret16
RMDevIDToHandleList_L1:             
		mov     ax,0014H
		ret16
                nop     
RMDevIDToHandleList_L2:             
                test    byte ptr _RMFlags,02H
                je      short RMDevIDToHandleList_L3
                sub     ax,ax
		ret16
	        nop     
RMDevIDToHandleList_L3: 
	        mov     ax,0001H
		ret16
                nop     
_RMDevIDToHandleListOrg endp

                ALIGN 2
                PUBLIC  _RMDevIDToHandleList16
_RMDevIDToHandleList16  proc far 
		enter32
		xor	eax, eax
		push	dword ptr [bp+42]
		push	dword ptr [bp+38]
		push	dword ptr [bp+34]
		push	dword ptr [bp+30]
		push	dword ptr [bp+26]
		push	dword ptr [bp+22]
		push	dword ptr [bp+18]
		push	dword ptr [bp+14]
		push	dword ptr [bp+10]
		call	_RMDevIDToHandleListOrg
		add	sp, 36
                ret32
_RMDevIDToHandleList16 endp

                ALIGN 2
_RMHandleToResourceHandleListOrg proc far
		enter16
                test    byte ptr _RMFlags,01H
                je      short RMHandleToResourceHandleList_L2
                test    byte ptr _RMFlags,04H
                je      short RMHandleToResourceHandleList_L1
                lea     ax,+6H[bp]
                push    ss
                push    ax
                push    0021H
                push    cs
                call    near ptr _CallRM
                ret16
                nop     
RMHandleToResourceHandleList_L1:
                mov     ax,0014H
                ret16
                nop     
RMHandleToResourceHandleList_L2:
                test    byte ptr _RMFlags,02H
                je      short RMHandleToResourceHandleList_L3
                sub     ax,ax
                ret16
                nop     
RMHandleToResourceHandleList_L3:
                 mov     ax,0001H
                ret16
                nop     
_RMHandleToResourceHandleListOrg endp

                ALIGN 2
                PUBLIC  _RMHandleToResourceHandleList16
_RMHandleToResourceHandleList16  proc far 
		enter32
		xor	eax, eax
		push	dword ptr [bp+14]
		push	dword ptr [bp+10]
		call	_RMHandleToResourceHandleListOrg
		add	sp, 8
                ret32
_RMHandleToResourceHandleList16 endp

                ALIGN 2
ISR00_16 proc far
                push    ebx
                mov     ebx, 0
                call    far ptr FLAT:Interrupt32
                pop     ebx
                retf
ISR00_16 endp

                ALIGN 2
ISR01_16 proc far
                push    ebx
                mov     ebx, 1
                call    far ptr FLAT:Interrupt32
                pop     ebx
                retf
ISR01_16 endp

                ALIGN 2
ISR02_16 proc far
                push    ebx
                mov     ebx, 2
                call    far ptr FLAT:Interrupt32
                pop     ebx
                retf
ISR02_16 endp

                ALIGN 2
ISR03_16 proc far
                push    ebx
                mov     ebx, 3
                call    far ptr FLAT:Interrupt32
                pop     ebx
                retf
ISR03_16 endp

                ALIGN 2
ISR04_16 proc far
                push    ebx
                mov     ebx, 4
                call    far ptr FLAT:Interrupt32
                pop     ebx
                retf
ISR04_16 endp

                ALIGN 2
ISR05_16 proc far
                push    ebx
                mov     ebx, 5
                call    far ptr FLAT:Interrupt32
                pop     ebx
                retf
ISR05_16 endp

                ALIGN 2
ISR06_16 proc far
                push    ebx
                mov     ebx, 6
                call    far ptr FLAT:Interrupt32
                pop     ebx
                retf
ISR06_16 endp

                ALIGN 2
ISR07_16 proc far
                push    ebx
                mov     ebx, 7
                call    far ptr FLAT:Interrupt32
                pop     ebx
                retf
ISR07_16 endp

;end of 16 bits code segment
__OffFinalCS16 label byte

CODE16 ends

CODE32 segment
ASSUME CS:FLAT, DS:FLAT, ES:FLAT

	public __GETDS
        public thunk1632_devhelp
	public thunk1632_devhelp_modified_ds	
        public DevHlp
        public DevHlp_ModifiedDS
	public STRATEGY_
	public IDC_
        public TIMER_
        extrn  ALSA_STRATEGY  : near
	extrn  ALSA_IDC : near
	extrn  ALSA_TIMER_ : near
        extrn  ALSA_Interrupt : near
        extrn  GetTKSSBase : near
        extrn  _rdOffset: dword

IFDEF KEE
	extrn  KernThunkStackTo16 : near
	extrn  KernThunkStackTo32 : near
ELSE
        extrn  StackAlloc : near
        extrn  StackFree  : near
ENDIF

;Called by Watcom to set the DS
        ALIGN 4
__GETDS proc near
	push	eax
        mov 	eax, DOS32FLATDS
	mov	ds, eax
	pop	eax
	ret
__GETDS endp

        ALIGN 4
DevHelpDebug  proc near
        int 3
        int 3
        ret
DevHelpDebug  endp

	ALIGN 	4

        ALIGN 4
DevHlp proc near
IFDEF FLATSTACK
        DevThunkStackTo16_Int
ENDIF

	jmp	far ptr CODE16:thunk3216_devhelp
	ALIGN 4
thunk1632_devhelp:
IFDEF FLATSTACK
        DevThunkStackTo32_Int
ENDIF
	ret
DevHlp endp

	ALIGN 	4
DevHlp_ModifiedDS proc near
IFDEF FLATSTACK
        DevThunkStackTo16_Int
ENDIF
	jmp	far ptr CODE16:thunk3216_devhelp_modified_ds
	ALIGN 4
thunk1632_devhelp_modified_ds:
IFDEF FLATSTACK
        DevThunkStackTo32_Int
ENDIF
	ret
DevHlp_ModifiedDS endp


IFNDEF KEE
;;******************************************************************************
;FixSelDPL:
;
; Set DPL of DOS32FLATDS selector to 0 or else we'll get a trap D when loading
; it into the SS register (DPL must equal CPL when loading a selector into SS)
;;******************************************************************************
        PUBLIC FixSelDPL
        ALIGN  4
FixSelDPL proc near
	cmp 	fWrongDPL, 1
	jne 	short @@fixdpl_end
	cmp 	SelRef, 0
	jne 	short @@fixdpl_endfix
	push	eax
	push	ebx
	push	edx
	sgdt	fword ptr [gdtsave]		; access the GDT ptr
	mov	ebx, dword ptr [gdtsave+2]	; get lin addr of GDT
	mov	eax, ds				; build offset into table
	and	eax, 0fffffff8h			; mask away DPL
	add	ebx, eax			; build address

	mov	eax, dword ptr [ebx+4]
	mov	edx, eax
	shr	edx, 13
	and	edx, 3
        
        ;has the OS/2 kernel finally changed the DPL to 0?
	cmp	edx, 0
	jne	@@changedpl
	mov 	fWrongDPL, 0		;don't bother anymore
	mov 	SelRef, 0
	jmp	short @@endchange
        
@@changedpl:
	mov 	oldDPL, eax
	and 	eax, NOT 6000h		;clear bits 5 & 6 in the high word (DPL)
	mov 	dword ptr [ebx+4], eax
@@endchange:
	pop	edx
	pop	ebx
	pop	eax
@@fixdpl_endfix:
	inc     SelRef
@@fixdpl_end:
        ret        
FixSelDPL endp        
;;******************************************************************************
; RestoreSelDPL:
;
;  Restore DPL of DOS32FLATDS selector or else OS/2 kernel code running in ring 3
;  will trap (during booting only; this sel has a DPL of 0 when PM starts up)
;;******************************************************************************
        PUBLIC RestoreSelDPL
        ALIGN  4
RestoreSelDPL proc near
	cmp 	fWrongDPL, 1
	jne 	short @@restdpl_end
        
	cmp 	SelRef, 1
	jne 	short @@restdpl_endrest
	push	eax
	push	ebx
	sgdt	fword ptr [gdtsave]		; access the GDT ptr
	mov	ebx, dword ptr [gdtsave+2]	; get lin addr of GDT
	mov	eax, ds				; build offset into table
	and	eax, 0fffffff8h			; mask away DPL
	add	ebx, eax			; build address

	mov	eax, oldDPL
	mov	dword ptr [ebx+4], eax
	pop	ebx
	pop	eax
@@restdpl_endrest:
	dec     SelRef
@@restdpl_end:
        ret
RestoreSelDPL endp
ENDIF

;*******************************************************************************
;Copy parameters to 16 bits stack and call 16:32 IDC handler
; 
; Paramters: IDC16_HANDLER pHandler
;            ULONG         cmd
;            ULONG         param1
;            ULONG         param2
;*******************************************************************************
        PUBLIC _CallPDD16
        ALIGN  4
_CallPDD16 proc near
        push ebp
	mov  ebp, esp
        push ebx

        lea  ebx, [ebp+8]
        DevThunkStackTo16_Int

        push dword ptr [ebx+16]    ;param2
        push dword ptr [ebx+12]    ;param1
        push dword ptr [ebx+8]     ;cmd
        call fword ptr [ebx]
        add  sp, 12
        
        DevThunkStackTo32_Int

        pop  ebx
        pop  ebp
        ret
_CallPDD16 endp

;*******************************************************************************
;Strategy entrypoint
; Parameter:
;  fs:ebx -> request packet pointer
;*******************************************************************************
        ALIGN 4
STRATEGY_ proc far	
        push	ds
	push	es
	push	fs
	push	gs

        mov 	eax, DOS32FLATDS
	mov	ds, eax
	mov	es, eax

IFDEF FLATSTACK

IFNDEF KEE
        ;done in init.cpp for the KEE version
        cmp     dword ptr [intSwitchStack], 0
        jne     stratcontinue

        ;get TKSSBase & intSwitchStack pointers
        call    GetTKSSBase
stratcontinue:
ENDIF

        DevThunkStackTo32
        cmp     eax, 0
        jne     @@stackswitchfail_strat
           
	call 	ALSA_STRATEGY

        DevThunkStackTo16

@@stackswitchfail_strat:
ELSE
        int     3
	call 	ALSA_STRATEGY
ENDIF

	pop	gs
	pop	fs
	pop	es
	pop	ds
	retf
STRATEGY_ endp

;*******************************************************************************
;IDC entrypoint
;
;in: ecx = cmd
;    edx = ULONG parameter
;return value in eax
;*******************************************************************************
        ALIGN 4
IDC_ proc far       
	push	ds
	push	es
	push	fs
	push	gs

        mov 	eax, DOS32FLATDS
	mov	ds, eax
	mov	es, eax

IFDEF FLATSTACK
        DevThunkStackTo32
        cmp     eax, 0
        jne     @@stackswitchfail_idc

	call 	ALSA_IDC

        DevThunkStackTo16

@@stackswitchfail_idc:

ELSE
        int     3
	call 	ALSA_IDC
ENDIF

	pop	gs
	pop	fs
	pop	es
	pop	ds
	retf
IDC_ endp
;*******************************************************************************
;Timer entrypoint
;
;*******************************************************************************
        ALIGN 4
TIMER_ proc far       
	push	ds
	push	es
	push	fs
	push	gs

        mov 	eax, DOS32FLATDS
	mov	ds, eax
	mov	es, eax

IFDEF FLATSTACK
        DevThunkStackTo32
        cmp     eax, 0
        jne     @@stackswitchfail_timer

	call 	ALSA_TIMER_

        DevThunkStackTo16

@@stackswitchfail_timer:

ELSE
        int     3
	call 	ALSA_TIMER_
ENDIF

	pop	gs
	pop	fs
	pop	es
	pop	ds
	retf
TIMER_ endp


;*******************************************************************************
;32 bits interrupt wrapper which calls the generic interrupt handler in irq.cpp
;On entry:
; EBX = irq nr
;*******************************************************************************
        ALIGN 4
Interrupt32 proc far
        enter   0, 0
        and     sp, 0fffch			; align stack

        pushad
	push	ds
	push	es
	push	fs
	push	gs

        mov 	eax, DOS32FLATDS
	mov	ds, eax
	mov	es, eax

        pushfd
        ; At this point a cli is redundant
        ; we enter the interrupt handler with interrupts disabled.
        ;cli

IFDEF FLATSTACK
        DevThunkStackTo32
        cmp     eax, 0
        jne     @@stackswitchfail_irq

        ;returns irq status in eax (1=handled; 0=unhandled)
	call 	ALSA_Interrupt

        DevThunkStackTo16
        
@@stackswitchfail_irq:
ELSE
        int     3
	call 	ALSA_Interrupt
ENDIF

        ;restore flags
        popfd

        cmp     eax, 1
        je      irqhandled
        stc			;tell OS/2 kernel we didn't handle this interrupt
        jmp     short endofirq

irqhandled:
        clc			;tell OS/2 kernel this interrupt was ours

endofirq:
       
	pop	gs
	pop	fs
	pop	es
	pop	ds
        popad

        leave
	retf
Interrupt32 endp

        ALIGN 4
HelpClose proc far	
        push	ds
        mov 	eax, DOS32FLATDS
	mov	ds, eax
	mov	_rdOffset, 0
	pop	ds
	retf
HelpClose endp


;*******************************************************************************
;resource manager wrappers (switch stack and call 16 bits function)
;*******************************************************************************
        public _RMAllocResource
        public _RMDeallocResource
        public _RMCreateDevice
        public _RMCreateAdapter
        public _RMCreateDriver
        public _RMDestroyDriver
        public _RMGetNodeInfo
        public _RMDevIDToHandleList
        public _RMHandleToResourceHandleList

	ALIGN 	4

        ALIGN 4
_RMAllocResource proc near
	enterKEERM
	xor	eax, eax
	push	dword ptr [edi+16]
	push	dword ptr [edi+12]
	push	dword ptr [edi+8]
	call	fword ptr RMAllocResource1632
	add	sp, 12
        retKEERM
_RMAllocResource endp

	ALIGN 	4
_RMDeallocResource proc near
	enterKEERM
	xor	eax, eax
	push	dword ptr [edi+12]
	push	dword ptr [edi+8]
	call	fword ptr RMDeallocResource1632
	add	sp, 8
        retKEERM
_RMDeallocResource endp

        ALIGN 4
_RMCreateDevice proc near
	enterKEERM
	xor	eax, eax
	push	dword ptr [edi+24]
	push	dword ptr [edi+20]
	push	dword ptr [edi+16]
	push	dword ptr [edi+12]
	push	dword ptr [edi+8]
	call	fword ptr RMCreateDevice1632
	add	sp, 20
        retKEERM
_RMCreateDevice endp

        ALIGN 4
_RMCreateAdapter proc near
	enterKEERM
	xor	eax, eax
	push	dword ptr [edi+24]
	push	dword ptr [edi+20]
	push	dword ptr [edi+16]
	push	dword ptr [edi+12]
	push	dword ptr [edi+8]
	call	fword ptr RMCreateAdapter1632
	add	sp, 20
        retKEERM
_RMCreateAdapter endp

        ALIGN 4
_RMCreateDriver proc near
	enterKEERM
	xor	eax, eax
	push	dword ptr [edi+12]
	push	dword ptr [edi+8]
	call	fword ptr RMCreateDriver1632
	add	sp, 8
        retKEERM
_RMCreateDriver endp

        ALIGN 4
_RMDestroyDriver proc near
	enterKEERM
	xor	eax, eax
	push	dword ptr [edi+8]
	call	fword ptr RMDestroyDriver1632
	add	sp, 4
        ret32
_RMDestroyDriver endp

        ALIGN 4
_RMGetNodeInfo  proc near
	enterKEERM
	xor	eax, eax
	push	dword ptr [edi+16]	
	push	dword ptr [edi+12]	
	push	dword ptr [edi+8]
	call	fword ptr RMGetNodeInfo1632
	add	sp, 12
        retKEERM
_RMGetNodeInfo endp

        ALIGN 4
_RMDevIDToHandleList  proc near
	enterKEERM
	xor	eax, eax
	push	dword ptr [edi+40]
	push	dword ptr [edi+36]
	push	dword ptr [edi+32]
	push	dword ptr [edi+28]
	push	dword ptr [edi+24]
	push	dword ptr [edi+20]
	push	dword ptr [edi+16]
	push	dword ptr [edi+12]
	push	dword ptr [edi+8]
	call	fword ptr RMDevIDToHandleList1632
	add	sp, 36
        retKEERM
_RMDevIDToHandleList endp

        ALIGN 4
_RMHandleToResourceHandleList  proc near
	enterKEERM
	xor	eax, eax
	push	dword ptr [edi+12]
	push	dword ptr [edi+8]
	call	fword ptr RMHandleToResourceHandleList1632
	add	sp, 8
        retKEERM
_RMHandleToResourceHandleList endp


CODE32 ends

DATA32 	segment
    public  __OffsetFinalCS16
    public  __OffsetFinalDS16
    public  PDDName
    public  _MSG_TABLE32
    public  RMAllocResource1632
    public  RMDeallocResource1632
    public  RMCreateDevice1632
    public  RMCreateAdapter1632
    public  RMCreateDriver1632
    public  RMDestroyDriver1632
    public  RMGetNodeInfo1632
    public  RMDevIDToHandleList1632
    public  RMHandleToResourceHandleList1632
    public  _TimerHandler16
    public  _ISR00
    public  _ISR01
    public  _ISR02
    public  _ISR03
    public  _ISR04
    public  _ISR05
    public  _ISR06
    public  _ISR07

IFDEF FLATSTACK
    extrn   intSwitchStack : dword
ENDIF

IFDEF KEE
    public  stackbase
    public  stacksel

    stackbase dd 0
    stacksel  dd 0    
ELSE

    public  gdtsave
    public  fWrongDPL
    public  oldDPL
    public  SelRef

    tempeax          dd 0
    tempedx          dd 0
    tempesi          dd 0
    cpuflags         dd 0

    gdtsave	     dq 0
    fWrongDPL	     dd 1	;DOS32FLATDS has the wrong DPL for SS
    SelRef	     dd 0
    oldDPL	     dd 0

    fInitStack       dd 0
ENDIF

    __OffsetFinalCS16 dw OFFSET CODE16:__OffFinalCS16
    __OffsetFinalDS16 dw OFFSET DATA16:__OffFinalDS16

    _MSG_TABLE32     dw OFFSET  DATA16:_MSG_TABLE16
    		     dw SEG     DATA16:_MSG_TABLE16
          
;16:16 address of driver name   
    PDDName          dw OFFSET  DATA16:pddname16
		     dw SEG     DATA16:pddname16

;16:32 addresses of resource manager functions in 16 bits code segment
    RMAllocResource1632  dd OFFSET CODE16:_RMAllocResource16
		         dw SEG CODE16:_RMAllocResource16
		         dw 0
    RMDeallocResource1632 dd OFFSET CODE16:_RMDeallocResource16
 		          dw SEG CODE16:_RMDeallocResource16
 		          dw 0
    RMCreateDevice1632   dd OFFSET CODE16:_RMCreateDevice16
		         dw SEG CODE16:_RMCreateDevice16
		         dw 0
    RMCreateAdapter1632  dd OFFSET CODE16:_RMCreateAdapter16
		         dw SEG CODE16:_RMCreateAdapter16
		         dw 0
    RMCreateDriver1632   dd OFFSET CODE16:_RMCreateDriver16
		         dw SEG CODE16:_RMCreateDriver16
		         dw 0
    RMDestroyDriver1632  dd OFFSET CODE16:_RMDestroyDriver16
  		         dw SEG CODE16:_RMDestroyDriver16
		         dw 0
    RMGetNodeInfo1632    dd OFFSET CODE16:_RMGetNodeInfo16
		         dw SEG CODE16:_RMGetNodeInfo16
		         dw 0
 RMDevIDToHandleList1632 dd OFFSET CODE16:_RMDevIDToHandleList16
		         dw SEG CODE16:_RMDevIDToHandleList16
		         dw 0

 RMHandleToResourceHandleList1632 dd OFFSET CODE16:_RMHandleToResourceHandleList16
        	    	          dw SEG CODE16:_RMHandleToResourceHandleList16
   	        	          dw 0

;16:16 address of uniaud_stub_timer
    _TimerHandler16   	 dd OFFSET CODE16:uniaud_stub_timer
                     	 dw OFFSET CODE16:uniaud_stub_timer

;16:16 addresses of interrupt dispatchers
    _ISR00               dw OFFSET CODE16:ISR00_16
			 dw SEG CODE16:ISR00_16
    _ISR01               dw OFFSET CODE16:ISR01_16
			 dw SEG CODE16:ISR01_16
    _ISR02               dw OFFSET CODE16:ISR02_16
			 dw SEG CODE16:ISR02_16
    _ISR03               dw OFFSET CODE16:ISR03_16
                         dw SEG CODE16:ISR03_16
    _ISR04               dw OFFSET CODE16:ISR04_16
                         dw SEG CODE16:ISR04_16
    _ISR05               dw OFFSET CODE16:ISR05_16
                         dw SEG CODE16:ISR05_16
    _ISR06               dw OFFSET CODE16:ISR06_16
			 dw SEG CODE16:ISR06_16
    _ISR07               dw OFFSET CODE16:ISR07_16
                         dw SEG CODE16:ISR07_16
DATA32 ends

end

