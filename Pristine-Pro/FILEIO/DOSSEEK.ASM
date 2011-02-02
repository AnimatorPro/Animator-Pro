CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i


;	pj_dseek(int file, long offset, int mode);
	public pj_dseek
pj_dseek proc near
jsk	struc
	jsk_ebp dd ?
	jsk_ret dd ?
	jsk_file dd ?
	jsk_loff dw ?
	jsk_hoff dw ?
	jsk_mode dd ?
jsk ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	mov eax,[ebp].jsk_mode
	mov ebx,[ebp].jsk_file
	mov cx,[ebp].jsk_hoff
	mov dx,[ebp].jsk_loff
	mov ah,42h
	int 21h
	jc	#badret
	shl	edx,16	;combine hi result in dx with lo result in ax into eax.
	mov dx,ax
	mov eax,edx
	jmp #retit
#badret:
	mov eax,err_seek
#retit:
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
pj_dseek endp

code	ends
	end
