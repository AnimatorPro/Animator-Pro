
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public pj_bcompare
	;pj_bcompare(UBYTE *s1, UBYTE *s2, unsigned count)
pj_bcompare PROC near
	push esi
	push edi
	push ecx

	mov esi,[esp+16]
	mov edi,[esp+20]
	mov ecx,[esp+24]
	inc ecx
	repe cmpsb
	mov eax,[esp+24]
	sub eax,ecx

	pop ecx
	pop edi
	pop esi
	ret	
pj_bcompare ENDP

code	ends
	end
