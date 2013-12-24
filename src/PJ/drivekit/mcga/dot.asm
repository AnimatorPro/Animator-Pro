
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public pj_mcga_get_dot
;extern int pj_mcga_get_dot(Vscreen *v, int x, int y);
	;unclipped get-dot
pj_mcga_get_dot proc near
mgdp	struc	;cdot parameter structure
	mgd_ds	dd ?
	mgd_edi dd ?
	mgd_ret dd ?
	mgd_v	dd ?
	mgd_x	dd ?
	mgd_y	dd ?
mgdp ends
	push edi
	push ds

	;put screen address into ds:ax
	mov edi,[esp].mgd_v
	mov eax,[esp].mgd_y
	mul [edi].bym_bpr	
	add eax,[esp].mgd_x
	add eax,[edi].bym_p
	mov dx,VGA_SEG
	mov ds,dx

	;and mgd pixel
	mov al,[eax]
	and eax,0ffh	;clear hi bits of result

	pop ds
	pop edi
	ret
pj_mcga_get_dot endp


	public pj_mcga_cput_dot
pj_mcga_cput_dot proc near
cdop	struc	;pj_mcga_cput_dot parameter structure
	cdo_ds	dd ?
	cdo_edi dd ?
	cdo_edx dd ?
	cdo_ret dd ?
	cdo_v	dd ?
	cdo_color dd ?
	cdo_x	dd ?
	cdo_y	dd ?
cdop ends

	push edx
	push edi
	push ds

	mov edi,[esp].cdo_v

	;load y coordinate and check it is within screen bounds
	mov eax,[esp].cdo_y
	or eax,eax	;test for y coordinate less than zero
	js	cdotz
	cmp	ax,[edi].bym_h
	jge	cdotz

	;put line address into eax
	mul [edi].bym_bpr	

	;load x coordinate and check it is within screen bounds
	mov edx,[esp].cdo_x
	or edx,edx	;test for x coordinate less than zero
	js	cdotz
	cmp	dx,[edi].bym_w
	jge	cdotz

	;finish computing screen address into ds:eax
	add eax,edx
	add eax,[edi].bym_p
	mov dx,VGA_SEG
	mov ds,dx

	;and set pixel to color
	mov edx,[esp].cdo_color
	mov [eax],dl

cdotz:
	pop ds
	pop edi
	pop edx
	ret
pj_mcga_cput_dot endp



;void JREGS pj_mcga_put_dot(Bytemap *v, RCOLOR color, LONG x, LONG y);
	public pj_mcga_put_dot
	; unclipped put dot
pj_mcga_put_dot proc near
_mcgap	struc	;bym_put_dot parameter structure
	_mcga_ds	dd ?
	_mcga_edi dd ?
	_mcga_ret dd ?
	_mcga_v	dd ?
	_mcga_color dd ?
	_mcga_x	dd ?
	_mcga_y	dd ?
_mcgap ends
	push edi
	push ds

	mov edi,[esp]._mcga_v

	;load y coordinate
	mov eax,[esp]._mcga_y
	;put line address into eax
	mul [edi].bym_bpr	
	add eax,[esp]._mcga_x
	add eax,[edi].bym_p
	;get pointer to screen structure
	mov edi,[esp]._mcga_v

	;put line address into edx
	mov eax,[esp]._mcga_y
	mul [edi].bym_bpr	
	add eax,[esp]._mcga_x
	mov edi,[edi].bym_p
	add edi,eax

	;get mcga segment into ds
	mov ax,VGA_SEG
	mov ds,ax

	;and set pixel to color
	mov eax,[esp]._mcga_color
	mov [edi],al

	pop ds
	pop edi
	ret
pj_mcga_put_dot endp


code	ends
	end

