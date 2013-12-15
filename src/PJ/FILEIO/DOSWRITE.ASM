CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

;	pj_dwrite(int file_handle, char *buffer, int size)
;	returns bytes actually written
	public pj_dwrite
pj_dwrite proc near
pjwrite struc
	pjw_savebp dd ?
	pjw_ret dd ?
	pjw_handle dd ?
	pjw_buffer dd ?
	pjw_size dd ?
pjwrite ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	mov eax,[ebp].pjw_size
	cmp eax,00h 	; size of 0 is always successful
	jz #retok
	mov ebx,[ebp].pjw_handle
	mov ecx,eax
	mov edx,[ebp].pjw_buffer
	mov ah,40h
	int 21h
	jnc #retok
	mov eax,-1		;error other than no space left on disk
#retok:
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
pj_dwrite endp

code	ends
	end
