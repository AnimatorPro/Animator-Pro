
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;exchange_words(UBYTE *s,UBYTE *d,unsigned count)
	public exchange_words
exchange_words proc near
	push esi
	push edi
	push ecx
	push ebx
	mov esi,[esp+20]
	mov edi,[esp+24]
	mov ecx,[esp+28]

exlp:
	mov ax,[esi]
	mov bx,[edi]
	mov [esi],bx
	add esi,2
	stosw
	loop exlp

	pop ebx
	pop ecx
	pop edi
	pop esi
	ret
exchange_words endp

code	ends
	end
