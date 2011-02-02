CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

;pj_key_in() - returns next keyboard input - will wait for it if necessary
	public pj_key_in
pj_key_in proc near
	mov ah,0
	int 16h
	and eax,0FFFFh
	ret
pj_key_in endp

code	ends
	end
