
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;pj_stuff_words(USHORT data, USHORT *buf, unsigned count)
;careful about passing a zero count.  It will be interpreted as 4 Gigawords.
	public pj_stuff_words
pj_stuff_words proc near
	push edi
	push ecx
	mov	edi,[esp+16]	;buf
	mov eax,[esp+12]   ;value to poke
	mov ecx,[esp+20]  ;count
	rep stosw
	pop ecx
	pop edi
	ret	
pj_stuff_words	endp

code	ends
	end
