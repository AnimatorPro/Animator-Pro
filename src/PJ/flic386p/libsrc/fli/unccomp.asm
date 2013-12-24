
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public pj_fcuncomp
	;uncompress color palette onto a buffer
pj_fcuncomp proc near
	push ebp
	mov ebp,esp
	push esi
	push edi
	push ecx
	push ebx

	xor eax,eax ;and hi bits of accumulator

	mov esi,[ebp+8]
	mov edi,[ebp+12]
	lodsw
	mov ebx,eax   ;get the count
	test ebx,ebx
	jmp jendu
ju:
	lodsb		;get colors to skip
	add edi,eax
	add edi,eax
	add edi,eax
	lodsb		;get colors to copy
	or  al,al	;test for zero
	jnz	ju2
	mov eax,256
ju2:
	mov ecx,eax
	add ecx,eax
	add ecx,eax
	rep movsb
	dec ebx
jendu:
	jnz ju

	pop ebx
	pop ecx
	pop edi
	pop esi
	pop	ebp
	ret	

pj_fcuncomp endp

	public pj_fcuncomp64
	;uncompress color palette onto a buffer
pj_fcuncomp64 proc near
	push ebp
	mov ebp,esp
	push esi
	push edi
	push ecx
	push ebx

	xor eax,eax ;and hi bits of accumulator

	mov esi,[ebp+8]
	mov edi,[ebp+12]
	lodsw
	mov ebx,eax   ;get the count
	test ebx,ebx
	jmp endu
u:
	lodsb		;get colors to skip
	add edi,eax
	add edi,eax
	add edi,eax
	lodsb		;get colors to copy
	or  al,al	;test for zero
	jnz	u2
	mov eax,256
u2:
	mov ecx,eax
	add ecx,eax
	add ecx,eax
lloop:
	lodsb
	shl al,2
	stosb
	loop lloop
	dec ebx
endu:
	jnz u

	pop ebx
	pop ecx
	pop edi
	pop esi
	pop	ebp
	ret	

pj_fcuncomp64 endp

code	ends
	end

