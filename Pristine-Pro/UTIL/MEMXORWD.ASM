
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP



;xor_words(USHORT data, USHORT *buf, unsigned words_8);
;	xor a buffer with constant data.  
	public	xor_words
xor_words	proc near
	push ebx
	push ecx

	mov eax,[esp+12]   ;value to xor
	mov bx,ax		  ;double up the data word in eax
	shl eax,16
	mov ax,bx
	mov	ebx,[esp+16]	 ;buf
	mov ecx,[esp+20]  ;count
ook:
	xor [ebx],eax
	xor [ebx+4],eax
	xor [ebx+8],eax
	xor [ebx+12],eax
	add ebx,16
	loop ook

	pop ecx
	pop ebx
	ret	
xor_words	endp

code	ends
	end
