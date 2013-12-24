
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;pj_copy_bytes(UBYTE *s,UBYTE *d,unsigned count)
	public pj_copy_bytes
pj_copy_bytes proc near
	push esi
	push edi
	push ecx
	mov esi,[esp+16]
	mov edi,[esp+20]
	mov ecx,[esp+24]
	rep movsb
	pop ecx
	pop edi
	pop esi
	ret
pj_copy_bytes endp

code	ends
	end
