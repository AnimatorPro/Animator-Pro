
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	public pj_fsame
	;pj_fsame(USHORT *buf, unsigned count);
	;	see how many words in a row of buffer are same as first
pj_fsame proc near
	push edi
	push ecx

	mov edi,[esp+12]
	mov ecx,[esp+16]
	mov ax,[edi]
	inc ecx
	repe scasw

	mov eax,[esp+16]
	sub eax,ecx

	pop ecx
	pop edi
	ret	
pj_fsame endp

code	ends
	end
