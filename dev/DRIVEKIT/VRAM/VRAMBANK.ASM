	include vram.i

_DATA           SEGMENT PUBLIC WORD USE32 'DATA'
	public vrambank
vrambank label byte
				db 0
_DATA           ENDS

_text	segment	para public use32 'code'
	assume cs:CGROUP,ds:DGROUP

	;switch banks
	;called from assembler
	;input:		cl   bank
	;trashes:	ax,dx,cl
	;
	public _setbank
_setbank proc near
	mov vrambank,cl
	mov dx,SC_INDEX ;bank bit 0
	mov ah,cl
	and ah,1
	mov al,ER_PAGE_SEL
	out dx,ax
	mov ah,cl		;bank bit 1
	and ah,2
	shl ah,4
	mov dx,MISC_INPUT
	in al,dx
	and al, not 20h
	mov dx,MISC_OUTPUT
	or al,ah
	out dx,al
;set bank bit 3
	mov dx,SC_INDEX
	mov al,ER_BANK_SEL
	out dx,al
	inc dx
	in al,dx
	and al,0f0h	;clear bank select bits
; duplicate bit 2 into bit 0 (set read and write bank equal)
	;mov ah,cl
	;shr ah,2
	;or ah,cl
	;or al,ah
	shr cl,2
	add cl,7
	not cl
	and cl,5
	or al,cl
	out dx,al
	ret
_setbank endp

_text ends
	end
