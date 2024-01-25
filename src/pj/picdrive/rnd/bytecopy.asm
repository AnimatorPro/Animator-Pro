
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;stuff_bytes(UBYTE data, UBYTE *buf, unsigned count)
;careful about passing a zero count.  It will be interpreted as 4 Gigabytes.
	public stuff_bytes
stuff_bytes proc near
	push edi
	push ecx
	mov	edi,[esp+16]	;buf
	mov eax,[esp+12]   ;value to poke
	mov ecx,[esp+20]  ;count
	rep stosb
	pop ecx
	pop edi
	ret	
stuff_bytes	endp


;copy_bytes(UBYTE *s,UBYTE *d,unsigned count)
	public copy_bytes
copy_bytes proc near
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
copy_bytes endp

code	ends
	end
