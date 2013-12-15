	include vram.i

_text	segment	para public use32 'code'
	assume cs:CGROUP,ds:DGROUP


	public _vram_get_hseg
_vram_get_hseg proc near
vghp struc
	vgh_ebp	dd ?
	vgh_ret dd ?
	vgh_v	dd ?
	vgh_pixbuf	dd ?
	vgh_x	dd ?
	vgh_y	dd ?
	vgh_width dd ?
vghp ends
	push ebp
	mov ebp,esp
	push edi
	push esi
	push ds



	;get ytable pointer for this line
	mov edi,[ebp].vgh_v
	mov edi,[edi].vs_ytable
	mov eax,[ebp].vgh_y
	shl eax,3
	add edi,eax


	mov ax,[edi].yta_split	;get split...
	test ax,ax
	;jnz vgh_done	;DEBUG
	jnz  vgh_split

vgh_left:		;non-split or hseg to left of split case
	mov esi,[edi].yta_address
	add esi,[ebp].vgh_x
	mov cx,[edi].yta_bank	;get bank
	setbank
	mov ecx,[ebp].vgh_width
	mov edx,ecx
	mov edi,[ebp].vgh_pixbuf
	shr ecx,1
	mov ax,PHAR_REAL_SEG
	mov ds,ax
	rep movsw
	test edx,1
	jz vgh_done
	movsb

vgh_done:		;restore regs and return
	pop ds
	pop esi
	pop edi
	pop ebp
	ret

vgh_right:		;hseg to the right of split case
	mov esi,[edi].yta_address
	add esi,[ebp].vgh_x
	sub esi,010000h			;wrap back to A0000 screen
	mov cx,[edi].yta_bank	
	inc cx					;we've gone to next bank
	setbank
	mov ecx,[ebp].vgh_width
	mov edi,[ebp].vgh_pixbuf
	mov ax,PHAR_REAL_SEG
	mov ds,ax
	rep movsb
	jmp vgh_done

vgh_split:	;here the line is split, but maybe this part of it isn't
	movzx edx,ax	;sign extend split_at
	mov eax,[ebp].vgh_x
	cmp eax,edx		;if (x >= split_at) goto vgh_right
	jae vgh_right	
	add eax,[ebp].vgh_width
	cmp eax,edx		;if (x+width < split_at) goto vgh_left
	jb vgh_left

;if made it here the segment is split.
;do 1st half of segment
	push edi
	mov cx,[edi].yta_bank
	setbank
	mov esi,[edi].yta_address
	mov edx,[ebp].vgh_x
	add esi,edx
	movzx ecx,[edi].yta_split
	sub ecx,edx
	mov edi,[ebp].vgh_pixbuf
	push ds
	mov ax,PHAR_REAL_SEG
	mov ds,ax
	rep movsb
	pop ds
;do 2nd half of segment
	pop esi
	mov cx,[esi].yta_bank
	inc cx					;we've gone to next bank
	setbank
	movzx edx,[esi].yta_split
	mov esi,0a0000h
	mov ecx,[ebp].vgh_width
	add ecx,[ebp].vgh_x
	sub ecx,edx
	mov ax,PHAR_REAL_SEG
	mov ds,ax
	rep movsb
	jmp vgh_done
_vram_get_hseg endp
_text	ends
	end
