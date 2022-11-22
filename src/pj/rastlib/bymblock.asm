
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	public pj_bym_set_hline
;void JREGS pj_bym_set_hline(Bytemap *bm, RCOLOR color, LONG minx, 
;	LONG y, LONG width);
pj_bym_set_hline proc near
chlp struc  ;bym_set_hline parameter structure
	chl_ebp	dd ?
	chl_ret dd ?
	chl_v	dd ?
	chl_color dd ?
	chl_x	dd ?
	chl_y	dd ?
	chl_w	dd ?
chlp ends
	push ebp
	mov ebp,esp
	push edx
	push ecx
	push edi
	push es

	;move start address of line into es:edi
	mov edi,[ebp].chl_v
	mov ax,[edi].bym_pseg
	mov es,ax
	mov eax,[ebp].chl_y
	mul [edi].bym_bpr
	mov edi,[edi].bym_p
	add edi,eax
	add edi,[ebp].chl_x

	mov ecx,[ebp].chl_w
	mov eax,[ebp].chl_color
	rep stosb

	pop es
	pop edi
	pop ecx
	pop edx
	pop ebp
	ret
pj_bym_set_hline endp


	public pj_bym_set_vline
;void JREGS pj_bym_set_vline(Bytemap *bm, RCOLOR color, LONG x, 
;	LONG miny, LONG height);
pj_bym_set_vline proc near
cvlp struc  ;bym_set_vline parameter structure
	cvl_ebp	dd ?
	cvl_ret dd ?
	cvl_v	dd ?
	cvl_color dd ?
	cvl_x	dd ?
	cvl_y	dd ?
	cvl_h	dd ?
cvlp ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	push edi
	push es

	mov edi,[ebp].cvl_v
	mov ebx,[edi].bym_bpr

	;move start address of line into es:edi
	mov ax,[edi].bym_pseg
	mov es,ax
	mov eax,[ebp].cvl_y
	mul ebx
	mov edi,[edi].bym_p
	add edi,eax
	add edi,[ebp].cvl_x

	dec ebx		;ebx what to add to get to next line
	mov eax,[ebp].cvl_color
	mov ecx,[ebp].cvl_h
	jecxz cvllz
cvllp:
	stosb
	add edi,ebx
	loop cvllp

cvllz:
	pop es
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
pj_bym_set_vline endp

cblp struc	;pj_bym_set_rect/pj_bym_xor_rect parameter structure
	cbl_edi dd ?
	cbl_esi dd ?
	cbl_ebp dd ?
	cbl_esp dd ?
	cbl_ebx dd ?
	cbl_edx dd ?
	cbl_ecx dd ?
	cbl_eax dd ?
	cbl_ret dd ?
	cbl_v	dd ?
	cbl_color dd ?
	cbl_x	dd ?
	cbl_y	dd ?
	cbl_w	dd ?
	cbl_h	dd ?
cblp ends

clipblock proc near
	;make sure width is positive
	mov	ebx,[ebp].cbl_w
	or	ebx,ebx	;set flags
	jle	bclipped
	;clip to the right
	mov	eax,[ebp].cbl_x	;get starting dest x
	sub	ax,[esi].bym_w ;if its past right of screen clip nothing left
	jge	bclipped
	cwde
	add	eax,ebx	;eax = portion of right side of block past right of screen
	jl	norclip ;if none of it past go clip another side
	sub	[ebp].cbl_w,eax	
norclip:	;clip to the left
	mov	eax,[ebp].cbl_x	;get starting dest x
	and	eax,eax		;set flags
	jns	nolclip
	add	[ebp].cbl_w,eax
	jle	bclipped
	mov [ebp].cbl_x,0
nolclip:	;clip off the bottom
	mov	ebx,[ebp].cbl_h
	or	ebx,ebx	;set flags
	jle	bclipped
	mov	eax,[ebp].cbl_y	;get starting dest y
	sub	ax,[esi].bym_h
	jge	bclipped
	cwde
	add	eax,ebx
	jl	nobclip
	sub	[ebp].cbl_h,eax
nobclip:	;clip off the top
	mov	eax,[ebp].cbl_y	;get starting dest y
	and	eax,eax		;set flags
	jns	nouclip
	add	[ebp].cbl_h,eax
	jle	bclipped
	mov [ebp].cbl_y,0
nouclip:
	clc
	ret
bclipped:
	stc
	ret
clipblock endp


	public pj_bym_set_rect
;void JREGS pj_bym_set_rect(Bytemap *bm,  RCOLOR color, 
;			 LONG x, LONG y, ULONG width, ULONG height);
pj_bym_set_rect proc near
	pushad
	mov ebp,esp
	push es

	mov esi,[ebp].cbl_v

	;clip it!
	call clipblock
	jc cblockout

	;address of upper left pixel into es:di
	mov eax,[ebp].cbl_y
	mov ebx,[esi].bym_bpr
	mul ebx
	mov edi,[ebp].cbl_x
	add edi,eax
	add edi,[esi].bym_p
	mov ax,[esi].bym_pseg
	mov es,ax

	sub ebx,[ebp].cbl_w	;ebx is what to add to get to next line
	mov esi,[ebp].cbl_h	
	mov eax,[ebp].cbl_color
	jmp zcbz

cbloop:
	mov ecx,[ebp].cbl_w
	rep stosb
	add edi,ebx
zcbz:
	dec esi
	js cblockout
	jmp cbloop
cblockout:
	pop es
	popad
	ret
pj_bym_set_rect endp

	public pj_bym_xor_rect
;static void pj_bym_xor_rect(Bytemap *bm, RCOLOR color,
;				 LONG x, LONG y, ULONG width, ULONG height);
pj_bym_xor_rect proc near
	pushad
	mov ebp,esp
	push es
	push ds

	mov esi,[ebp].cbl_v

	;clip it!
	call clipblock
	jc xorcblockout

	;address of upper left pixel into es:di
	mov eax,[ebp].cbl_y
	mov ebx,[esi].bym_bpr
	mul ebx
	mov edi,[ebp].cbl_x
	add edi,eax
	add edi,[esi].bym_p
	mov ax,[esi].bym_pseg
	mov es,ax
	mov ds,ax

	sub ebx,[ebp].cbl_w	;ebx is what to add to get to next line
	mov eax,[ebp].cbl_color
	mov ah,al
	mov esi,edi
	jmp xorzcbz

xorbloop:
	mov ecx,[ebp].cbl_w
	jecxz xorzi
xori:
	lodsb
	xor al,ah
	stosb
	loop xori
xorzi:
	add edi,ebx
	add esi,ebx
xorzcbz:
	dec [ebp].cbl_h
	js xorcblockout
	jmp xorbloop

xorcblockout:
	pop ds
	pop es
	popad
	ret
pj_bym_xor_rect endp


code	ends
	end
