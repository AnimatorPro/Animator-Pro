
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public fcontrast
	;fcontrast(USHORT *s1, USHORT *s2, unsigned count)
fcontrast PROC near
	push esi
	push edi
	push ecx

	mov esi,[esp+16]
	mov edi,[esp+20]
	mov ecx,[esp+24]
	repne cmpsw
	inc ecx
	mov eax,[esp+24]
	sub eax,ecx

	pop ecx
	pop edi
	pop esi
	ret	
fcontrast ENDP

code	ends
	end
