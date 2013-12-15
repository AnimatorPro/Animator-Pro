CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	public pj_bsame
	;pj_bsame(UBYTE *buf, unsigned count);
	;	see how many bytes in a row of buffer are same as first
pj_bsame proc near
	push edi
	push ecx

	mov edi,[esp+12]
	mov ecx,[esp+16]
	mov al,[edi]
	inc ecx
	repe scasb

	mov eax,[esp+16]
	sub eax,ecx

	pop ecx
	pop edi
	ret	
pj_bsame endp

code	ends
	end
