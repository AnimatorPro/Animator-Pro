;**********************************************************************
;
; EVGAXHLI.ASM -- Draw an XOR'd Horizontal Line, one byte at a time.
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


	public _evga_x_hline
_evga_x_hline proc near
vxhp struc
	vxh_ebp	dd ?
	vxh_ret dd ?
	vxh_v	dd ?
	vxh_color	dd ?
	vxh_x	dd ?
	vxh_y	dd ?
	vxh_width dd ?
vxhp ends
	push ebp
	mov ebp,esp
	push esi
	push edi
	push es

	mov ax,PHAR_REAL_SEG
	mov es,ax

	;get ytable pointer for this line
	mov esi,[ebp].vxh_v
	mov esi,[esi].vs_ytable
	mov eax,[ebp].vxh_y
	shl eax,3
	add esi,eax

;
; COMMENTED OUT - SEE NOTE IN HEADER
;
;	mov ax,[esi].yta_split	;get split...
;	test ax,ax
;	jnz  splitit

nosplit_left:
	mov edi,[esi].yta_address
	add edi,[ebp].vxh_x
	mov cx,[esi].yta_bank	;get bank
	setbank
	mov eax,[ebp].vxh_color
	mov ecx,[ebp].vxh_width
nsl1:
	xor es:[edi],al
	inc edi
	loop nsl1

dhli_done:
	pop es
	pop edi
	pop esi
	pop ebp
	ret

;
; COMMENTED OUT - SEE NOTE IN HEADER
;
;nosplit_right:
;	mov edi,[esi].yta_address
;	add edi,[ebp].vxh_x
;	sub edi,010000h
;	mov cx,[esi].yta_bank	;get bank
;	inc cx			;but we're the next one...
;	setbank
;	mov eax,[ebp].vxh_color
;	mov ecx,[ebp].vxh_width
;nsrl1:
;	xor es:[edi],al
;	inc edi
;	loop nsrl1
;	jmp dhli_done
;
;splitit:	;here the line is split, but maybe this part of it isn't
;	movzx edx,ax	;sign extend split_at
;	mov eax,[ebp].vxh_x
;	cmp eax,edx		;if (x >= split_at) goto nosplit_right
;	jae nosplit_right	
;	add eax,[ebp].vxh_width
;	cmp eax,edx		;if (x+width < split_at) goto nosplit_left
;	jb nosplit_left
;
;;if made it here the segment is split.
;
;
;	;debugging...
;	;mov eax,120	;DEBUG
;	;mov [ebp].vxh_color,eax	;DEBUG
;
;	;do 1st half of segment
;	mov cx,[esi].yta_bank	;get bank
;	setbank
;	mov edi,[esi].yta_address
;	mov edx,[ebp].vxh_x
;	add edi,edx
;	movzx ecx,[esi].yta_split		;get split_at
;	sub ecx,edx
;	mov eax,[ebp].vxh_color
;sl1:
;	xor es:[edi],al
;	inc edi
;	loop sl1
;
;	;do 2nd half of segment
;
;
;	;mov eax,30	;DEBUG
;	;mov [ebp].vxh_color,eax	;DEBUG
;
;	mov cx,[esi].yta_bank	;get bank
;	inc cx
;	setbank
;	mov edi,[esi].yta_address
;	movzx edx,[esi].yta_split
;	add edi,edx
;	sub edi,010000h
;	mov ecx,[ebp].vxh_width
;	add ecx,[ebp].vxh_x
;	sub ecx,edx
;	mov eax,[ebp].vxh_color
;sl2:
;	xor es:[edi],al
;	inc edi
;	loop sl2
;
;
;	jmp dhli_done

_evga_x_hline endp
_text	ends
	end
