

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public pj_tnsame
;pj_tnsame(Pixel *s2x,int bcount,int mustmatch);
pj_tnsame proc near
tnsp	struc	;pj_tnskip parameter structure
	tns_edi dd ?
	tns_esi dd ?
	tns_edx dd ?
	tns_ecx dd ?
	tns_ebx dd ?
	tns_ret dd ?	;return address for function
	tns_s2x	dd ?	;1st parameter 
	tns_bcount	dd ?
	tns_mustmatch	dd ?
tnsp ends
	push ebx
	push ecx
	push edx
	push esi
	push edi

	mov	edi,[esp].tns_s2x		;get starting address in es:di
	mov	edx,[esp].tns_bcount		;dx is 'dif_count' return value
	mov	ebx,edx		;bx is # of pixels left
	mov	esi,0		;si is # of pixels examined
alp:
				;break out of loop if less than 4 pixels
				;left to examine
	cmp	ebx,[esp].tns_mustmatch
	js	zalp

	;same_count = pj_bsame(s2x,wcount)
	mov ecx,ebx
	mov al,[edi]
	rep scasb
	inc cx
	sub edi,2
	mov eax,ebx
	sub eax,ecx

	cmp eax,[esp].tns_mustmatch	;if mustmatch or more
	jns gotsame		;go truncate dif_count
	add	esi,eax
	add	edi,eax
	sub	ebx,eax
	jmp	alp
gotsame:
	mov	edx,esi		
zalp:
	mov eax,edx		;return dif_count

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	ret
pj_tnsame endp


code	ends
	end
