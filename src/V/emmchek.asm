; emmchek.asm - See if we've got EMM.  If so we'll put the undo buffer there.
;    Tom Hudson gave me this routine.
;
; Check to see if EMM is resident
; emm_present()
; returns 0 (no EMM) or 1 (EMM present)
;


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

_TEXT	SEGMENT

	PUBLIC	_emm_present
_emm_present	PROC	far
	push	bp
	push	si
	push	di
	push	cx
	push	dx
	push	ds

; Try to open EMM device

	mov	ah,3dh
	mov	al,0
	mov	dx,seg emm_name
	mov	ds,dx
	mov	dx,offset emm_name
	int	21h
	jc	emm_absent

	mov	bx,ax
	mov	ah,44h
	mov	al,7
	mov	cx,0
	mov	dx,offset emm_buff
	int	21h
	push	ax
	mov	ah,3eh
	int	21h
	jc	emm_absent	;close failure
	pop	ax
	or	al,al
	jz	emm_absent

emm_present:
	mov	ax,1
	jmp	chek_done

emm_absent:
	mov	ax,0

chek_done:
	pop	ds
	pop	dx
	pop	cx
	pop	di
	pop	si
	pop	bp
	ret

_emm_present	ENDP

_TEXT	ENDS

_DATA	SEGMENT

emm_name	db	'EMMXXXX0',0
emm_buff	dw	0

_DATA	ENDS

	END

