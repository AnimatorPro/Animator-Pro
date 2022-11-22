CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	public dos_mem_free
dos_mem_free proc near
	push ebx
	push edx
	mov ebx,0FFFFFFFFh
	mov ah,48h
	int 21h
	mov eax,01000h
	mul ebx
	pop edx
	pop ebx
	ret
dos_mem_free endp


code	ends
	end
