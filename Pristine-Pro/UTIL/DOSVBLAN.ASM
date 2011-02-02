CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	;busy-wait for vblank
	public pj_wait_vblank
pj_wait_vblank proc near
	push edx
	mov	dx,3dah	;video status port
wvb:
	in	al,dx
	test al,8
	jz wvb
	pop edx
	ret
pj_wait_vblank endp

	;busy-wait for out of vblank
	public pj_wait_novblank
pj_wait_novblank proc near
	push edx
	mov	dx,3dah	;video status port
wnvb:
	in	al,dx
	test al,8
	jnz wnvb
	pop edx
	ret
pj_wait_novblank endp

	;wait for novblank then wait for vblank
	public pj_wait_vsync
pj_wait_vsync proc near
	call near ptr pj_wait_novblank
	jmp short pj_wait_vblank
pj_wait_vsync endp

code	ends
	end
