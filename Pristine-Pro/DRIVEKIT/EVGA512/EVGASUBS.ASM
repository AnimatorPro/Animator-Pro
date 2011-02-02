;**********************************************************************
;
; EVGASUBS.ASM -- Check for EVGA board and initialize.
;
;
; Modified by Panacea Inc.
;
; Panacea Inc.
; 50 Nashua Road, Suite 305
; Londonderry, New Hampshire, 03053-3444
; (603) 437-5022
;
;
;Revision History:
;
;When     Who   What
;======== ===   =======================================================
;09/13/90 JBR   Start of development.
;
;**********************************************************************

	include evga.i


_text	segment	para public use32 'code'
	assume cs:CGROUP,ds:DGROUP

	public isit_evga
isit_evga proc near
	mov eax,1                   ; No way to check for it, assume yes.
	ret
isit_evga endp

	public get_vmode	;get current vga mode
get_vmode proc near
	mov AH,0Bh
	int 10h
	and eax,0ffh
	ret
get_vmode endp

	public set_vmode	;set former vga mode
set_vmode proc near
	push ebp
	mov ebp,esp
;	mov eax,8[ebp]
;	mov AH,00h                  ; Set normal VGA mode
           mov       AX, 3
	int 10h
           pop       ebp
	ret
set_vmode endp

	public set_xmode	;set extended vga mode
set_xmode proc near
	push ebp
	mov ebp,esp
	push ebx
	mov ebx,8[ebp]
	mov ax,70h
	int 10h
	pop ebx
	pop ebp
	ret
set_xmode endp

EXTREG equ 3c4h

	public enable_ext		;enable evga specific extension regs.
enable_ext proc near
;
; TRIDENT boards sometimes need to be put into 64K mode (The Everex already
; is). Some Trident customers are reporting failure at 1024x768. This should
; fix that problem, and allow the driver to support the 8900 as well as the
; 8800.
;
           push      EDX
           push      EBX
           push      EAX

                                            ; IF it's a TRIDENT 8800/8900
           Mov        DX, 03C4H             ;    Set 64K banks (A0000..AFFFF)
           Mov        AL, 0BH               ;    Index register = 0B
           Out        DX, AL                ;
           Inc        DX                    ;
           In         AL, DX                ;    Change 3C5.0E's definition
           Mov        DX, 03CEH             ;
           Mov        AX, 0506H             ;    Set 64K bank mode.
           Out        DX, AX                ; END IF

           Mov        DX, 03C4H             ; FOR EVEREX - Fix SR2
           Mov        AX, 0F02H             ;
           Out        DX, AX                ;
;
; Now we need to make sure that our palette is set correctly. This means
; that the first-stage EGA to VGA LUT must be set properly for all other
; VGA palette operations to work properly.
;
           mov       DX, 3DAH
           in        AL, DX            ; Clear flip-flop register state.

           mov       ECX, 10H          ; Number of EGA entries.
           xor       AL, AL
           mov       DL, 0C0H          ; Point to the EGA regs.
EGAPalLoop:
           out       DX, AL            ; Set the index.
           jmp       $+2               ; Wait for it to be ready again
           out       DX, AL            ; Set the data
           inc       AL                ; We want a one-to-one ramp.
           loop      EGAPalLoop

           mov       AL, 20H
           out       DX, AL            ; We're done - tell VGA.

           pop       EAX
           pop       EBX
           pop       EDX
	ret
enable_ext endp


	public disable_ext		;disable evga extension regs
disable_ext proc near
	ret
disable_ext endp

	;busy-wait for vblank (this works with any VGA)
	public evga_wait_vblank
evga_wait_vblank proc near
	mov	dx,3dah	;video status port
wvb:
	in	al,dx
	test al,8
	jz wvb
	xor eax,eax
	ret
evga_wait_vblank endp


_text	ends
	end
