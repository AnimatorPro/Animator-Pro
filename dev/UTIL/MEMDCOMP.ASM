
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	public pj_dcompare
	;pj_dcompare(USHORT *s1, USHORT *s2, unsigned count)
pj_dcompare PROC near
	push esi
	push edi
	push ecx

	mov esi,[esp+16]
	mov edi,[esp+20]
	mov ecx,[esp+24]
	inc ecx
	repe cmpsd
	mov eax,[esp+24]
	sub eax,ecx

	pop ecx
	pop edi
	pop esi
	ret	
pj_dcompare ENDP

code	ends
	end
