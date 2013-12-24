
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;xor_group(USHORT data, USHORT *buf, unsigned words);
;	xor a buffer with constant data.  Count in 2 byte words
	public	xor_group
xor_group	proc near
	push ebx
	push ecx

	mov eax,[esp+12]   ;value to xor
	mov	ebx,[esp+16]	 ;buf
	mov ecx,[esp+20]  ;count
glook:
	xor [ebx],ax
	add ebx,2
	loop glook

	pop ecx
	pop ebx
	ret	
xor_group	endp

code	ends
	end
