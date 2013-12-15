
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;pj_copy_words(UBYTE *s,UBYTE *d,unsigned count)
	public pj_copy_words
pj_copy_words proc near
	push esi
	push edi
	push ecx
	mov esi,[esp+16]
	mov edi,[esp+20]
	mov ecx,[esp+24]
	rep movsw
	pop ecx
	pop edi
	pop esi
	ret
pj_copy_words endp

code	ends
	end
