CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	;Set BIOS video mode
	;pj_set_vmode(mode)
	public pj_set_vmode
pj_set_vmode proc near
	mov eax,4[esp]
	int 10H
	ret
pj_set_vmode endp

code	ends
	end
