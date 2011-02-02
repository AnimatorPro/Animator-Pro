
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;pj_xor_bytes(UBYTE data, USHORT *buf, unsigned words);
;	xor a buffer with constant data.  Count in 2 byte words
	public	pj_xor_bytes
pj_xor_bytes	proc near
	push ebx
	push ecx

	mov eax,[esp+12]   ;value to xor
	mov	ebx,[esp+16]	 ;buf
	mov ecx,[esp+20]  ;count
blook:
	xor [ebx],al
	add ebx,1
	loop blook

	pop ecx
	pop ebx
	ret	
pj_xor_bytes	endp

code	ends
	end
