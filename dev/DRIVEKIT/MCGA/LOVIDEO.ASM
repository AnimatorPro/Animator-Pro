	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	;Set BIOS video mode
	;set_vmode(mode)
	public set_vmode
set_vmode proc near
	mov eax,4[esp]
	int 10H
	ret
set_vmode endp

	;Get BIOS video mode
	;get_vmode()
	public get_vmode
get_vmode proc near
	push ebx
	mov ah,0fh
	int 10H
	and eax,0ffh
	pop ebx
	ret
get_vmode endp

	;busy-wait for vblank
	public mcga_wait_vblank
mcga_wait_vblank proc near
	push edx
	mov	dx,3dah	;video status port
wvb:
	in	al,dx
	test al,8
	jz wvb
	pop edx
	ret
mcga_wait_vblank endp

	;busy-wait for out of vblank
	public mcga_wait_novblank
mcga_wait_novblank proc near
	push edx
	mov	dx,3dah	;video status port
wnvb:
	in	al,dx
	test al,8
	jnz wnvb
	pop edx
	ret
mcga_wait_novblank endp

	;wait for novblank then wait for vblank
	public mcga_wait_vsync
mcga_wait_vsync proc near
	call near ptr mcga_wait_novblank
	jmp short mcga_wait_vblank
mcga_wait_vsync endp

code	ends
	end
