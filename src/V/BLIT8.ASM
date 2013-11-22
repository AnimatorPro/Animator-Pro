;blit8.asm - Your basic blit (copy only, no xor or anything fancy) on
;a byte-a-pixel source and dest.  Not set up to deal with overlapping
;source and destination.  Clips to fit destination if necessary. (see
;clipit.asm)
;:ts=10

	TITLE   blit8
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



	PUBLIC	_blit8
;blit8(wid, height, sx1,sy1,spt,snext_line, dx1,dy1,dpt,dnext_line)
wid	equ 4+2
height	equ 6+2
sx1	equ 8+2
sy1	equ 10+2
spt_o	equ 12+2
spt_s	equ 14+2
snext	equ 16+2
dx1	equ 18+2
dy1	equ 20+2
dpt_o	equ 22+2
dpt_s	equ 24+2
dnext	equ 26+2
_blit8	PROC far
	push bp
	mov bp,sp
	push es
	push ds
	push si
	push di
	push bx
	push cx

	call FAR PTR clipblit
	jc za1

	;get starting source address in ds:si
	lds	si,[bp+spt_o]	
	mov	ax,[bp+sy1]
	mov	bx,[bp+snext]
	mul	bx
	add	si,ax
	add	si,[bp+sx1]

	;get starting destination address in es:di
	les	di,[bp+dpt_o]	
	mov	ax,[bp+dy1]
	mov	bx,[bp+dnext]
	mul	bx
	add	di,ax
	add	di,[bp+dx1]

	mov	bx,si
	mov	dx,di
	cld
	jmp	zcb

clipped:	jmp	za1

cb:	mov	cx,[bp+wid]	;width into count register
	mov	si,bx		;start of line into source index
	mov	di,dx		;start of line into dest index
	rep 	movsb
	add	bx,[bp+snext]
	add	dx,[bp+dnext]
zcb: 	dec 	WORD PTR[bp+height]
	js za1
	jmp cb


za1:
	pop cx
	pop bx
	pop di
	pop si
	pop ds
	pop es
	pop bp
	ret

_blit8	ENDP


	extrn	clipblit:far

_TEXT	ENDS
END
