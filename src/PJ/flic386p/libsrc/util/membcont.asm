
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public pj_bcontrast
	;pj_bcontrast(UBYTE *s1, UBYTE *s2, unsigned count)
pj_bcontrast PROC near
	push esi
	push edi
	push ecx

	mov esi,[esp+16]
	mov edi,[esp+20]
	mov ecx,[esp+24]
	repne cmpsb
	inc ecx
	mov eax,[esp+24]
	sub eax,ecx

	pop ecx
	pop edi
	pop esi
	ret	
pj_bcontrast ENDP

code	ends
	end
