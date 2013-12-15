CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

;	pj_dread(int file_handle, char *buffer, int size)
;	returns bytes actually read
	public pj_dread
pj_dread proc near
pjread struc
	pjr_savebp dd ?
	pjr_ret dd ?
	pjr_handle dd ?
	pjr_buffer dd ?
	pjr_size dd ?
pjread ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	mov ebx,[ebp].pjr_handle
	mov ecx,[ebp].pjr_size
	mov edx,[ebp].pjr_buffer
	mov ah,3fh
	int 21h
	jnc #retok
#retok:
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
pj_dread endp


code	ends
	end
