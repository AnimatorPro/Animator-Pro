
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;int back_scan(int value, UBYTE *buf, unsigned count)
;     Return how many bytes starting at buf and running _backwards_ match
;     value.  Used in determining bounding box of non-key-color part of
;     screen for cel/clip.
back_scan	proc	near
	public back_scan
	push edi
	push ecx

	mov eax,[esp+12]
	mov edi,[esp+16]
	mov ecx,[esp+20]
	dec edi
	std
	rep scasb
	cld
	inc ecx
	mov eax,[esp+20]
	sub eax,ecx

	pop ecx
	pop edi
	ret
back_scan endp

code	ends
	end
