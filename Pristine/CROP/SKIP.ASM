;skip.asm - low level stuff to compress pictures into FLI's.

;:ts=10

	TITLE   skip
_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST,	_BSS,	_DATA
	ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP
_TEXT      SEGMENT

	PUBLIC _tnskip
;tnskip(s1,s2,bcount,mustmatch)
s1	equ	[bp+4+2]
s1s	equ	[bp+6+2]
s2	equ	[bp+8+2]
s2s	equ	[bp+10+2]
bcount	equ	[bp+12+2]
mmatch	equ	[bp+14+2]
_tnskip	proc far
difcount	equ	[bp-2]
	push bp
	mov bp,sp
	sub sp,4	;space for locals
	push bx
	push si
	push di
	push ds

	mov word ptr difcount,0	;zero out return value
	lds si,s1
	les di,s2
	mov bx,bcount
	mov dx,mmatch

tnsloop:
	;calculate number of pixels different in s1 and s2 into ax
	mov cx,bx
	inc cx
	repne cmpsb
	mov ax,bx
	sub ax,cx
	dec si	;move source pointers just past this different run
	dec di
	sub bx,ax
	add difcount,ax	;and different count to return value

	cmp bx,dx		;see if near the end...
	js endcheck

	;see if enough in a row match to break out of this
	mov cx,dx
	repe cmpsb
	jz ztnskip	;if all of them match between s1 and s2 go home
	inc cx
	mov ax,dx		;calc ones that do match into ax
	sub ax,cx
	add difcount,ax	;add it to difcount return value
	sub bx,ax		;sub it from pixels left to examine
	dec si		;update s1,s2 pointers
	dec di
	jmp tnsloop
endcheck:
	;check last couple of pixels
	mov cx,bx
	inc cx
	repe cmpsb
	jcxz ztnskip	;if all of them match between s1 and s2 go home
	add difcount,bx	;otherwise assume no skip this time around

ztnskip:
	mov ax,difcount
	pop ds
	pop di
	pop si
	pop bx
	mov sp,bp
	pop bp
	ret
_tnskip	endp


	PUBLIC	_tnsame
;tnsame(s2x,wcount,mustmatch)
s2x	equ [bp+4+2]
wcount	equ word ptr[bp+8+2]
mumatch	equ [bp+10+2]
_tnsame	PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	push bx

				
	les	di,s2x		;get starting address in es:di
	mov	dx,wcount		;dx is 'dif_count' return value
	mov	bx,dx		;bx is # of pixels left
	mov	si,0		;si is # of pixels examined
alp:
				;break out of loop if less than 4 pixels
				;left to examine
	cmp	bx,mumatch
	js	zalp

	;same_count = bsame(s2x,wcount)
	mov cx,bx
	mov al,es:[di]
	rep scasb
	inc cx
	sub di,2
	mov ax,bx
	sub ax,cx

	cmp ax,mumatch			;if mustmatch or more
	jns gotsame		;go truncate dif_count
	add	si,ax
	add	di,ax
	sub	bx,ax
	jmp	alp
gotsame:
	mov	dx,si		
zalp:
	mov ax,dx		;return dif_count
	pop bx
	pop di
	pop si
	pop ds
	pop bp
	ret

_tnsame	ENDP


_TEXT	ENDS
END
