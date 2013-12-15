CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	;Get BIOS video mode
	;get_vmode()
	public pj_get_vmode
pj_get_vmode proc near
	push ebx
	mov ah,0fh
	int 10H
	and eax,0ffh
	pop ebx
	ret
pj_get_vmode endp

code	ends
	end
