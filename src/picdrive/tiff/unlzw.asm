; Set your editor tabsize to 8 for this file!
;****************************************************************************
; UNLZW.ASM    - Decompress TIFF type-5 data (LZW encoded).
;		 (386 protected mode version).
;****************************************************************************

_DATA	segment word public use32 'data'

msktbl	     db 0ffh,07fh,03fh,01fh,00fh,007h,003h,001h
	     db 0ffh,07fh,03fh,01fh,00fh,007h,003h,001h
cnttbl	     dd 8,7,6,5,4,3,2,1
	     dd 8,7,6,5,4,3,2,1
bitspercode  dd 09,10,11,11,12,12,12,12
bitsleft     dd ?
curmask      db ?

LADDERSZ  equ	8*4096 ; size of a ladder table entry * number of table entries

_DATA	ends

_TEXT	segment word public use32 'code'
	assume	cs:_TEXT,DS:_DATA

;-----------------------------------------------------------------------------
; void unlzw_init(Ladder_table *plad)
;   Init the ladder data table.
;-----------------------------------------------------------------------------

	public	unlzw_init

unlzw_init proc near
	mov	edx,[esp+4]		; load pointer to ladder table. make
	lea	eax,[edx+LADDERSZ]	; ptr to asciitab at end of ladder tbl.
	xor	ecx,ecx 		; prime ascii value/counter.
initloop:
	mov	byte  ptr[eax],cl	; store ascii table value, store pointer
	mov	dword ptr[edx],eax	; to it in ladder table, store size
	mov	dword ptr[edx+4],0	; (zero) in ladder table.
	add	edx,8			; point to next ladder table slot.
	inc	eax			; point to next ascii table slot.
	inc	ecx			; increment ascii/counter value.
	cmp	ecx,256 		; are we done yet?
	jne	short initloop		; nope, do it some more.
	ret
unlzw_init endp

;-----------------------------------------------------------------------------
; Errcode unlzw(char *inbuf, char *outbuf, Ladder_table *plad, int buflen)
;
;  This routine decompresses an entire strip.
;  The Ladder_table structure is defined as:
;	typedef struct {
;	   char *pstring;
;	   int	count;
;	   } Ladder_table;
;  the caller need not concern itself with the contents of this table, but it
;  must allocate us an array containing 4096 Ladder_table structure entries.
;-----------------------------------------------------------------------------


; esi - Pointer to next input bit (expressed as bit offset, not byte offset)
; edi - Pointer to next output byte.
; ebp - Pointer to ladder table.
; ebx - Index of next entry in ladder table, also determines input code bitsize.
;

	public	unlzw

unlzw	proc	near

sourcep equ	[esp+24]
destp	equ	[esp+28]
tablep	equ	[esp+32]
buflen	equ	[esp+36]

CLRCODE equ	256
EOICODE equ	257
FSTCODE equ	258

	push	ebp
	push	ebx
	push	esi
	push	edi
	sub	esp,4

	mov	esi,sourcep
	mov	edi,destp
	mov	ebp,tablep
	mov	curmask,0ffh
	mov	bitsleft,8
clrtable:
	mov	ebx,FSTCODE
getcode:
	mov	ecx,ebx
	shr	ecx,9
	mov	ecx,[ecx*4+bitspercode]
	movzx	eax,byte ptr [esi]	;6;we may need up to 3 bytes to extract
	mov	dh,[esi+1]		;4;the next code, so get all 3 into the
	mov	dl,[esi+2]		;4;right registers to be masked/shifted.
	and	al,curmask		;6;mask off the unneeded high order bits.
	sub	ecx,bitsleft		;6;adjust shiftcount for the number of
	shld	ax,dx,cl		;3;bits already in al, then shift in
	mov	edx,ecx 		;2;the right number of low order bits.
	shr	ecx,3			;3;adjust the input pointer based on
	lea	esi,[esi+ecx+1] 	;3;whether we completely consumed one
	mov	cl,[edx+msktbl] 	;
	mov	curmask,cl		;
	mov	ecx,[edx*4+cnttbl]	;
	mov	bitsleft,ecx		;

	cmp	eax,EOICODE
	je	short alldone
	cmp	eax,CLRCODE
	je	short clrtable

	mov	edx,[ebp+eax*8]
	test	edx,edx
	jz	short error_null
	mov	ecx,[ebp+eax*8+4]
	inc	ecx
	sub	buflen,ecx
	js	short error_ovfl
	mov	[ebp+ebx*8],edi
	mov	[ebp+ebx*8+4],ecx
	inc	ebx
	xchg	esi,edx
	rep movsb
	mov	esi,edx
	jmp	getcode

error_ovfl:
error_null:
	mov	eax,-3			; return Err_bad_input
	jmp	short exit
alldone:
	xor	eax,eax
exit:
	add	esp,4
	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret

unlzw	endp

_TEXT	ends
	end
