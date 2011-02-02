;**********************************************************************
;
; EVGABANK.ASM --  Bank switch code for Everex EVGA (and Trident chips)
;
; Written by Panacea Inc.
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

_DATA      SEGMENT   PUBLIC WORD USE32 'DATA'
	public    tridbank
tridbank   label     byte
	db        0
_DATA      ENDS

_text	segment	para public use32 'code'
	assume cs:CGROUP,ds:DGROUP

	;switch banks
	;called from assembler
	;input:  	cl   bank
	;trashes:	ax,dx,cl
	;
	public _setbank
_setbank proc near
	mov       tridbank,cl
           Mov       DX, 03C4H          ; This is almost too easy. Handles
           Mov       AL, 0EH            ; 8800, 8900, and ZyMOS Poach 51
           Out       DX, AL
           Inc       DX
           In        AL, DX
           And       AL, 0F0H           ; Mask out bank bits
           Or        AL, CL             ; Or in new bank bits
           Xor       AL, 2
           Out       DX, AL
	ret
_setbank endp

_text      ends
	end
