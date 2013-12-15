
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	;Set BIOS video mode
	;pj_8514_set_vmode(mode)
	public pj_8514_set_vmode
pj_8514_set_vmode proc near
	mov eax,4[esp]
	int 10H
	ret
pj_8514_set_vmode endp

	;Get BIOS video mode
	;pj_8514_get_vmode()
	public pj_8514_get_vmode
pj_8514_get_vmode proc near
	push ebx
	mov ah,0fh
	int 10H
	and eax,0ffh
	pop ebx
	ret
pj_8514_get_vmode endp

	;busy-wait for vblank
	public pj_8514_wait_vblank
pj_8514_wait_vblank proc near
	push edx
	mov	dx,3dah	;video status port
wvb:
	in	al,dx
	test al,8
	jz wvb
	pop edx
	ret
pj_8514_wait_vblank endp

	;busy-wait for out of vblank
	public pj_8514_wait_novblank
pj_8514_wait_novblank proc near
	push edx
	mov	dx,3dah	;video status port
wnvb:
	in	al,dx
	test al,8
	jnz wnvb
	pop edx
	ret
pj_8514_wait_novblank endp

	;wait for novblank then wait for vblank
	public pj_8514_wait_vsync
pj_8514_wait_vsync proc near
	call near ptr pj_8514_wait_novblank
	jmp short pj_8514_wait_vblank
pj_8514_wait_vsync endp

code	ends
	end
