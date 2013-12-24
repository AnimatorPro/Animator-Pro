;**********************************************************************
;
; EVGAPUTH.ASM -- Copy a horizontal line to display, 1 byte at a time.
;                 Original VRAM code required a check to see if a bank
;                 split in the middle of a line. Using the 512x480 mode
;                 on the EVGA, that's not a problem. The Split code has
;                 been commented out, instead of deleted, in case someone
;                 wants to add EVGA support for modes that might have a bank
;                 switch occur mid-line.
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


	public _evga_put_hseg
_evga_put_hseg proc near
vphp struc
	vph_ebp	dd ?
	vph_ret dd ?
	vph_v	dd ?
	vph_pixbuf	dd ?
	vph_x	dd ?
	vph_y	dd ?
	vph_width dd ?
vphp ends
	push ebp
	mov ebp,esp
	push esi
	push edi
	push es

	mov ax,PHAR_REAL_SEG
	mov es,ax

	;get ytable pointer for this line
	mov esi,[ebp].vph_v
	mov esi,[esi].vs_ytable
	mov eax,[ebp].vph_y
	shl eax,3
	add esi,eax

;
; COMMENTED OUT - SEE NOTE IN HEADER
;
;	mov ax,[esi].yta_split	;get split...
;	test ax,ax
;	jnz  vph_split

vph_left:		;non-split or hseg to left of split case
	mov edi,[esi].yta_address
	add edi,[ebp].vph_x
	mov cx,[esi].yta_bank	;get bank
	setbank
	mov ecx,[ebp].vph_width
	mov edx,ecx
	mov esi,[ebp].vph_pixbuf
	shr ecx,1
	rep movsw
	test edx,1
	jz vph_done
	movsb

vph_done:		;restore regs and return
	pop es
	pop edi
	pop esi
	pop ebp
	ret

;
; COMMENTED OUT - SEE NOTE IN HEADER
;
;vph_right:		;hseg to the right of split case
;	mov edi,[esi].yta_address
;	add edi,[ebp].vph_x
;	sub edi,010000h			;wrap back to A0000 screen
;	mov cx,[esi].yta_bank	
;	inc cx					;we've gone to next bank
;	setbank
;	mov ecx,[ebp].vph_width
;	mov esi,[ebp].vph_pixbuf
;	rep movsb
;	jmp vph_done
;
;vph_split:	;here the line is split, but maybe this part of it isn't
;	movzx edx,ax	;sign extend split_at
;	mov eax,[ebp].vph_x
;	cmp eax,edx		;if (x >= split_at) goto vph_right
;	jae vph_right	
;	add eax,[ebp].vph_width
;	cmp eax,edx		;if (x+width < split_at) goto vph_left
;	jb vph_left
;
;;if made it here the segment is split.
;;do 1st half of segment
;	push esi
;	mov cx,[esi].yta_bank
;	setbank
;	mov edi,[esi].yta_address
;	mov edx,[ebp].vph_x
;	add edi,edx
;	movzx ecx,[esi].yta_split
;	sub ecx,edx
;	mov esi,[ebp].vph_pixbuf
;	rep movsb
;;do 2nd half of segment
;	pop edi
;	mov cx,[edi].yta_bank
;	inc cx					;we've gone to next bank
;	setbank
;	movzx edx,[edi].yta_split
;	mov edi,0a0000h
;	mov ecx,[ebp].vph_width
;	add ecx,[ebp].vph_x
;	sub ecx,edx
;	rep movsb
;	jmp vph_done
_evga_put_hseg endp
_text	ends
	end
