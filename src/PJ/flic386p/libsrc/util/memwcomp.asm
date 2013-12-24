
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public pj_fcompare
	;pj_fcompare(USHORT *s1, USHORT *s2, unsigned count)
pj_fcompare PROC near
	push esi
	push edi
	push ecx

	mov esi,[esp+16]
	mov edi,[esp+20]
	mov ecx,[esp+24]
	inc ecx
	repe cmpsw
	mov eax,[esp+24]
	sub eax,ecx

	pop ecx
	pop edi
	pop esi
	ret	
pj_fcompare ENDP

code	ends
	end
