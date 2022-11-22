	include vram.i


_text	segment	para public use32 'code'
	assume cs:CGROUP,ds:DGROUP


	public _vram_d_hline
_vram_d_hline proc near
vdhp struc
	vdh_ebp	dd ?
	vdh_ret dd ?
	vdh_v	dd ?
	vdh_color	dd ?
	vdh_x	dd ?
	vdh_y	dd ?
	vdh_width dd ?
vdhp ends
	push ebp
	mov ebp,esp
	push esi
	push edi
	push es

	mov ax,PHAR_REAL_SEG
	mov es,ax

	;get ytable pointer for this line
	mov esi,[ebp].vdh_v
	mov esi,[esi].vs_ytable
	mov eax,[ebp].vdh_y
	shl eax,3
	add esi,eax

	mov ax,[esi].yta_split	;get split...
	test ax,ax
	jnz  splitit

nosplit_left:
	mov edi,[esi].yta_address
	add edi,[ebp].vdh_x
	mov cx,[esi].yta_bank	;get bank
	setbank
	mov eax,[ebp].vdh_color
	mov ecx,[ebp].vdh_width
	rep stosb

dhli_done:
	pop es
	pop edi
	pop esi
	pop ebp
	ret

nosplit_right:
	mov edi,[esi].yta_address
	add edi,[ebp].vdh_x
	sub edi,010000h
	mov cx,[esi].yta_bank	;get bank
	inc cx			;but we're the next one...
	setbank
	mov eax,[ebp].vdh_color
	mov ecx,[ebp].vdh_width
	rep stosb
	jmp dhli_done

splitit:	;here the line is split, but maybe this part of it isn't
	movzx edx,ax	;sign extend split_at
	mov eax,[ebp].vdh_x
	cmp eax,edx		;if (x >= split_at) goto nosplit_right
	jae nosplit_right	
	add eax,[ebp].vdh_width
	cmp eax,edx		;if (x+width < split_at) goto nosplit_left
	jb nosplit_left

;if made it here the segment is split.


	;debugging...
	;mov eax,120	;DEBUG
	;mov [ebp].vdh_color,eax	;DEBUG

	;do 1st half of segment
	mov cx,[esi].yta_bank	;get bank
	setbank
	mov edi,[esi].yta_address
	mov edx,[ebp].vdh_x
	add edi,edx
	movzx ecx,[esi].yta_split		;get split_at
	sub ecx,edx
	mov eax,[ebp].vdh_color
	rep stosb
	;do 2nd half of segment


	;mov eax,30	;DEBUG
	;mov [ebp].vdh_color,eax	;DEBUG

	mov cx,[esi].yta_bank	;get bank
	inc cx
	setbank
	mov edi,[esi].yta_address
	movzx edx,[esi].yta_split
	add edi,edx
	sub edi,010000h
	mov ecx,[ebp].vdh_width
	add ecx,[ebp].vdh_x
	sub ecx,edx
	mov eax,[ebp].vdh_color
	rep stosb



	jmp dhli_done

_vram_d_hline endp
_text	ends
	end
