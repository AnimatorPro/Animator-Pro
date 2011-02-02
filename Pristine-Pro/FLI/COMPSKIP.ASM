

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public pj_tnskip
;pj_tnskip(Pixel *s1,Pixel *s2,int bcount,int mustmatch);
pj_tnskip proc near
tnkp	struc	;pj_tnskip parameter structure
	tnk_ebp dd ?
	tnk_ret dd ?	;return address for function
	tnk_s1	dd ?	;1st parameter 
	tnk_s2	dd ?
	tnk_bcount	dd ?
	tnk_mustmatch	dd ?
tnkp ends
	push	ebp
	mov ebp,esp
	sub esp,4	;space for local below
difcount	equ	dword ptr[ebp-4]
	push ebx
	push ecx
	push edx
	push esi
	push edi

	mov difcount,0	;zero out return value
	mov esi,[ebp].tnk_s1
	mov edi,[ebp].tnk_s2
	mov ebx,[ebp].tnk_bcount
	mov edx,[ebp].tnk_mustmatch

tnsloop:
	;calculate number of pixels different in s1 and s2 into ax
	mov ecx,ebx
	inc ecx
	repne cmpsb
	mov eax,ebx
	sub eax,ecx
	dec esi	;move source pointers just past this different run
	dec edi
	sub ebx,eax
	add difcount,eax	;and different count to return value

	cmp ebx,edx		;see if near the end...
	js endcheck

	;see if enough in a row match to break out of this
	mov ecx,edx
	repe cmpsb
	jz ztnskip	;if all of them match between s1 and s2 go home
	inc ecx
	mov eax,edx		;calc ones that do match into ax
	sub eax,ecx
	add difcount,eax	;add it to difcount return value
	sub ebx,eax		;sub it from pixels left to examine
	dec esi		;update s1,s2 pointers
	dec edi
	jmp tnsloop
endcheck:
	;check last couple of pixels
	mov ecx,ebx
	inc ecx
	repe cmpsb
	jcxz ztnskip	;if all of them match between s1 and s2 go home
	add difcount,ebx	;otherwise assume no skip this time around

ztnskip:
	mov eax,difcount

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	mov esp,ebp
	pop ebp
	ret
pj_tnskip endp




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
